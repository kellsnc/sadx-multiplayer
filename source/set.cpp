#include "pch.h"
#include "set.h"
#include "camera.h"
#include "splitscreen.h"

#define SQUARE(x)((x) * (x))

FastFunctionHook<void> ProcessStatusTable_h(0x46BCE0);
FastFunctionHook<Bool, task*, float> CheckRangeWithR_h(0x46BFA0);
FastFunctionHook<Bool, task*, float> CheckRangeOutWithR_h(0x46C010);
FastFunctionHook<Bool, task*, float> SDCheckRangeOutWithR_h(0x5EDB10);
FastUsercallHookPtr<void(*)(Uint32 u32SetType, const char* pcFileName), noret, rECX, stack4> LoadSetFile_h(0x422930);

DataPointer(OBJ_CONDITION*, pObjStatusEntry, 0x46B817); // Get objStatusEntry pointer from function to be compatible with mods that overwrite it (LimitBreak, Windy Valley Beta)

// Function to get a specific SET task, used to sync object state for netplay
task* GetSetTask(int set_id, const char* name)
{
	if (numStatusEntry < set_id)
	{
		return nullptr;
	}

	OBJ_CONDITION* item = &pObjStatusEntry[set_id];

	if (!item->pTask)
	{
		return nullptr;
	}

	auto& objinfo = pObjItemTable->pObjItemEntry[item->pObjEditEntry->usID & 0xFFF];

	if (strcmp(objinfo.strObjName, name))
	{
		return nullptr;
	}

	return item->pTask;
}

// Function to get id of SET task, used to sync object state for netplay
int GetSetID(task* tp)
{
	if (tp->ocp)
	{
		for (int i = 0; i < numStatusEntry; ++i)
		{
			// Get objStatusEntry pointer from function to be compatible with mods that overwrite it (LimitBreak, Windy Valley Beta)
			OBJ_CONDITION* item = &pObjStatusEntry[i];

			if (tp->ocp == item)
			{
				return i;
			}
		}
	}
	return -1;
}

void CreateSetTask(OBJ_CONDITION* item, _OBJ_EDITENTRY* objentry, _OBJ_ITEMENTRY* objinfo, float distance)
{
	if (item->ssCondition & 1)
	{
		return;
	}

	auto tp = CreateElementalTask(objinfo->ucInitMode, objinfo->ucLevel, objinfo->fnExec);
	item->ssCondition |= 1; // we're loaded!
	++item->scCount;
	tp->ocp = item;
	auto twp = tp->twp;

	if (twp)
	{
		if (item->ssCondition & 2)
		{
			auto objsocd = item->unionStatus.pObjSleepCondition;
			twp->pos = objsocd->pos;
			twp->ang = objsocd->ang;
			twp->scl = objsocd->scl;
			FreeMemory(objsocd);
			item->ssCondition &= ~2u;
		}
		else
		{
			twp->pos.x = objentry->xpos;
			twp->pos.y = objentry->ypos;
			twp->pos.z = objentry->zpos;
			twp->ang.x = objentry->rotx;
			twp->ang.y = objentry->roty;
			twp->ang.z = objentry->rotz;
			twp->scl.x = objentry->xscl;
			twp->scl.y = objentry->yscl;
			twp->scl.z = objentry->zscl;
		}
	}

	item->unionStatus.fRangeOut = distance; // tell CheckRangeOut when to unload object
	item->pTask = tp;
}

Bool CheckRangeXYZRP(NJS_POINT3* pf, Float rx0, Float ry0, Float rz0, Float R)
{
	float r1 = SQUARE(rx0 - pf->x);
	if (r1 <= R)
	{
		float r2 = r1 + SQUARE(ry0 - pf->y);
		if (r2 <= R)
		{
			float r3 = r2 + SQUARE(rz0 - pf->z);
			if (r3 <= R)
			{
				return 1;
			}
		}
	}
	return 0;
}

bool CheckRangeM(Float rx0, Float ry0, Float rz0, Float R)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertwp[i])
		{
			if (CheckRangeXYZRP(&playertwp[i]->pos, rx0, ry0, rz0, R))
			{
				return true;
			}
		}

		if (splitscreen::IsScreenEnabled(i))
		{
			auto cam_pos = GetCameraPosition(i);

			if (cam_pos && CheckRangeXYZRP(cam_pos, rx0, ry0, rz0, R))
			{
				return true;
			}
		}
	}

	return false;
}

// Load objects around every players
void __cdecl ProcessStatusTable_r()
{
	if (multiplayer::IsActive())
	{
		for (int i = 0; i < numStatusEntry; ++i)
		{
			OBJ_CONDITION* item = &pObjStatusEntry[i];

			// Object already loaded
			if (item->ssCondition >= 0 || (item->ssCondition & 1))
			{
				continue;
			}

			auto objentry = item->pObjEditEntry;

			// If the object id is beyond the object count
			int count = objentry->usID & 0xFFF;
			if (count != 0 && pObjItemTable->ssCount <= count)
			{
				continue;
			}

			int cliplevel = (objentry->usID >> 12) & 7;

			if (cliplevel)
			{
				if (cliplevel == 1)
				{
					if (ClipLevel >= 1)
					{
						continue;
					}
				}
				else if (ClipLevel >= 2)
				{
					continue;
				}
			}

			auto& objinfo = pObjItemTable->pObjItemEntry[objentry->usID & 0xFFF];

			if (objinfo.ssAttribute & 2)
			{
				CreateSetTask(item, objentry, &objinfo, 0.0f);
			}
			else
			{
				float dist = (objinfo.ssAttribute & 1) ? objinfo.fRange : 160000.0f;

				if ((boolOneShot == FALSE && objinfo.ssAttribute & 4) ||
					CheckRangeM(objentry->xpos, objentry->ypos, objentry->zpos, dist))
				{
					CreateSetTask(item, objentry, &objinfo, dist);
				}
			}
		}

		boolOneShot = TRUE;
	}
	else
	{
		ProcessStatusTable_h.Original();
	}
}

Bool CheckRangeWithR_m(task* tp, float fRange)
{
	if ((tp->ocp && (tp->ocp->ssCondition & 8)) || fRange == 0.0f)
	{
		return FALSE;
	}

	if (CheckRangeM(tp->twp->pos.x, tp->twp->pos.y, tp->twp->pos.z, fRange))
	{
		return FALSE;
	}

	return TRUE;
}

// Check object deletion for every player
Bool __cdecl CheckRangeWithR_r(task* tp, float fRange)
{
	if (multiplayer::IsActive())
	{
		return CheckRangeWithR_m(tp, fRange);
	}
	else
	{
		return CheckRangeWithR_h.Original(tp, fRange);
	}
}

// Check object deletion for every player
Bool __cdecl CheckRangeOutWithR_r(task* tp, float fRange)
{
	if (multiplayer::IsActive())
	{
		if (CheckRangeWithR_m(tp, fRange))
		{
			tp->exec = FreeTask;
			return TRUE;
		}

		return FALSE;
	}
	else
	{
		return CheckRangeOutWithR_h.Original(tp, fRange);
	}
}

// Check object deletion for every player, but in Sky Deck
Bool __cdecl SDCheckRangeOutWithR_r(task* tp, float fRange)
{
	if (multiplayer::IsActive())
	{
		if (CheckRangeWithR_m(tp, fRange))
		{
			tp->exec = FreeTask;
			return TRUE;
		}

		return FALSE;
	}
	else
	{
		return SDCheckRangeOutWithR_h.Original(tp, fRange);
	}
}

// Load Multiplayer version of setfiles.
void __cdecl LoadSetFile_r(Uint32 u32SetType, const char* pcFileName)
{
	if (multiplayer::IsActive())
	{
		std::string temp = (std::string)"M" + (std::string)pcFileName;

		LoadSetFile_h.Original(u32SetType, temp.c_str());

		// If loaded properly exit, otherwise run original behaviour
		if (SetFiles[u32SetType])
		{
			return;
		}
	}

	LoadSetFile_h.Original(u32SetType, pcFileName);
}

void InitSET()
{
	ProcessStatusTable_h.Hook(ProcessStatusTable_r);
	CheckRangeWithR_h.Hook(CheckRangeWithR_r);
	CheckRangeOutWithR_h.Hook(CheckRangeOutWithR_r);
	SDCheckRangeOutWithR_h.Hook(SDCheckRangeOutWithR_r);
	LoadSetFile_h.Hook(LoadSetFile_r);
}
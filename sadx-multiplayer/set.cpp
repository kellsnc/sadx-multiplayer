#include "pch.h"
#include "set.h"
#include "camera.h"

Trampoline* ProcessStatusTable_t = nullptr;
Trampoline* CheckRangeWithR_t    = nullptr;
Trampoline* CheckRangeOutWithR_t = nullptr;
Trampoline* LoadSetFile_t        = nullptr;

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

// Load objects around every players
void __cdecl ProcessStatusTable_r()
{
	if (multiplayer::IsBattleMode())
	{
		for (int i = 0; i < numStatusEntry; ++i)
		{
			// Get objStatusEntry pointer from function to be compatible with mods that overwrite it (LimitBreak, Windy Valley Beta)
			OBJ_CONDITION* item = &(*(OBJ_CONDITION**)0x46B817)[i];

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

				if (boolOneShot == FALSE && objinfo.ssAttribute & 4)
				{
					CreateSetTask(item, objentry, &objinfo, dist);
					continue;
				}

				for (int i = 0; i < PLAYER_MAX; ++i)
				{
					if (!playertwp[i]) continue;

					auto cam_pos = GetCameraPosition(i);
					NJS_POINT3 pos = cam_pos ? *cam_pos : playertwp[i]->pos;
					njSubVector(&pos, (NJS_POINT3*)&objentry->xpos);

					if (njScalor2(&pos) < dist)
					{
						CreateSetTask(item, objentry, &objinfo, dist);
						break;
					}
				}
			}
		}

		boolOneShot = TRUE;
	}
	else
	{
		TARGET_DYNAMIC(ProcessStatusTable)();
	}
}

BOOL CheckRangeWithR_m(task* tp, float fRange)
{
	if ((tp->ocp && (tp->ocp->ssCondition & 8)) || fRange == 0.0f)
	{
		return FALSE;
	}

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertwp[i])
		{
			auto cam_pos = GetCameraPosition(i);
			NJS_POINT3 pos = cam_pos ? *cam_pos : playertwp[i]->pos;
			pos.x -= tp->twp->pos.x;
			pos.y -= tp->twp->pos.y;
			pos.z -= tp->twp->pos.z;
			
			if (njScalor2(&pos) < fRange)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

// Check object deletion for every player
BOOL __cdecl CheckRangeWithR_r(task* tp, float fRange)
{
	if (multiplayer::IsActive())
	{
		return CheckRangeWithR_m(tp, fRange);
	}
	else
	{
		return TARGET_DYNAMIC(CheckRangeWithR)(tp, fRange);
	}
}

// Check object deletion for every player
BOOL __cdecl CheckRangeOutWithR_r(task* tp, float fRange)
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
		return TARGET_DYNAMIC(CheckRangeOutWithR)(tp, fRange);
	}
}

// Load Multiplayer version of setfiles:

void __cdecl LoadSetFile_o(unsigned int u32SetType, const char* pcFileName)
{
	const auto LoadSetFile_ptr = LoadSetFile_t->Target();

	__asm
	{
		push[pcFileName]
		mov ecx, u32SetType
		call LoadSetFile_ptr
		add esp, 4
	}
}

void __cdecl LoadSetFile_r(unsigned int u32SetType, const char* pcFileName)
{
	if (multiplayer::IsActive())
	{
		std::string temp = (std::string)"M" + (std::string)pcFileName;

		LoadSetFile_o(u32SetType, temp.c_str());

		// If loaded properly exit, otherwise run original behaviour
		if (SetFiles[u32SetType])
		{
			return;
		}
	}
	
	LoadSetFile_o(u32SetType, pcFileName);
}

static void __declspec(naked) LoadSetFile_j()
{
	__asm
	{
		push[esp + 04h]
		push ecx
		call LoadSetFile_r
		pop ecx
		add esp, 4
		retn
	}
}

void InitSET()
{
	ProcessStatusTable_t = new Trampoline(0x46BCE0, 0x46BCE5, ProcessStatusTable_r);
	CheckRangeWithR_t    = new Trampoline(0x46BFA0, 0x46BFA7, CheckRangeWithR_r);
	CheckRangeOutWithR_t = new Trampoline(0x46C010, 0x46C018, CheckRangeOutWithR_r);
	LoadSetFile_t        = new Trampoline(0x422930, 0x422938, LoadSetFile_j);
}
#include "pch.h"
#include "set.h"
#include "camera.h"

Trampoline* ProcessStatusTable_t = nullptr;
Trampoline* CheckRangeOutWithR_t = nullptr;

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
	if (IsMultiplayerEnabled())
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

					NJS_POINT3 pos = camera_twp ? *GetCameraPosition(i) : playertwp[i]->pos;
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

// Check object deletion for every player
BOOL __cdecl CheckRangeOutWithR_r(task* tp, float fRange)
{
	if (IsMultiplayerEnabled())
	{
		// Do not delete if the no delete flag is set
		if (tp->ocp && (tp->ocp->ssCondition & 8))
		{
			return FALSE;
		}

		// Do not delete if range is 0
		if (fRange == 0.0f)
		{
			return FALSE;
		}

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (playertp[i])
			{
				NJS_POINT3 pos = *GetCameraPosition(i);
				njSubVector(&pos, &tp->twp->pos);

				if (njScalor2(&pos) < fRange)
				{
					return FALSE;
				}
			}
		}

		tp->exec = FreeTask;
		return TRUE;
	}
	else
	{
		return TARGET_DYNAMIC(CheckRangeOutWithR)(tp, fRange);
	}
}

void InitSET()
{
	ProcessStatusTable_t = new Trampoline(0x46BCE0, 0x46BCE5, ProcessStatusTable_r);
	CheckRangeOutWithR_t = new Trampoline(0x46C010, 0x46C018, CheckRangeOutWithR_r);
}
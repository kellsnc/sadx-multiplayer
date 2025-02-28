#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "splitscreen.h"
#include "multiplayer.h"

FastUsercallHookPtr<void(*)(task*, taskwk*), noret, rEDI, rESI> savepointCollision_t(0x44F430);
FastFunctionHook<void, CUSTUM_PRINT_NUMBER*, Sint32, Sint32> PrintTimer_t(0x4BABE0);

float savepointGetSpeed_m(taskwk* twp, int pID)
{
	auto ptwp = playertwp[pID];
	auto ppwp = playerpwp[pID];

	if (!ptwp || !ppwp)
	{
		return 0.0f;
	}

	Float spd = njScalor(&ppwp->spd);

	if (DiffAngle(0x4000 - ptwp->ang.y, twp->ang.y) <= 0x4000)
	{
		return spd;
	}
	else
	{
		return -spd;
	}
}

void __cdecl savepointCollision_r(task* tp, taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		savepoint_data->tp[0]->twp->ang.x = twp->ang.x + savepoint_data->ang.x;
		savepoint_data->tp[0]->twp->ang.y = twp->ang.y + savepoint_data->ang.y;
		savepoint_data->tp[0]->twp->ang.z = twp->ang.z + savepoint_data->ang.z;
		savepoint_data->tp[1]->twp->ang.x = twp->ang.x + savepoint_data->ang.x;
		savepoint_data->tp[1]->twp->ang.y = twp->ang.y + savepoint_data->ang.y;
		savepoint_data->tp[1]->twp->ang.z = twp->ang.z + savepoint_data->ang.z;

		if (twp->mode == 1)
		{
			auto entity = CCL_IsHitPlayer(twp);

			if (entity)
			{
				twp->mode = 2;
				int pID = TASKWK_PLAYERID(entity);
				savepoint_data->write_timer = 300;
				savepoint_data->ang_spd.y = (Angle)((savepointGetSpeed_m(twp, pID) * 10.0f) * 65536.0f * 0.0028f);
				updateContinueData(&entity->pos, &entity->ang);
				SetBroken(tp);
				dsPlay_oneshot(SE_SAVE, 0, 0, 0);
				twp->btimer = pID;
			}
		}

		EntryColliList(twp);
	}
	else
	{
		savepointCollision_t.Original(tp, twp);
	}
}

void __cdecl PrintTimer_r(CUSTUM_PRINT_NUMBER* custom, Sint32 min, Sint32 sec)
{
	if (!SplitScreen::IsActive())
	{
		PrintTimer_t.Original(custom, min, sec);
	}
}

void patch_savepoint_init()
{
	savepointCollision_t.Hook(savepointCollision_r);
	PrintTimer_t.Hook(PrintTimer_r);
}

RegisterPatch patch_savepoint(patch_savepoint_init);
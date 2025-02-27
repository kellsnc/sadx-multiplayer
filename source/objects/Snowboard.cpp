#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"

FastFunctionHook<void, task*> SetPlayerSnowBoard_Hook(0x4E9660);

static void SetPlayerSnowBoard_m()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			auto char_id = TASKWK_CHARID(ptwp);

			switch (char_id)
			{
			case Characters_Sonic:
				SetInputP(i, PL_OP_SNOWBOARDING);
				CreateElementalTask(3u, 2, (TaskFuncPtr)0x4959E0)->twp->counter.b[0] = i;
				break;
			case Characters_Tails:
				SetInputP(i, PL_OP_SNOWBOARDING);
				CreateElementalTask(3u, 2, (TaskFuncPtr)0x461510)->twp->counter.b[0] = i;
				break;
			}
		}
	}
}

static void __cdecl SetPlayerSnowBoard_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		SetPlayerSnowBoard_m();
		FreeTask(tp);
	}
	else
	{
		SetPlayerSnowBoard_Hook.Original(tp);
	}
}

void patch_snowboard_init()
{
	SetPlayerSnowBoard_Hook.Hook(SetPlayerSnowBoard_r);
}

RegisterPatch patch_snowboard(patch_snowboard_init);
#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "patch_boss_common.h"
#include "levels.h"
#include "result.h"

FastFunctionHook<void, task*> E101_Main_t(E101_Main);

static void __cdecl Rd_E101_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		//WRONG
		auto twp = tp->twp;

		auto hit_twp = CCL_IsHitPlayer(twp);
		if (hit_twp && TASKWK_CHARID(hit_twp) != Characters_Gamma) // allow other characters to hurt enemy
		{
			SetWinnerMulti(TASKWK_PLAYERID(hit_twp));
			twp->flag |= Status_Hurt;
		}

		E101_Main_t.Original(tp);
	}
}

void patch_e101_init()
{
	E101_Main_t.Hook(Rd_E101_r);
}

#ifdef MULTI_TEST
RegisterPatch patch_e101(patch_e101_init);
#endif
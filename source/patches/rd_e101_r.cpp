#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "boss-common.h"
#include "levels.h"
#include "result.h"

FastFunctionHook<void, task*> E101R_Main_t(0x56C0B0);

static void __cdecl Rd_E101_R_r(task* tp)
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

		E101R_Main_t.Original(tp);
	}
}

void patch_e101r_init()
{
	E101R_Main_t.Hook(Rd_E101_R_r);
}

#ifdef MULTI_TEST
RegisterPatch patch_e101r(patch_e101r_init);
#endif
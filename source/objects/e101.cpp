#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "bosses.h"
#include "levels.h"
#include "result.h"

#ifdef MULTI_TEST
static void __cdecl Rd_E101_r(task* tp);
FastFunctionHook<void, task*> E101_Main_t(E101_Main, Rd_E101_r);
static void __cdecl Rd_E101_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
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
#endif

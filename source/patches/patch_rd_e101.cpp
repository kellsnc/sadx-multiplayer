#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "result.h"

FastFunctionHook<void, task*> E101_execTgt_h(0x567fd0);

static void __cdecl E101_execTgt_r(task* tp)
{
	auto twp = tp->twp;

	auto hit_twp = CCL_IsHitPlayer(twp);
	if (hit_twp && TASKWK_CHARID(hit_twp) != Characters_Gamma) // allow other characters to hurt enemy
	{
		SetWinnerMulti(TASKWK_PLAYERID(hit_twp));
		twp->flag |= Status_Hurt;
	}

	E101_execTgt_h.Original(tp);
}

void patch_e101_init()
{
	E101_execTgt_h.Hook(E101_execTgt_r);
}

#ifdef MULTI_TEST
RegisterPatch patch_e101(patch_e101_init);
#endif
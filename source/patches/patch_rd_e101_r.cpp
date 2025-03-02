#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "result.h"

FastFunctionHook<void, task*> execE101R_h(0x56C0B0);

static void __cdecl execE101R_r(task* tp)
{
	auto twp = tp->twp;

	auto hit_twp = CCL_IsHitPlayer(twp);
	if (hit_twp && TASKWK_CHARID(hit_twp) != Characters_Gamma) // allow other characters to hurt enemy
	{
		SetWinnerMulti(TASKWK_PLAYERID(hit_twp));
		twp->flag |= Status_Hurt;
	}

	execE101R_h.Original(tp);
}

void patch_e101r_init()
{
	execE101R_h.Hook(execE101R_r);
}

#ifdef MULTI_TEST
RegisterPatch patch_e101r(patch_e101r_init);
#endif
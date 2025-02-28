#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "patch_boss_common.h"

// Egg Walker

FastFunctionHook<void, task*> Egm2_t(0x576650);

static const int timeLimit = 600;

void Egm2_r(task* tp)
{
	if (tp->twp && !tp->twp->mode)
		ResetBossRNG();

	Egm2_t.Original(tp);
	Boss_SetNextPlayerToAttack(timeLimit);
}

void patch_egm3_init()
{
	Egm2_t.Hook(Egm2_r);
}

#ifdef MULTI_TEST
RegisterPatch patch_egm3(patch_egm3_init);
#endif
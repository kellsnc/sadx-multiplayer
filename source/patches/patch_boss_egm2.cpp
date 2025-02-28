#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "patch_boss_common.h"
#include "levels.h"

FastFunctionHook<void, task*> eggWalker_t(0x576650);

static const int timeLimit = 600;

void eggWalker_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		return MultiArena(tp);
	}

	if (tp->twp && !tp->twp->mode)
		ResetBossRNG();

	eggWalker_t.Original(tp);
	Boss_SetNextPlayerToAttack(timeLimit);
}

void patch_egm3_init()
{
	eggWalker_t.Hook(eggWalker_r);
}

#ifdef MULTI_TEST
RegisterPatch patch_egm3(patch_egm3_init);
#endif
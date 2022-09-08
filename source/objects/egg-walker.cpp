#include "pch.h"
#include "bosses.h"

static FunctionHook<void, task*> eggWalker_t(0x576650);

static const int timeLimit = 600;


void eggWalker_r(task* tp)
{
	if (tp->twp && !tp->twp->mode)
		ResetBossRNG();

	eggWalker_t.Original(tp);
	Boss_SetNextPlayerToAttack(timeLimit);
}

void initEggWalkerPatches()
{
	eggWalker_t.Hook(eggWalker_r);
}
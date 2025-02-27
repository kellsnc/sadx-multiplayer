#include "pch.h"
#include "multiplayer.h"
#include "result.h"

static void __cdecl uptarget_exec_wait_r(task* tp);
static void __cdecl dummytarget_exec_r(task* tp);

FastFunctionHookPtr<decltype(&uptarget_exec_wait_r)> uptarget_exec_wait_t(0x5B5740);
FastFunctionHookPtr<decltype(&dummytarget_exec_r)> dummytarget_exec_t(0x5B5960);

static void __cdecl uptarget_exec_wait_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (IsPlayerInSphere(&twp->pos, twp->scl.x))
		{
			twp->mode = 1;
			twp->wtimer = 7;
		}
	}
	else
	{
		uptarget_exec_wait_t.Original(tp);
	}
}

static void __cdecl dummytarget_exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto hit_twp = CCL_IsHitBullet(tp->twp);

		// Set the winner when a bullet collides
		// The last collision will be actual winner
		if (hit_twp)
		{
			SetWinnerMulti(hit_twp->btimer); // player number is stored in btimer thanks to patch in E102.cpp
		}
	}

	dummytarget_exec_t.Original(tp);
}

void patch_finalegg_uptarget_init()
{
	uptarget_exec_wait_t.Hook(uptarget_exec_wait_r);
	dummytarget_exec_t.Hook(dummytarget_exec_r);
}

RegisterPatch patch_finalegg_uptarget(patch_finalegg_uptarget_init);
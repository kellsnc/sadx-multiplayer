#include "pch.h"
#include "multiplayer.h"
#include "result.h"

static void __cdecl ObjectFrogCollision_r(task* tp);
static void __cdecl ObjectFrog_r(task* tp);
static void __cdecl ObjectSandFrog_r(task* tp);

FastUsercallHookPtr<decltype(&ObjectFrogCollision_r), noret, rEAX> ObjectFrogCollision_h(0x4FA2C0);
FastFunctionHookPtr<decltype(&ObjectFrog_r)> ObjectFrog_h(0x4FA320);
FastFunctionHookPtr<decltype(&ObjectSandFrog_r)> ObjectSandFrog_h(0x598040);

static void ObjectFrogCollision_m(taskwk* twp)
{
	auto pnum = IsPlayerInSphere(&twp->pos, (twp->scl.x + 1.0f) * 14.0f) - 1;

	if (pnum >= 0)
	{
		SetWinnerMulti(pnum);
		SetFinishAction();
		twp->mode = 3;
	}
}

static void __cdecl ObjectFrogCollision_r(task* tp)
{
	if (multiplayer::IsBattleMode())
	{
		ObjectFrogCollision_m(tp->twp);
	}
	else
	{
		ObjectFrogCollision_h.Original(tp);
	}
}

static void __cdecl ObjectFrog_r(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 2)
	{
		ObjectFrogCollision_m(twp);
	}

	ObjectFrog_h.Original(tp);
}

static void __cdecl ObjectSandFrog_r(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 2)
	{
		ObjectFrogCollision_m(twp);
	}

	ObjectSandFrog_h.Original(tp);
}

void patch_frog_init()
{
	ObjectFrogCollision_h.Hook(ObjectFrogCollision_r);
	ObjectFrog_h.Hook(ObjectFrog_r);
	ObjectSandFrog_h.Hook(ObjectSandFrog_r);
}

RegisterPatch patch_frog(patch_frog_init);
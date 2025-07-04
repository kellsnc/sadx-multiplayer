#include "pch.h"
#include "multiplayer.h"

FastFunctionHook<void, task*> dispMirrorTaskP_h(0x61E1B0);
FastFunctionHook<void, task*> execMirrorTaskP_h(0x61E230);

static auto getMirror = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x61E070, rESI);

static void dispMirrorTaskp_m(task* tp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertp[i])
		{
			getMirror(playertp[i]);
		}
	}
}

static void __cdecl dispMirrorTaskP_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		dispMirrorTaskp_m(tp);
	}
	else
	{
		dispMirrorTaskP_h.Original(tp);
	}
}

static void execMirrorTaskP_m(task* tp)
{
	auto awp = tp->awp;

	if (awp->work.ul[0] == 0u)
	{
		awp->work.ul[0] = 1u;
		tp->disp = (TaskFuncPtr)0x61E1B0;
	}

	if (GetStageNumber() != LevelAndActIDs_TwinklePark3)
	{
		FreeTask(tp);
	}

	tp->disp(tp);
}

static void __cdecl execMirrorTaskP_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		execMirrorTaskP_m(tp);
	}
	else
	{
		execMirrorTaskP_h.Original(tp);
	}
}

void patch_mirror_init()
{
	dispMirrorTaskP_h.Hook(dispMirrorTaskP_r);
	execMirrorTaskP_h.Hook(execMirrorTaskP_r);
}

RegisterPatch patch_mirror(patch_mirror_init);
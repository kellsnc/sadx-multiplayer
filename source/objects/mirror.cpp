#include "pch.h"
#include "multiplayer.h"

Trampoline* dispMirrorTaskP_t = nullptr;
Trampoline* execMirrorTaskP_t = nullptr;

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
		TARGET_DYNAMIC(dispMirrorTaskP)(tp);
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
		TARGET_DYNAMIC(execMirrorTaskP)(tp);
	}
}

void PatchTwinkleMirrors()
{
	dispMirrorTaskP_t = new Trampoline(0x61E1B0, 0x61E1B8, dispMirrorTaskP_r);
	WriteCall((void*)((int)dispMirrorTaskP_t->Target() + 3), (void*)0x441AC0); // Patch trampoline
	execMirrorTaskP_t = new Trampoline(0x61E230, 0x61E235, execMirrorTaskP_r);
	WriteCall((void*)((int)execMirrorTaskP_t->Target()), (void*)0x4258F0); // Patch trampoline
}
#include "pch.h"
#include "multiplayer.h"
#include "result.h"

static void ObjectFrogCollision_w();
static void __cdecl ObjectFrog_r(task* tp);
static void __cdecl ObjectSandFrog_r(task* tp);

Trampoline ObjectFrogCollision_t(0x4FA2C0, 0x4FA2C7, ObjectFrogCollision_w);
Trampoline ObjectFrog_t(0x4FA320, 0x4FA325, ObjectFrog_r);
Trampoline ObjectSandFrog_t(0x598040, 0x598045, ObjectSandFrog_r);

static void ObjectFrogCollision_o(task* tp)
{
	auto target = ObjectFrogCollision_t.Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

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
	if (multiplayer::IsActive())
	{
		ObjectFrogCollision_m(tp->twp);
	}
	else
	{
		return ObjectFrogCollision_o(tp);
	}
}

static void __declspec(naked) ObjectFrogCollision_w()
{
	__asm
	{
		push eax
		call ObjectFrogCollision_r
		pop eax
		retn
	}
}

static void __cdecl ObjectFrog_r(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 2)
	{
		ObjectFrogCollision_m(twp);
	}

	TARGET_STATIC(ObjectFrog)(tp);
}

static void __cdecl ObjectSandFrog_r(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 2)
	{
		ObjectFrogCollision_m(twp);
	}

	TARGET_STATIC(ObjectSandFrog)(tp);
}
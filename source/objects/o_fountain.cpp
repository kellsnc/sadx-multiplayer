#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "UsercallFunctionHandler.h"

UsercallFunction<void, taskwk*> execFountain(0x61B8B0, rESI);
UsercallFunction<void, taskwk*> expandAndConstrict(0x61B800, rESI);

//UsercallFuncVoid(execFountain, (taskwk* twp), (twp), 0x61B8B0, rESI);
//UsercallFuncVoid(expandAndConstrict, (taskwk* twp), (twp), 0x61B800, rESI);

static void pushUp_m(task* tp, int pnum)
{
	auto awp = tp->awp;
	auto twp = tp->twp;
	auto ptwp = playertwp[pnum];

	dsPlay_timer(106, (int)tp, 1, 0, 30);
	SetSpringVelocityP(pnum, awp->work.f[0], awp->work.f[1], awp->work.f[2]);
	SetRotationP(pnum, 0, twp->ang.y + 0x8000, 0);

	ptwp->pos.x = twp->pos.x;
	ptwp->pos.z = twp->pos.z;
	if (ptwp->pos.y - twp->pos.y < 20.0f)
	{
		ptwp->pos.y = twp->pos.y + 20.0f;
	}
}

static void ObjectFountain_m(task* tp)
{
	auto twp = tp->twp;

	dsPlay_timer_v(105, (int)twp, 1, 0, 30, twp->pos.x, twp->pos.y, twp->pos.z);

	bool up = false;
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (!ptwp)
			continue;

		Float x = ptwp->pos.x - twp->pos.x;
		Float y = ptwp->pos.y - twp->pos.y;
		Float z = ptwp->pos.z - twp->pos.z;

		if (njSqrt(z * z + x * x) <= 15.0f && y > 0.0f && y < twp->scl.y * 100.0f)
		{
			up = true;
			pushUp_m(tp, i);
			twp->mode = 2;
			if (twp->smode == 0)
				twp->wtimer = 0;
			twp->smode = 1;
		}
	}

	if (up == false && twp->mode == 2)
	{
		twp->mode = 1;
		twp->wtimer = 0;
	}

	execFountain(twp);
	expandAndConstrict(twp);
	LoopTaskC(tp);
	EntryColliList(twp);

	if (++twp->btimer > 3)
	{
		twp->btimer = 0;
	}

	tp->disp(tp);
}

static void ObjectFoutain_r(task* tp);
FastFunctionHook<void, task*> ObjectFoutain_h(0x61BDC0, ObjectFoutain_r);
static void ObjectFoutain_r(task* tp)
{
	auto twp = tp->twp;
	if (multiplayer::IsActive() && (twp->mode == 1 || twp->mode == 2))
	{
		ObjectFountain_m(tp);
	}
	else
	{
		ObjectFoutain_h.Original(tp);
	}
}

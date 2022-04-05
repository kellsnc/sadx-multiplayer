#include "pch.h"
#include "UsercallFunctionHandler.h"
#include "multiplayer.h"

// Reverse Loop in Lost World

#define LOOP_PNUM(twp) twp->smode

static auto SetPath2Taskwk = GenerateUsercallWrapper<void (*)(pathtag* ptag, taskwk* twp, float onpathpos)>(noret, 0x5E3860, rEDX, rESI, stack4);

static void ReverseRuinLoop_m(task* tp)
{
	auto twp = tp->twp;

	if (GetStageNumber() != twp->timer.w[0])
	{
		FreeTask(tp);
		return;
	}

	if (!multiplayer::IsActive())
	{
		return;
	}

	auto ptag = (pathtag*)twp->value.ptr;
	auto pnum = LOOP_PNUM(twp);
	auto ptwp = playertwp[pnum];

	if (!ptwp)
	{
		return;
	}

	switch (twp->mode)
	{
	case 0i8:
		if (twp->mode == 0i8)
		{
			twp->mode = 1i8;
			twp->wtimer = 0i16;
			twp->scl.x = 0.0f;
			twp->pos.x = 6950.0f;
			twp->pos.y = -2440.0f;
			twp->pos.z = 993.0f;
			CCL_Init(tp, (CCL_INFO*)0x2038D10, 1, 4u);
			SetPath2Taskwk(ptag, twp, twp->scl.x);
			twp->scl.y = ptag->totallen - 10.0f;
		}
		break;
	case 1i8:
		if ((twp->cwp->flag & 1) && twp->cwp->hit_cwp == ptwp->cwp)
		{
			SetInputP(pnum, PL_OP_PLACEWITHSPIN);
			twp->mode = 2i8;
		}
		EntryColliList(twp);
		break;
	case 2i8:
		twp->scl.x += 15.0f;

		SetPath2Taskwk(ptag, twp, twp->scl.x);

		ptwp->pos = twp->pos;
		ptwp->ang.x = twp->ang.x;
		ptwp->ang.y = 0x4000 - twp->ang.y;
		ptwp->ang.z = twp->ang.z;

		if (twp->scl.x > (double)twp->scl.y)
		{
			SetVelocityP(pnum, njCos(0x4000 - twp->ang.y) * 3.0f, 1.0f, njSin(0x4000 - twp->ang.y) * -3.0f);
			SetInputP(pnum, PL_OP_LETITGO);
			FreeTask(tp);
		}
		break;
	}
}

static void __cdecl ReverseRuinLoop_r(task* tp);
Trampoline ReverseRuinLoop_t(0x5E3960, 0x5E3967, ReverseRuinLoop_r);
static void __cdecl ReverseRuinLoop_r(task* tp)
{
	if (tp->twp->smode == 0)
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto ctp = CreateElementalTask(2u, LEV_2, ReverseRuinLoop_m);
			*ctp->twp = *tp->twp;
			LOOP_PNUM(ctp->twp) = i;
		}

		tp->twp->smode = 1;
	}

	if (!multiplayer::IsActive())
	{
		TARGET_STATIC(ReverseRuinLoop)(tp);
	}
}
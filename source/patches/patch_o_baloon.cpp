#include "pch.h"
#include "multiplayer.h"
#include "result.h"

#define BALOON_PNUM(twp) twp->smode

enum : __int8
{
	MODE_INITIAL,
	MODE_NORMAL,
	MODE_DOWN,
	MODE_HOLD,
	MODE_END
};

static void __cdecl ObjectBaloon_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectBaloon_r)> ObjectBaloon_h(0x7A21C0);

static void Normal_m(taskwk* twp)
{
	twp->value.f = njSin((twp->counter.l + 800) << 8) * 4.0f;
	twp->ang.z = static_cast<Angle>(njCos(200 * (twp->counter.l + 2000)) * 4096.0f);
	twp->scl.x = 1.0f - njCos(twp->counter.l << 11) * 0.05f;
	twp->scl.y = 1.0f - njSin(twp->counter.l << 11) * 0.05f;
	++twp->counter.l;

	auto pnum = IsPlayerInSphere(twp->pos.x, twp->pos.y + twp->value.f - 22.0f, twp->pos.z, 8.0f) - 1;
	if (pnum >= 0)
	{
		BALOON_PNUM(twp) = pnum;
		twp->mode = MODE_DOWN;
		SetInputP(pnum, PL_OP_PLACEON);
		twp->counter.f = -1.5f;
		SleepTimer();
		SetWinnerMulti(pnum);
		SetFinishAction();
	}

	EntryColliList(twp);
}

static void Down_m(taskwk* twp)
{
	twp->value.f += twp->counter.f;

	auto ptwp = playertwp[BALOON_PNUM(twp)];
	if (ptwp)
	{
		ptwp->pos = { twp->pos.x, twp->pos.y + twp->value.f - 24.0f, twp->pos.z };
	}

	twp->counter.f += 0.15f;
	if (twp->counter.f > 0.0f)
	{
		twp->mode = MODE_HOLD;
	}
}

static void Hold_m(taskwk* twp)
{
	twp->ang.z = static_cast<Angle>(njCos(200 * (twp->counter.l + 2000)) * 4096.0f);
	twp->scl.x = 1.0f - njCos(twp->counter.l << 11) * 0.05f;
	twp->scl.y = 1.0f - njSin(twp->counter.l << 11) * 0.05f;

	if (twp->value.f < 344.0)
	{
		twp->value.f += (njSin(200 * twp->counter.l) * 0.5f + 0.3f) * 0.5f;
	}

	++twp->counter.l;

	auto ptwp = playertwp[BALOON_PNUM(twp)];
	if (ptwp)
	{
		ptwp->pos = { twp->pos.x, twp->pos.y + twp->value.f - 24.0f, twp->pos.z };
	}

	if (EV_CheckCansel())
		twp->mode = MODE_END;
}

static void ObjectBaloon_m(task* tp)
{
	if (!CheckRangeOut(tp))
	{
		auto twp = tp->twp;

		switch (twp->mode)
		{
		case MODE_INITIAL:
			tp->disp = (TaskFuncPtr)0x7A1E10;
			CCL_Init(tp, (CCL_INFO*)0x38CD140, 2, 4u);

			twp->mode = MODE_NORMAL;
			twp->counter.l = static_cast<int>(njRandom() * 100.0);
			twp->value.l = 0;
			twp->ang.z = 0;
			twp->scl.x = 1.0f;
			twp->scl.y = 1.0f;
			twp->scl.z = 1.0f;
			break;
		case MODE_NORMAL:
			Normal_m(twp);
			break;
		case MODE_DOWN:
			Down_m(twp);
			break;
		case MODE_HOLD:
			Hold_m(twp);
			break;
		case MODE_END:
			DeadOut(tp);
			break;
		default:
			twp->mode = 0i8;
			break;
		}

		tp->disp(tp);
	}
}

static void __cdecl ObjectBaloon_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectBaloon_m(tp);
	}
	else
	{
		ObjectBaloon_h.Original(tp);
	}
}

void patch_baloon_init()
{
	ObjectBaloon_h.Hook(ObjectBaloon_r);
}

RegisterPatch patch_baloon(patch_baloon_init);
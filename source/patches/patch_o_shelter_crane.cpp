#include "pch.h"
#include "multiplayer.h"

// Rewrite ObjShelterCrane by placing detected player in twp->btimer

enum MD_CRANE // made up
{
	MDCRANE_SWITCH,
	MDCRANE_DOWN,
	MDCRANE_GRAB,
	MDCRANE_UP_GRABBED,
	MDCRANE_UP_EMPTY,
	MDCRANE_UP_RESET,
	MDCRANE_SIDE,
	MDCRANE_END,
	MDCRANE_RELEASE,
	MDCRANE_RETURN,
	MDCRANE_RESET
};

static auto sub_5A4F10 = GenerateUsercallWrapper<void (*)(taskwk* twp)>(noret, 0x5A4F10, rECX); //inline
static auto sub_5A4E30 = GenerateUsercallWrapper<Bool(*)(float*, float, float)>(rEAX, 0x5A4E30, rECX, stack4, stack4); //inline
static auto sub_5A4E60 = GenerateUsercallWrapper<Bool(*)(float*, float, float)>(rEAX, 0x5A4E60, rECX, stack4, stack4); //inline
static auto SetColli = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5A4E90, rEAX);

static void __cdecl ObjShelterCrane_r(task* tp);
FastFunctionHookPtr<decltype(&ObjShelterCrane_r)> ObjShelterCrane_t(0x5A5110);

static void ObjShelterCrane_m(task* tp)
{
	auto twp = tp->twp;
	auto mwp = tp->mwp;

	if (twp->wtimer > 0)
	{
		--twp->wtimer;
		EntryColliList(twp);
		tp->disp(tp);
		return;
	}

	switch (twp->mode)
	{
	case MDCRANE_SWITCH:
		if (GetSwitchOnOff(mwp->work.w[0]))
		{
			twp->wtimer = 20;
			twp->mode = MDCRANE_DOWN;
			sub_5A4F10(twp);
			SetColli(tp);
		}
		break;
	case MDCRANE_DOWN:
		if (sub_5A4E60(&twp->counter.f, 1.0f, twp->scl.y))
		{
			twp->wtimer = 40;
			twp->mode = MDCRANE_GRAB;
		}
		break;
	case MDCRANE_GRAB:
		mwp->ang_spd.x += 224;

		if (mwp->ang_spd.x >= 5632)
		{
			auto pnum = IsPlayerInSphere(twp->pos.x + mwp->spd.x, twp->pos.y + twp->counter.f - 113.0f, twp->pos.z + mwp->spd.z, 15.0f) - 1;

			if (pnum >= 0)
			{
				twp->btimer = pnum;
				SetInputP(pnum, PL_OP_PLACEON);
				mwp->spd.y = playertwp[pnum]->pos.y - (twp->pos.y + twp->counter.f);
				twp->cwp->info[1].attr |= 0x10u;
				twp->cwp->info[2].attr |= 0x10u;
				twp->mode = MDCRANE_UP_GRABBED;
				twp->smode = 1;
				twp->wtimer = 20;
			}
			else
			{
				twp->mode = MDCRANE_UP_EMPTY;
				twp->wtimer = 20;
			}
		}

		SetColli(tp);
		break;
	case MDCRANE_UP_GRABBED:
		if (sub_5A4E30(&twp->counter.f, 1.0f, 0.0f))
		{
			twp->wtimer = 40;
			twp->timer.l = 0;
			twp->mode = MDCRANE_SIDE;
		}
		SetColli(tp);
		break;
	case MDCRANE_UP_EMPTY:
		if (sub_5A4E30(&twp->counter.f, 1.0f, 0.0f))
		{
			twp->mode = MDCRANE_UP_RESET;
		}
		SetColli(tp);
		break;
	case MDCRANE_UP_RESET:
		mwp->ang_spd.x -= 224;

		if (mwp->ang_spd.x <= 0)
		{
			mwp->ang_spd.x = 0;
			twp->mode = MDCRANE_SWITCH;
			twp->wtimer = 20;
		}

		SetColli(tp);
		break;
	case MDCRANE_SIDE:
		if (sub_5A4E30(&twp->timer.f, 2.0f, mwp->rad))
		{
			twp->pos.x = njSin(twp->ang.y) * twp->scl.x + mwp->height;
			twp->pos.z = njCos(twp->ang.y) * twp->scl.x + mwp->weight;
			twp->mode = MDCRANE_END;
			twp->wtimer = 40;
		}
		else
		{
			twp->pos.x = mwp->acc.x + twp->pos.x;
			twp->pos.z = mwp->acc.z + twp->pos.z;
		}
		break;
	case MDCRANE_END:
		twp->smode = 0;
		SetInputP(twp->btimer, PL_OP_LETITGO);
		twp->mode = MDCRANE_RELEASE;
		sub_5A4F10(twp);
		break;
	case MDCRANE_RELEASE:
		mwp->ang_spd.x -= 224;

		if (mwp->ang_spd.x <= 0)
		{
			mwp->ang_spd.x = 0;
			twp->mode = MDCRANE_RETURN;
			twp->wtimer = 20;
		}

		SetColli(tp);
		break;
	case MDCRANE_RETURN:
		if (sub_5A4E60(&twp->timer.f, 2.0f, 0.0f))
		{
			twp->pos.x = mwp->height;
			twp->pos.z = mwp->weight;
			twp->mode = MDCRANE_RESET;
		}
		else
		{
			twp->pos.x -= mwp->acc.x;
			twp->pos.z -= mwp->acc.z;
		}
		break;
	case MDCRANE_RESET:
		twp->mode = MDCRANE_SWITCH;
		break;
	}

	if (twp->smode == 1)
	{
		SetPositionP(twp->btimer, twp->pos.x + mwp->spd.x, twp->pos.y + mwp->spd.y + twp->counter.f, twp->pos.z + mwp->spd.z);
	}

	EntryColliList(twp);
	tp->disp(tp);
}

static void __cdecl ObjShelterCrane_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjShelterCrane_m(tp);
	}
	else
	{
		ObjShelterCrane_t.Original(tp);
	}
}

void patch_shelter_crane_init()
{
	ObjShelterCrane_t.Hook(ObjShelterCrane_r);
}

RegisterPatch patch_shelter_crane(patch_shelter_crane_init);
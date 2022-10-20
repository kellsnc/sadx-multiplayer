#include "pch.h"
#include "multiplayer.h"
#include "e_cart.h"

enum : __int8
{
	MODE_WAIT,
	MODE_1,
	MODE_SLIDE,
	MODE_STOP,
	MODE_FIX,
};

struct stopperwk // custom
{
	__int8 mode;
	float prev_frame;
	float frame;
	float frame_increment;
	float max_frame;
	NJS_ACTION* action;
	NJS_TEXLIST* texlist;
	CCL_INFO* colli;
	NJS_OBJECT* dyncol;
	NJS_POINT3 pos;
	float dyncol_range;
	NJS_OBJECT* object;
	task* cart_tp;
};

static auto setCart = GenerateUsercallWrapper<BOOL(*)(task* tp)>(rEAX, 0x7B1240, rEAX);
static auto slideStopper = GenerateUsercallWrapper<BOOL(*)(task* tp)>(rEAX, 0x7B1040, rEAX);

static void execObject_m(task* tp)
{
	auto twp = tp->twp;
	auto wk = (stopperwk*)tp->mwp;

	wk->prev_frame = wk->frame;
	*(NJS_POINT3*)wk->dyncol->pos = wk->pos;
	MirenObjCheckCollisionP(twp, wk->dyncol_range);

	switch (wk->mode)
	{
	case MODE_WAIT:
	{
		auto ptwp = CCL_IsHitPlayer(twp);

		if (ptwp)
		{
			auto cart_tp = CartChangeForceMode(TASKWK_PLAYERID(ptwp));
			if (cart_tp)
			{
				wk->cart_tp = cart_tp;
				wk->mode = MODE_1;
			}
		}
		EntryColliList(twp);
		break;
	}
	case MODE_1:
		if (setCart(tp))
		{
			MirenSoundPlayOneShotSE(85, &twp->pos, nullptr);
			wk->mode = MODE_SLIDE;
		}
		break;
	case MODE_SLIDE:
		if (slideStopper(tp))
		{
			wk->mode = MODE_STOP;
		}
		break;
	case MODE_STOP:
		CartGetOffPlayer(wk->cart_tp);
		MirenSoundPlayOneShotSE(9, &twp->pos, nullptr);
		wk->mode = MODE_FIX;
		break;
	}

	tp->disp(tp);
}

static void __cdecl ObjectCartStopper_execObject_r(task* tp);
Trampoline ObjectCartStopper_execObject_t(0x7B13D0, 0x7B13D5, ObjectCartStopper_execObject_r);
static void __cdecl ObjectCartStopper_execObject_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		execObject_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjectCartStopper_execObject)(tp);
	}
}
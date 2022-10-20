#include "pch.h"
#include "multiplayer.h"

// Shutter door in Twinkle Park Act 1

enum : __int8
{
	MODE_0,
	MODE_1,
	MODE_2,
	MODE_3,
	MODE_4
};

static bool IsPlayerInCart()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertwp[i] && playertwp[i]->smode == PL_OP_PLACEWITHCART)
		{
			return true;
		}
	}
	return false;
}

static void ObjectTPShutter_m(task* tp)
{
	if (!CheckRangeOutWithR(tp, 1020100.0f))
	{
		auto twp = tp->twp;

		switch (twp->mode)
		{
		case MODE_0:
			if (twp->scl.x >= 0.0f)
			{
				twp->mode = MODE_1;
			}
			else
			{
				twp->mode = MODE_2;
			}
			twp->value.f = twp->pos.y + 64.6f;
			twp->scl.x = 0.0f;
			twp->scl.z = 0.0f;
			twp->scl.y = 0.0f;
			twp->timer.f = twp->pos.y;
			twp->smode = MODE_0;
			twp->counter.ptr = (void*)0x027C17CC;
			tp->disp = (TaskFuncPtr)0x624280;
			CCL_Init(tp, (CCL_INFO*)0x27C6910, 1, 4u);
			SET_COLLI_RANGE(twp->cwp, 102.8f);
			break;
		case MODE_1:
			if (IsPlayerInSphere(&twp->pos, 80.0f))
			{
				twp->smode = twp->mode;
				twp->mode = MODE_3;
			}

			twp->wtimer += 16ui16;
			if (twp->wtimer >= 1024ui16)
			{
				twp->wtimer = 0ui16;
			}

			break;
		case MODE_2:
			if (IsPlayerInCart())
			{
				twp->smode = twp->mode;
				twp->mode = MODE_3;
			}

			twp->wtimer += 16ui16;
			if (twp->wtimer >= 1024ui16)
			{
				twp->wtimer = 0ui16;
			}

			break;
		case MODE_3:
			if (!IsPlayerInCart() && twp->smode == MODE_2)
			{
				twp->mode = MODE_4;
			}

			if (++twp->btimer > 120i8)
			{
				twp->btimer = 120i8;
				twp->pos.y += 0.5f;
			}

			if (twp->pos.y <= twp->value.f)
			{
				dsPlay_timer_v(88, (int)twp, 1, 0, 2, twp->pos.x, twp->pos.y, twp->pos.z);
			}
			else
			{
				twp->pos.y = twp->value.f;
			}

			twp->wtimer -= 16ui16;
			if (twp->wtimer == 0ui16)
			{
				twp->wtimer = 1024ui16;
			}

			break;
		case 4i8:
			twp->pos.y -= 0.5f;
			if (twp->pos.y >= twp->timer.f)
			{
				dsPlay_timer_v(88, (int)twp, 1, 0, 2, twp->pos.x, twp->pos.y, twp->pos.z);
			}
			else
			{
				twp->pos.y = twp->timer.f;
				twp->mode = MODE_2;
				twp->btimer = 0i8;
			}

			if (IsPlayerInCart())
			{
				twp->mode = MODE_3;
			}

			twp->wtimer += 16ui16;
			if (twp->wtimer >= 1024ui16)
			{
				twp->wtimer = 0ui16;
			}

			break;
		}

		EntryColliList(twp);
		tp->disp(tp);
	}
}

static void __cdecl ObjectTPShutter_r(task* tp);
Trampoline ObjectTPShutter_t(0x624380, 0x624386, ObjectTPShutter_r);
static void __cdecl ObjectTPShutter_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectTPShutter_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjectTPShutter)(tp);
	}
}
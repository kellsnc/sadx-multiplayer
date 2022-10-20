#include "pch.h"
#include "multiplayer.h"

enum : __int8
{
	MODE_INIT,
	MODE_NORMAL,
	MODE_END
};

static auto Initial = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x60DDB0, rEDI); // custom name original is "init_9"

static bool GetPressurePoint(task* tp, NJS_POINT3* pt)
{
	bool test = false;
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (CheckPlayerRideOnMobileLandObjectP(i, tp))
		{
			auto ptwp = playertwp[i];

			if (!test)
			{
				*pt = ptwp->pos;
				test = true;
			}
			else
			{
				pt->x = (pt->x + ptwp->pos.x) / 2.0f;
				pt->y = (pt->y + ptwp->pos.y) / 2.0f;
				pt->z = (pt->z + ptwp->pos.z) / 2.0f;
			}
		}
	}

	return test;
}

static void Normal_m(task* tp)
{
	auto twp = tp->twp;
	auto dyncol = (NJS_OBJECT*)twp->counter.ptr;

	if (!(!rd_mountain_twp || rd_mountain_twp->scl.y == 0.0f && rd_mountain_twp->counter.f == 0.0f))
	{
		twp->pos.y += rd_mountain_twp->counter.f;
		dyncol->pos[1] = twp->pos.y;
	}

	Angle newx = 0;
	Angle newz = 0;

	NJS_POINT3 pt;
	if (GetPressurePoint(tp, &pt))
	{
		if (twp->btimer == 0)
		{
			twp->value.f = -0.035f;
		}

		uint16_t idk = 200 / (max(0, twp->btimer) + 1);
		twp->wtimer += idk;

		if (idk >= 1)
			++twp->btimer;
		else
			twp->value.f = 0.0f;

		float x = pt.x - twp->pos.x;
		float y = pt.y - twp->pos.y;
		float z = pt.z - twp->pos.z;

		if (x <= 0.0f)
		{
			newz = NJM_RAD_ANG(atan2(abs(x), y));
		}
		else
		{
			newz = NJM_RAD_ANG(-atan2(x, y));
		}
		newz = (Angle)((float)newz * 0.083333336f);

		if (z <= 0.0f)
		{
			newx = NJM_RAD_ANG(-atan2(abs(z), y));
		}
		else
		{
			newx = NJM_RAD_ANG(atan2(z, y));
		}
		newx = (Angle)((float)newx * 0.083333336f);
	}
	else
	{
		twp->btimer = 0i8;
		twp->wtimer = 0ui16;
	}

	twp->ang.x = AdjustAngle(twp->ang.x, 2 * newx, 15);
	twp->ang.z = AdjustAngle(twp->ang.z, 2 * newz, 15);

	dyncol->ang[0] = twp->ang.x;
	dyncol->ang[2] = twp->ang.z;

	MirenObjCheckCollisionP(twp, 90.0f);
}

static void ObjectMountainAsiba_m(task* tp)
{
	if (CheckRangeOut(tp))
	{
		return;
	}

	auto twp = tp->twp;

	if (twp->mode == MODE_INIT)
	{
		tp->dest = (TaskFuncPtr)0x541360; // "End"
		tp->disp = (TaskFuncPtr)0x60DEA0; // "Draw"
		Initial(tp);
		twp->mode = MODE_NORMAL;

		if (rd_mountain_twp)
		{
			twp->pos.y = rd_mountain_twp->pos.y;
		}
	}
	else
	{
		Normal_m(tp);
	}

	tp->disp(tp);
}

static void __cdecl ObjectMountainAsiba_r(task* obj);
Trampoline ObjectMountainAsiba_t(0x60E150, 0x60E156, ObjectMountainAsiba_r);
static void __cdecl ObjectMountainAsiba_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectMountainAsiba_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjectMountainAsiba)(tp);
	}
}
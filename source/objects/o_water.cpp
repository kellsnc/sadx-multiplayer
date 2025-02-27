#include "pch.h"
#include "camera.h"
#include "multiplayer.h"

// Waterslide in Lost World

#define WATER_PNUM(twp) twp->smode

static void __cdecl ObjectWater_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectWater_r)> ObjectWater_t(0x5E3830);

static void __cdecl Wait_m(task* tp);

static void __cdecl Normal2_m(task* tp)
{
	auto twp = tp->twp;
	auto pnum = WATER_PNUM(twp);
	auto ptwp = playertwp[pnum];

	if (ssActNumber != 1)
	{
		FreeTask(tp);
		return;
	}

	if (ptwp)
	{
		twp->wtimer -= 256ui16;

		ptwp->pos.y -= njSin(twp->wtimer) * 5.0f;
		ptwp->pos.z -= njCos(twp->wtimer) * 5.0f;

		if (twp->wtimer < 257ui16)
		{
			SetInputP(pnum, PL_OP_LETITGO);
			Angle3 ang = { 0, 0x4000, 0 };
			NJS_POINT3 v = { 0.0f, -1.0f, 0.0f };
			SetVelocityAndRotationAndNoconTimeP(0, &v, &ang, 60);
			tp->exec = Wait_m;
		}
	}
	else
	{
		tp->exec = Wait_m;
	}
}

static void __cdecl Normal_m(task* tp)
{
	auto twp = tp->twp;
	auto pnum = WATER_PNUM(twp);
	auto ptwp = playertwp[pnum];
	auto ppwp = playerpwp[pnum];

	if (ssActNumber != 1)
	{
		FreeTask(tp);
		return;
	}

	if (ptwp && ppwp)
	{
		// Speed:
		if (twp->scl.y < 6.0f && twp->scl.x > 100.0f)
		{
			twp->scl.y += 0.1f;
		}

		// End of path:
		if (twp->scl.x >= 2521.0f)
		{
			CameraReleaseEventCamera_m(pnum);
			twp->scl.y = 1.0;
			twp->wtimer = 18204;
			tp->exec = Normal2_m;
			return;
		}

		// Movement:
		auto x = (float)(per[pnum]->x1 << 8);
		if (x >= -3072.0f)
		{
			if (x > 3072.0f)
			{
				twp->scl.z -= 0.5f;
				if (twp->scl.z < -15.0f)
				{
					twp->scl.z = -15.0f;
				}
			}
		}
		else
		{
			twp->scl.z += 0.5f;
			if (twp->scl.z > 15.0f)
			{
				twp->scl.z = 15.0f;
			}
		}

		twp->scl.x += twp->scl.y;

		pathinfo pi1, pi2;
		pi1.onpathpos = twp->scl.x;
		GetStatusOnPath((pathtag*)0x20343BC, &pi1);
		pi2.onpathpos = twp->scl.x - 40.0f;
		GetStatusOnPath((pathtag*)0x20343BC, &pi2);

		twp->pos.x = pi1.xpos;
		twp->pos.y = pi1.ypos;
		twp->pos.z = pi1.zpos;

		twp->ang.y = NJM_RAD_ANG(atan2(pi1.front.x, pi1.front.z));
		ptwp->ang.y = 0x4000 - twp->ang.y;

		auto sin = njSin(twp->ang.y + 0x4000) * twp->scl.z;
		ptwp->pos.x = twp->counter.f;
		twp->counter.f = twp->pos.x + sin;

		auto cos = njCos(twp->ang.y + 0x4000) * twp->scl.z;
		ptwp->pos.z = twp->value.f;
		twp->value.f = twp->pos.z + sin;

		auto height = ppwp->shadow.y_bottom;
		if (height == -1000000.0f)
		{
			height = ppwp->shadow.y_top;
		}
		ptwp->pos.y = height - 2.0f;
		if (twp->pos.y - ptwp->pos.y > 10.0f)
		{
			ptwp->pos.y = twp->pos.y - 4.0f;
		}

		NJS_POINT3 pos = { twp->counter.f, twp->pos.y + 2.0f, twp->value.f };
		NJS_POINT3 velo = { 0.0f, 0.2f, 0.0f };
		CreateWater(&pos, &velo, 1.0f);

		NJS_POINT3 cam_pos, cam_tgt;
		cam_pos.x = pi2.xpos + sin * 0.5f;
		cam_pos.y = pi2.ypos + 6.0f;
		cam_pos.z = pi2.zpos + cos * 0.5f;
		cam_tgt.x = pi1.xpos + sin * 0.5f;
		cam_tgt.y = pi1.ypos + 6.0f;
		cam_tgt.z = pi1.zpos + cos * 0.5f;

		SetRuinWaka1Data(&cam_pos, &cam_tgt, pnum);
	}
	else
	{
		tp->exec = Wait_m;
	}
}

static void __cdecl Wait_m(task* tp)
{
	auto twp = tp->twp;
	auto pnum = WATER_PNUM(twp);
	auto ptwp = playertwp[pnum];

	if (ssActNumber != 1)
	{
		FreeTask(tp);
		return;
	}

	if (ptwp)
	{
		if ((ptwp->pos.x > 270.0f && ptwp->pos.x < 480.0f) && (ptwp->pos.z > 1660.0f && ptwp->pos.z < 1725.0f) && ptwp->pos.y < 140.0f)
		{
			NJS_POINT3 onpnt3;
			SCPathPntNearToOnpos((pathtag*)0x20343BC, &ptwp->pos, &onpnt3, &twp->scl.x);

			twp->counter.f = ptwp->pos.x;
			twp->value.f = ptwp->pos.z;
			SetInputP(pnum, PL_OP_PLACEWITHRAFT);
			CameraSetEventCamera_m(pnum, CAMMD_RuinWaka1, CAMADJ_RELATIVE1);
			dsPlay_oneshot(198, 0, 0, 0);
			tp->exec = Normal_m;
		}
	}
}

static void __cdecl ObjectWater_r(task* tp)
{
	tp->exec = Wait_m;

	for (int i = 1; i < PLAYER_MAX; ++i)
	{
		auto ctp = CreateElementalTask(2u, LEV_2, Wait_m);
		*ctp->twp = *tp->twp;
		WATER_PNUM(ctp->twp) = i;
	}
}

void patch_water_init()
{
	ObjectWater_t.Hook(ObjectWater_r);
}

RegisterPatch patch_water(patch_water_init);
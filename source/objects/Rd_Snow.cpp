#include "pch.h"
#include "camera.h"
#include "multiplayer.h"

DataPointer(NJS_POINT3, pos_7E9628, 0x7E9628);
DataPointer(NJS_POINT3, pos_7E9634, 0x7E9634);
DataPointer(NJS_POINT3, pos_7E964C, 0x7E964C);
static NJS_POINT3 pos_spdman = { 132.0f, 375.0f, -42.0f };
static NJS_VECTOR vel_spdman = { 4.0f, 0.4f, 0.0f };
static Angle3 ang_spdman = { 0, 0xB200, 0 };

static void __cdecl Rd_Snow_r(task* tp);
Trampoline Rd_Snow_t(0x4E9D90, 0x4E9D97, Rd_Snow_r);

static void __cdecl RdSnowBoardingSpeedManager_m(task* tp)
{
	auto awp = tp->awp;
	auto& timer = awp->work.ul[0];
	auto pnum = awp->work.ul[1];
	auto ppwp = playerpwp[pnum];
	
	if (ppwp)
	{
		if (ppwp->spd.x > 4.0f) ppwp->spd.x = 4.0f;
	}

	if (++timer > 100)
	{
		FreeTask(tp);
	}
}

static void RdSnowCheckEnterIceCave_m(task* tp)
{
	auto twp = tp->twp;
	auto pnum = IsPlayerInsideSphere(&pos_7E9628, 35.0f) - 1;

	if (pnum >= 0)
	{
		ChangeActM(1);
		twp->mode = 2;

		if (GetPlayerNumber() != 7)
		{
			auto jointp = CreateElementalTask(6u, 3, (TaskFuncPtr)0x4EBDF0)->twp;
			jointp->pos.x = 1075.0f;
			jointp->pos.y = 336.0f;
			jointp->pos.z = 336.0f;
		}

		ADXTaskInit();
		BGM_Play(53);
		dsPlay_iloop(991, -1, 8, 0);
	}

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp && ptwp->pos.y < -475.0f && !(playerpwp[i]->item & Powerups_Dead))
		{
			KillHimByFallingDownP(i);
		}
	}
}

static void RdSnowCheckExitIceCave_m(task* tp)
{
	auto pnum = IsPlayerInsideSphere(&pos_7E9634, 35.0f) - 1;
	if (pnum >= 0)
	{
		ChangeActM(1);
		tp->twp->mode = 0;
	}
}

static void RdSnowBoardingRegular_m(task* tp)
{
	bool done = true;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (!ptwp || ptwp->smode == 24)
		{
			continue;
		}

		// Detach
		if (GetDistance(&pos_7E964C, &ptwp->pos) < 200.0f)
		{
			ResetPerspective_m(i);
			SetInputP(i, PL_OP_LETITGO);
			CameraReleaseEventCamera();
			continue;
		}

		done = false;

		// Speed limiter for somewhere, spammed somehow
		if ((ptwp->flag & Status_Ground) && GetDistance(&pos_spdman, &ptwp->pos) < 30.0f)
		{
			SetVelocityAndRotationAndNoconTimeP(0, &vel_spdman, &ang_spdman, 60);
			auto awp = CreateElementalTask(8u, 2, RdSnowBoardingSpeedManager_m)->awp;
			awp->work.ul[1] = i;
		}

		float deg = fabsf(playerpwp[i]->spd.x);

		if (deg < 7.82f)
		{
			deg *= 0.12787724f;
		}
		else
		{
			deg = 1.0f;
		}

		njSetPerspective_m(i, AdjustAngle(ds_GetPerspective_m(i), 0x31C7 - static_cast<Angle>(sqrtf(deg) * -5461.0f), 512));
	}

	if (done == true)
	{
		tp->twp->mode = 9;
	}
}

static void __cdecl Rd_Snow_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		
		NadareStart = FALSE;

		switch (twp->mode)
		{
		case 0:
			RdSnowInit(tp);
			break;
		case 1:
			RdSnowCheckEnterIceCave_m(tp);
			break;
		case 2:
			RdSnowCheckExitIceCave_m(tp);
			break;
		case 8:
			RdSnowBoardingRegular_m(tp);
			break;
		}
	}
	else
	{
		TARGET_STATIC(Rd_Snow)(tp);
	}
}
#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "multiplayer.h"
#include "splitscreen.h"
#include "camera.h"

DataPointer(NJS_POINT3, pos_7E9628, 0x7E9628);
DataPointer(NJS_POINT3, pos_7E9634, 0x7E9634);
DataPointer(NJS_POINT3, pos_7E964C, 0x7E964C);
static NJS_POINT3 pos_spdman = { 132.0f, 375.0f, -42.0f };
static NJS_VECTOR vel_spdman = { 4.0f, 0.4f, 0.0f };
static NJS_VECTOR vel_hardcodedgap = { 14.0f, 0.4f, 0.0f };
static Angle3 ang_spdman = { 0, 0xB200, 0 };

FastFunctionHookPtr<TaskFuncPtr> Rd_Snow_h(0x4E9D90);
FastFunctionHookPtr<TaskFuncPtr> SetPlayerSnowBoard_Hook(0x4E9660);
FastFunctionHook<void, task*> dispBgSnow_h(0x4E9950);

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
			CameraReleaseEventCamera_m(i);
			continue;
		}

		done = false;

		if ((ptwp->flag & Status_Ground))
		{
			// Controlled speed at the beginning
			if (GetDistance(&pos_spdman, &ptwp->pos) < 30.0f)
			{
				SetVelocityAndRotationAndNoconTimeP(i, &vel_spdman, &ang_spdman, 60);
				auto awp = CreateElementalTask(8u, 2, RdSnowBoardingSpeedManager_m)->awp;
				awp->work.ul[1] = i;
			}

			// Since the avalanch is not there, we hardcode a speed to pass the pit
			else if (ptwp->pos.x < -5362.0f && ptwp->pos.x > -5462.0f && ptwp->pos.y > -4000.0f)
			{
				SetVelocityAndRotationAndNoconTimeP(i, &vel_hardcodedgap, &ang_spdman, 60);
			}
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

			if (twp->mode == 8)
			{
				// Reset camera when dying in multiplayer mode
				SetDefaultNormalCameraMode(CAMMD_SNOWBOARD, CAMADJ_THREE1);

				// Snowboard camera for other players
				for (int i = 1; i < PLAYER_MAX; ++i)
				{
					CameraSetEventCameraFunc_m(i, CameraSnowboard, CAMADJ_THREE1, CDM_LOOKAT);
				}
			}

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
		Rd_Snow_h.Original(tp);
	}
}

static void SetPlayerSnowBoard_m()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			auto char_id = TASKWK_CHARID(ptwp);

			switch (char_id)
			{
			case Characters_Sonic:
				SetInputP(i, PL_OP_SNOWBOARDING);
				CreateElementalTask(3u, 2, (TaskFuncPtr)0x4959E0)->twp->counter.b[0] = i;
				break;
			case Characters_Tails:
				SetInputP(i, PL_OP_SNOWBOARDING);
				CreateElementalTask(3u, 2, (TaskFuncPtr)0x461510)->twp->counter.b[0] = i;
				break;
			}
		}
	}
}

static void __cdecl SetPlayerSnowBoard_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		SetPlayerSnowBoard_m();
		FreeTask(tp);
	}
	else
	{
		SetPlayerSnowBoard_Hook.Original(tp);
	}
}

// Fix display masks
static void __cdecl dispBgSnow_r(task* tp)
{
	dispBgSnow_h.Original(tp);

	if (camera_twp && tp->twp->mode == 10 && splitscreen::IsActive())
	{
		NJS_POINT3* pos = &camera_twp->pos;

		if (pos->y <= 200.0f)
		{
			if (pos->y < 0.0f)
			{
				if (pos->x >= 1612.0f)
				{
					MaskBlock = 0x1C;
				}
				else
				{
					MaskBlock = 0x6;
				}
			}
			else
			{
				if (pos->x >= 1666.0f)
				{
					MaskBlock = 0x18;
				}
				else
				{
					MaskBlock = 0x6;
				}
			}
		}
		else if (pos->x >= 1666.0f)
		{
			MaskBlock = 0x78;
		}
		else
		{
			MaskBlock = 0x62;
		}
	}
}

void patch_rd_snow_init()
{
	Rd_Snow_h.Hook(Rd_Snow_r);
	SetPlayerSnowBoard_Hook.Hook(SetPlayerSnowBoard_r);
	dispBgSnow_h.Hook(dispBgSnow_r);
	
	WriteData((uint8_t*)0x4E91C0, 0xC3ui8); // Remove landtable collision chunk optimisation
	WriteData((void**)0x4EA26E, (void*)0x4EA1D0); // Patch skybox mode
	WriteData((uint8_t*)0x4E91AE, (uint8_t)PLAYER_MAX); // Breath generator for all players
}

RegisterPatch patch_rd_snow(patch_rd_snow_init);
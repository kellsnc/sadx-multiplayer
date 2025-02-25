#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "VariableHook.hpp"
#include "utils.h"
#include "multiplayer.h"
#include "camera.h"

VariableHook<int, 0x3C80620> inwind_timer_m;
VariableHook<Bool, 0x3C80618> windshadow_m;
VariableHook<int, 0x3C80608> EC_mode_m;

static void __cdecl ObjectSkydeck_Wall_r(task* tp);
static void __cdecl RdSkydeckWind_r(__int16 act);
static void __cdecl Skydeck_EggcarrierCtrl_r(__int16 act);

FastFunctionHookPtr<decltype(&ObjectSkydeck_Wall_r)> ObjectSkydeck_Wall_t(0x5EF2B0, ObjectSkydeck_Wall_r);
FastFunctionHookPtr<decltype(&RdSkydeckWind_r)> RdSkydeckWind_t(0x5EF300, RdSkydeckWind_r);
FastFunctionHookPtr<decltype(&Skydeck_EggcarrierCtrl_r)> Skydeck_EggcarrierCtrl_t(0x5ECA80, Skydeck_EggcarrierCtrl_r);

static void Skydeck_EggcarrierCtrl_m(__int16 act)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		SetFreeCameraMode_m(i, FALSE);

		auto ptwp = playertwp[i];

		if (!ptwp)
			continue;

		auto ppwp = playerpwp[i];
		auto posx = ptwp->pos.x;

		if (posx < -1600.0f && !(ppwp->item & Powerups_Dead))
		{
			KillHimByFallingDownP(i);
		}

		if (CurrentCharacter == Characters_Knuckles)
		{
			continue;
		}

		// Skip mode checks if the button is pressed
		if (GetSwitchOnOff(0))
		{
			continue;
		}

		auto& EC_mode = EC_mode_m[i];

		// Manage mode for current player
		switch (EC_mode)
		{
		case 0:
			if (posx > -500.0f)
			{
				EC_mode = 1;
			}
			break;
		case 1:
			if (posx < -500.0f)
			{
				EC_mode = 0;
			}
			if (posx > -200.0f)
			{
				EC_mode = 3;
			}
			if (SkyDeck_SkyRotationX >= 3640 && SkyDeck_SkyRotationX < 0x8000)
			{
				EC_mode = 2;
			}
			break;
		case 2:
			if (posx < -500.0f)
			{
				EC_mode = 0;
			}
			if (posx > -200.0f)
			{
				EC_mode = 3;
			}
			if (SkyDeck_SkyRotationX > 0x8000 && SkyDeck_SkyRotationX <= 0xF1C7)
			{
				EC_mode = 1;
			}
			break;
		case 3:
			if (posx < -200.0f)
			{
				EC_mode = 1;
			}
			if (posx > 225.0f)
			{
				EC_mode = 4;
			}
			break;
		case 4:
			if (posx < -500.0f)
			{
				EC_mode = 3;
			}
			break;
		default:
			EC_mode = 0;
			break;
		}
	}

	if (CurrentCharacter == Characters_Knuckles)
	{
		return;
	}

	if (GetSwitchOnOff(0))
	{
		// Force gravity when button is pressed
		cloud_pitch = AdjustAngle(cloud_pitch, 0, 64);
		cloud_roll = AdjustAngle(cloud_roll, 0, 64);

		// Turn off the button if all players pass the check
		bool turnoff = true;
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto ptwp = playertwp[i];
			if (ptwp && ptwp->pos.x < 225.0f && ptwp->pos.y > -25.0f)
			{
				turnoff = false;
			}
		}

		if (turnoff)
		{
			SetSwitchOnOff(0, 0);
		}
	}
	else
	{
		// Get highest mode
		int target_mode = 0;
		for (auto& i : EC_mode_m)
		{
			if (i > target_mode)
				target_mode = i;
		}

		// Turn ship:
		switch (target_mode)
		{
		case 0:
			cloud_roll = AdjustAngle(cloud_roll, 61895, 16);
			cloud_pitch = AdjustAngle(cloud_pitch, 0, 16);
			break;
		case 1:
			cloud_roll = AdjustAngle(cloud_roll, 3640, 16);
			cloud_pitch = AdjustAngle(cloud_pitch, 0, 16);
			break;
		case 2:
			cloud_roll = AdjustAngle(cloud_roll, 61895, 16);
			cloud_pitch = AdjustAngle(cloud_pitch, 0, 16);
			break;
		case 3:
			cloud_roll = AdjustAngle(cloud_roll, 0, 16);
			cloud_pitch = AdjustAngle(cloud_pitch, 0, 16);
			break;
		case 4:
			cloud_roll = AdjustAngle(cloud_roll, 0, 16);
			cloud_pitch = AdjustAngle(cloud_pitch, 49334, 64);
			break;
		}
	}

	SetUserGravityXZ(cloud_roll, cloud_pitch);
}

static void __cdecl Skydeck_EggcarrierCtrl_r(__int16 act)
{
	if (multiplayer::IsActive() && act == 2)
	{
		Skydeck_EggcarrierCtrl_m(act);
	}
	else
	{
		Skydeck_EggcarrierCtrl_t.Original(act);
	}
}

static void RdSkydeckWind_m(__int16 act)
{
	if (act != 1)
		return;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (!ptwp)
			continue;

		auto pmwp = playermwp[i];
		auto& inwind_timer = inwind_timer_m[i];
		auto& windshadow = windshadow_m[i];

		float v1 = (SkyDeck_SkyPosition.y - 1350.0f - 50.0f) * 0.005f;
		float pow_y, pow_x;

		if (v1 < 0.0f)
		{
			if (v1 < -1.0f)
			{
				inwind_timer = 0;
				v1 = -1.0f;
			}

			pow_y = 0.001f * (v1 + 1.0f) + 0.0005f;
			pow_x = (v1 + 1.0f) * 0.005f;
		}
		else
		{
			if (v1 > 1.0f)
			{
				inwind_timer = 0;
				v1 = 1.0f;
			}

			pow_y = 0.0015f - 0.001f * v1;
			pow_x = 0.005f - v1 * 0.005f;
		}

		if (ptwp->pos.z > -3100.0f)
		{
			pow_y *= 0.5f;
			pow_x *= 0.3f;
		}

		Angle3 ang{};
		auto ground = GetShadowPos(ptwp->pos.x, ptwp->pos.y + 2.5f, ptwp->pos.z, &ang);
		if (ptwp->pos.y + 2.5f - ground < 200.0f)
		{
			ground = 0.0f;
		}
		ground = ptwp->pos.y - ground;

		if (ground > 40.0f || windshadow)
		{
			inwind_timer -= 4;
			if (inwind_timer < 0)
			{
				inwind_timer = 0;
			}
		}
		else if (++inwind_timer > 90)
		{
			inwind_timer = 90;
		}

		float accy = (float)inwind_timer * pow_y;

		if (pmwp->acc.y < accy)
		{
			pmwp->acc.y += accy;
		}

		auto accx = inwind_timer ? ((max(0.0f, min(200.0f, ground))) + 100.0f) * pow_x : 0;

		if (pmwp->acc.x < accx)
		{
			pmwp->acc.x += accx;
		}

		windshadow = FALSE;
	}
}

static void __cdecl RdSkydeckWind_r(__int16 act)
{
	if (multiplayer::IsActive())
	{
		RdSkydeckWind_m(act);
	}
	else
	{
		RdSkydeckWind_t.Original(act);
	}
}

static void __cdecl ObjectSkydeck_Wall_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 1)
	{
		auto twp = tp->twp;
		auto player = CCL_IsHitPlayer(twp);

		if (player)
		{
			windshadow_m[TASKWK_PLAYERID(player)] = TRUE;
		}
	}
	else
	{
		ObjectSkydeck_Wall_t.Original(tp);
	}
}
#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

MAKEVARMULTI(int, inwind_timer, 0x3C80620);
MAKEVARMULTI(BOOL, windshadow, 0x3C80618);

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
		auto& inwind_timer = *inwind_timer_m[i];
		auto& windshadow = *windshadow_m[i];

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
			pow_x = 0.005 - v1 * 0.005f;
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

static void __cdecl RdSkydeckWind_r(__int16 act);
Trampoline RdSkydeckWind_t(0x5EF300, 0x5EF309, RdSkydeckWind_r);
static void __cdecl RdSkydeckWind_r(__int16 act)
{
	if (multiplayer::IsActive())
	{
		RdSkydeckWind_m(act);
	}
	else
	{
		TARGET_STATIC(RdSkydeckWind)(act);
	}
}

static void __cdecl ObjectSkydeck_Wall_r(task* tp);
Trampoline ObjectSkydeck_Wall_t(0x5EF2B0, 0x5EF2B5, ObjectSkydeck_Wall_r);
static void __cdecl ObjectSkydeck_Wall_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 1)
	{
		auto twp = tp->twp;
		auto player = CCL_IsHitPlayer(twp);

		if (player)
		{
			*windshadow_m[TASKWK_PLAYERID(player)] = TRUE;
		}
	}
	else
	{
		TARGET_STATIC(ObjectSkydeck_Wall)(tp);
	}
}

#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "VariableHook.hpp"
#include "camera.h"

static void ColliNoWaterRange_m(task* tp)
{
	auto twp = tp->twp;

	if (CheckRangeOut(tp))
	{
		return;
	}

	if (twp->mode == 0i8)
	{
		twp->mode = 1i8; // now that's usefull
	}
	else if (twp->mode == 1i8)
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto ptwp = playertwp[i];

			if (ptwp)
			{
				if (ptwp->cwp)
				{
					if (CS_CheckCollisionPlayer2NoWater(ptwp->cwp->info, twp))
					{
						playerpwp[i]->shadow.Attr_top &= ~(ColFlags_Water | 0x400000);
					}
				}
			}

			auto cam_pos = GetCameraPosition(i);

			if (cam_pos)
			{
				camera_sphere_info.center = *cam_pos;
				if (CS_CheckCollisionPlayer2NoWater(&camera_sphere_info, twp))
				{
					flagCameraNoUnderWater_m[i] = TRUE;
				}
			}
		}
	}
	else
	{
		twp->mode = 0i8;
	}
}

static void __cdecl ColliNoWaterRange_r(task* tp);
FastFunctionHookPtr<decltype(&ColliNoWaterRange_r)> ColliNoWaterRange_t(0x4D4E10, ColliNoWaterRange_r);
static void __cdecl ColliNoWaterRange_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ColliNoWaterRange_m(tp);
	}
	else
	{
		ColliNoWaterRange_t.Original(tp);
	}
}
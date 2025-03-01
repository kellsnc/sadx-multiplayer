#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "sadx_utils.h"

// Red Mountain lava adjustments
// - Lava can now go down
// - Players below lava level are teleported to the surface
// - Replaced shape collision with geometry collision

#define TARGET rd_mountain_twp->scl.x
#define SPEED rd_mountain_twp->scl.y

static void __cdecl Object_Mountain_Yougan_r(task* tp);
FastFunctionHookPtr<decltype(&Object_Mountain_Yougan_r)> Object_Mountain_Yougan_h(0x602670);

static void __cdecl Object_Mountain_Yougan_m(task* tp)
{
	auto twp = tp->twp;

	switch (twp->mode)
	{
	case 0:
	{
		twp->mode = 1;
		tp->disp = (TaskFuncPtr)0x602530;
		tp->dest = (TaskFuncPtr)0x59DBF0;
		twp->pos.x = -1468.0f;
		twp->pos.z = 1559.0f;

		Float height = 28.0f;
		if (GetPlayerNumber() != Characters_Gamma)
		{
			height = 130.0f;
		}
		twp->scl.z = height;
		twp->pos.y = height;
		rd_mountain_twp->pos.y = height;
		rd_mountain_twp->scl.x = height;

		// We replace shape collision with a geometry collision, more stable
		auto object = GetMobileLandObject();
		if (object)
		{
			object->pos[0] = twp->pos.x;
			object->pos[1] = twp->pos.y;
			object->pos[2] = twp->pos.z;
			object->ang[0] = 0;
			object->ang[1] = 0;
			object->ang[2] = 0;
			object->scl[0] = 1.0f;
			object->scl[1] = 1.0f;
			object->scl[2] = 1.0f;
			object->basicdxmodel = ((NJS_OBJECT*)0x2484D5C)->basicdxmodel;
			RegisterCollisionEntry(ColFlags_Solid | ColFlags_Dynamic | ColFlags_Hurt, tp, object);
			twp->counter.ptr = object;
		}

		break;
	}
	case 1:
		twp->pos.z = 1559.0f;
		twp->pos.x = -1468.0f;

		if (rd_mountain_twp)
		{
			twp->pos.y += SPEED;

			if ((SPEED >= 0.0f && twp->pos.y >= TARGET) || (SPEED < 0.0f && twp->pos.y <= TARGET))
			{
				twp->pos.y = twp->scl.z = TARGET;
				rd_mountain_twp->scl.y = 0.0f;
				twp->mode = 3;
			}

			rd_mountain_twp->pos.y = twp->pos.y;
			rd_mountain_twp->counter.f = rd_mountain_twp->scl.y;
		}
		dsPlay_timer(SE_DM_MAGMAUP, (int)twp, 1, 0, 30);
		break;
	case 3:
	{
		Float offset = njSin(NJM_DEG_ANG(++twp->wtimer)) * 0.7f;
		rd_mountain_twp->counter.f = offset + twp->scl.z - twp->pos.y;
		twp->pos.y = rd_mountain_twp->pos.y = offset + twp->scl.z;
		if (rd_mountain_twp->scl.y != 0.0f)
		{
			twp->mode = 1;
		}
		break;
	}
	}

	// Move geometry collision up
	auto col = (NJS_OBJECT*)twp->counter.ptr;
	if (col)
	{
		col->pos[1] = twp->pos.y;
	}

	// If some players are below lava level, teleport above
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];
		if (ptwp && ptwp->pos.y < twp->pos.y - 5.0f)
		{
			ptwp->pos.y = twp->pos.y;
			ptwp->wtimer = 240;
		}
	}

	object_youUP_yogun_yogun.basicdxmodel->mats->attr_texId = *(Uint8*)0x3C80F94;
	tp->disp(tp);
}

static void __cdecl Object_Mountain_Yougan_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		Object_Mountain_Yougan_m(tp);
		tp->exec = Object_Mountain_Yougan_m;
	}
	else
	{
		Object_Mountain_Yougan_h.Original(tp);
	}
}

void patch_mountain_yougan_init()
{
	Object_Mountain_Yougan_h.Hook(Object_Mountain_Yougan_r);
}

RegisterPatch patch_mountain_yougan(patch_mountain_yougan_init);
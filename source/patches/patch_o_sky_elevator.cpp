#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl objSkyEVStep_r(task* tp);
FastFunctionHookPtr<decltype(&objSkyEVStep_r)> objSkyEVStep_t(0x5F5A70);

static void __cdecl objSkyEVStep_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 1)
	{
		auto twp = tp->twp;
		checkSkyEVColli(twp);
		tp->disp(tp);

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto ptwp = playertwp[i];

			if (!ptwp)
				continue;

			NJS_POINT3 v;
			v.x = ptwp->pos.x - twp->pos.x;
			v.y = ptwp->pos.y - twp->pos.y - 17.56f;
			v.z = ptwp->pos.z - twp->pos.z;
			njPushMatrix(_nj_unit_matrix_);
			if (twp->ang.y != 0x10000)
			{
				ROTATEY(0, -twp->ang.y);
			}
			njCalcVector(0, &v, &v);
			njPopMatrixEx();
			if ((ptwp->flag & 1) && fabs(v.x) < 18.14f && fabs(v.y) < 10.0f && fabs(v.z) < 16.53f)
			{
				twp->mode = 2i8;
			}
		}
	}
	else
	{
		objSkyEVStep_t.Original(tp);
	}
}

void patch_sky_elevator_init()
{
	objSkyEVStep_t.Hook(objSkyEVStep_r);
}

RegisterPatch patch_sky_elevator(patch_sky_elevator_init);
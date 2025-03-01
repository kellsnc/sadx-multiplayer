#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

// Patches the carousel floor to work with multiple players

static void __cdecl execTPFloor_r(task* tp);
FastFunctionHookPtr<decltype(&execTPFloor_r)> execTPFloor_t(0x6210B0);

static void execTPFloor_m(task* tp)
{
	auto twp = tp->twp;
	Angle add_ang = tp->ptp->twp->wtimer;

	twp->ang.y += add_ang;

	MirenObjCheckCollisionP(twp, 200.0f);

	if (twp->flag & 0x100)
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (CheckPlayerRideOnMobileLandObjectP(i, tp))
			{
				// For some reason they did not use forcewk here

				auto ptwp = playertwp[i];

				NJS_POINT3 v;
				v.x = ptwp->pos.x - twp->pos.x;
				v.y = 0.0f;
				v.z = ptwp->pos.z - twp->pos.z;
				njPushMatrix(_nj_unit_matrix_);
				ROTATEY(0, add_ang);
				njCalcVector(0, &v, &v);
				njPopMatrixEx();
				ptwp->pos.x = v.x + twp->pos.x;
				ptwp->pos.z = v.z + twp->pos.z;
			}
		}
	}

	tp->disp(tp);
}

static void __cdecl execTPFloor_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 1)
	{
		execTPFloor_m(tp);
	}
	else
	{
		execTPFloor_t.Original(tp);
	}
}

void patch_twinkle_go_round_init()
{
	execTPFloor_t.Hook(execTPFloor_r);
}

RegisterPatch patch_twinkle_go_round(patch_twinkle_go_round_init);
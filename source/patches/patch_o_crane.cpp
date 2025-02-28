#include "pch.h"
#include "multiplayer.h"

// Crane platform in Speed Highway

static Bool __cdecl checkPlayerRideOnTheCage_r(task* tp);
static void __cdecl execCage_r(task* tp);

FastUsercallHookPtr<decltype(&checkPlayerRideOnTheCage_r), rEAX, rEAX> checkPlayerRideOnTheCage_t(0x61B060);
FastFunctionHookPtr<decltype(&execCage_r)> execCage_t(0x61B190);

static Bool __cdecl checkPlayerRideOnTheCage_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto ptwp = playertwp[i];

			if (ptwp && ptwp->flag & Status_Ground)
			{
				NJS_POINT3 p;
				p.x = ptwp->pos.x - twp->pos.x;
				p.y = ptwp->pos.y - twp->pos.y - 20.0f;
				p.z = ptwp->pos.z - twp->pos.z;

				njPushMatrix(_nj_unit_matrix_);
				if (twp->ang.y != 0x10000)
				{
					njRotateY(0, (uint16_t)-LOWORD(twp->ang.y));
				}
				njCalcVector(nullptr, &p, &p);
				njPopMatrixEx();

				if (fabs(p.x) < 20.0f && fabs(p.y) < 10.0f && fabs(p.z) < 20.0f)
				{
					return TRUE;
				}
			}
		}

		return FALSE;
	}
	else
	{
		return checkPlayerRideOnTheCage_t.Original(tp);
	}
}

static void __cdecl execCage_r(task* tp)
{
	execCage_t.Original(tp);

	for (int i = 2; i < PLAYER_MAX; ++i)
	{
		if (CheckPlayerRideOnMobileLandObjectP(i, tp))
		{
			tp->fwp[i] = tp->fwp[0];
		}
	}
}

void patch_crane_init()
{
	checkPlayerRideOnTheCage_t.Hook(checkPlayerRideOnTheCage_r);
	execCage_t.Hook(execCage_r);
}

RegisterPatch patch_crane(patch_crane_init);
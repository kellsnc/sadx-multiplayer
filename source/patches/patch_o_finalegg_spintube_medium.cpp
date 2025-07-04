#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl SpinTubeMedium_ExecATask_r(task* tp);
FastUsercallHookPtr<decltype(&SpinTubeMedium_ExecATask_r), noret, rEAX> SpinTubeMedium_ExecATask_h(0x5BCBA0);

static void ExecATask_m(task* tp)
{
	auto twp = tp->twp;
	auto fwp = tp->fwp;

	MirenObjCheckCollisionP(twp, 310.0f);

	// Rotate tube
	twp->wtimer += 512;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		NJS_POINT3 ppos;
		GetPlayerPosition(i, 0, &ppos, nullptr);

		njPushMatrix(_nj_unit_matrix_);
		njTranslateEx(&twp->pos);
		ROTATEZ(0, twp->ang.z);
		ROTATEY(0, twp->ang.y);
		ROTATEX(0, twp->ang.x);
		njPushMatrixEx();
		njInvertMatrix(0);
		NJS_POINT3 v;
		njCalcPoint(0, &ppos, &v);
		njPopMatrixEx();
		njPushMatrix(_nj_unit_matrix_);
		njRotateZ(0, 512);
		NJS_POINT3 v2;
		njCalcVector(0, &v, &v2);
		njPopMatrixEx();
		njSubVector(&v2, &v);
		njCalcVector(0, &v2, &fwp[i].pos_spd);
		njPopMatrixEx();
	}
}

static void __cdecl SpinTubeMedium_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp);
	}
	else
	{
		SpinTubeMedium_ExecATask_h.Original(tp);
	}
}

void patch_finalegg_spintube_medium_init()
{
	SpinTubeMedium_ExecATask_h.Hook(SpinTubeMedium_ExecATask_r);
}

RegisterPatch patch_finalegg_spintube_medium(patch_finalegg_spintube_medium_init);
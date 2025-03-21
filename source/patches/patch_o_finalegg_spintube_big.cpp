#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl SpinTubeBig_ExecATask_r(task* tp);
FastUsercallHookPtr<decltype(&SpinTubeBig_ExecATask_r), noret, rEAX> SpinTubeBig_ExecATask_h(0x5BCED0);

static void ExecATask_m(task* tp)
{
	auto twp = tp->twp;
	auto fwp = tp->fwp;

	MirenObjCheckCollisionP(twp, 340.0f);

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

static void __cdecl SpinTubeBig_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp);
	}
	else
	{
		SpinTubeBig_ExecATask_h.Original(tp);
	}
}

void patch_finalegg_spintube_big_init()
{
	SpinTubeBig_ExecATask_h.Hook(SpinTubeBig_ExecATask_r);
}

RegisterPatch patch_finalegg_spintube_big(patch_finalegg_spintube_big_init);
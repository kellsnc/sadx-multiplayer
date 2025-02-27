#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl SpinTubeSmall_ExecATask_r(task* tp);
FastUsercallHookPtr<decltype(&SpinTubeSmall_ExecATask_r), noret, rEAX> SpinTubeSmall_ExecATask_t(0x5BC870);

static void ExecATask_m(task* tp)
{
	auto twp = tp->twp;
	auto fwp = tp->fwp;

	MirenObjCheckCollisionP(twp, 280.0f);

	// Rotate tube
	twp->wtimer += 512;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		NJS_POINT3 ppos;
		GetPlayerPosition(i, 0, &ppos, nullptr);

		njPushMatrix(_nj_unit_matrix_);
		njTranslateEx(&twp->pos);
		njRotateZ_(twp->ang.z);
		njRotateY_(twp->ang.y);
		njRotateX_(twp->ang.x);
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

static void __cdecl SpinTubeSmall_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp);
	}
	else
	{
		SpinTubeSmall_ExecATask_t.Original(tp);
	}
}

void patch_finalegg_spintube_small_init()
{
	SpinTubeSmall_ExecATask_t.Hook(SpinTubeSmall_ExecATask_r);
}

RegisterPatch patch_finalegg_spintube_small(patch_finalegg_spintube_small_init);
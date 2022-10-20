#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

static void SpinTubeBig_ExecATask_w();
Trampoline SpinTubeBig_ExecATask_t(0x5BCED0, 0x5BCED7, SpinTubeBig_ExecATask_w);

static void SpinTubeBig_ExecATask_o(task* tp)
{
	auto target = SpinTubeBig_ExecATask_t.Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

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

static void __cdecl SpinTubeBig_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp);
	}
	else
	{
		SpinTubeBig_ExecATask_o(tp);
	}
}

static void __declspec(naked) SpinTubeBig_ExecATask_w()
{
	__asm
	{
		push eax
		call SpinTubeBig_ExecATask_r
		pop eax
		retn
	}
}
#include "pch.h"
#include "multiplayer.h"

DataPointer(NJS_OBJECT, object_kaiten_kaitendaiz_kaitendaiz, 0x18300C8);

static void ExecATask_w();
Trampoline ObjShelterKaitendaiExecATask_t(0x59E760, 0x59E767, ExecATask_w);

static void ExecATask_o(task* tp)
{
	auto target = ObjShelterKaitendaiExecATask_t.Target();
	__asm
	{
		mov edi, [tp]
		call target
	}
}

static void ExecATask_m(task* tp)
{
	auto twp = tp->twp;
	auto fwp = tp->fwp;
	auto col = (NJS_OBJECT*)twp->value.ptr;
	auto ang_spd = static_cast<Angle>(twp->scl.x);

	twp->ang.y += ang_spd;

	col->ang[1] = twp->ang.y;
	col->pos[0] = twp->pos.x + object_kaiten_kaitendaiz_kaitendaiz.pos[0];
	col->pos[1] = twp->pos.y + object_kaiten_kaitendaiz_kaitendaiz.pos[1];
	col->pos[2] = twp->pos.z + object_kaiten_kaitendaiz_kaitendaiz.pos[2];

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (CheckPlayerRideOnMobileLandObjectP(i, tp))
		{
			auto ptwp = playertwp[i];

			NJS_VECTOR pos;
			NJS_VECTOR dir = { ptwp->pos.x - twp->pos.x, 0.0f, ptwp->pos.z - twp->pos.z };

			njPushMatrix(_nj_unit_matrix_);
			njRotateY_(ang_spd);
			njCalcVector(0, &dir, &pos);
			njPopMatrixEx();
			njSubVector(&pos, &dir);

			fwp[i].pos_spd = pos;
			fwp[i].ang_spd.y = -ang_spd;
		}
	}
}

static void __cdecl ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp);
	}
	else
	{
		ExecATask_o(tp);
	}
}

static void __declspec(naked) ExecATask_w()
{
	__asm
	{
		push edi
		call ExecATask_r
		pop edi
		retn
	}
}
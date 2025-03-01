#include "pch.h"
#include "multiplayer.h"

DataPointer(NJS_OBJECT, object_kaiten_kaitendaiz_kaitendaiz, 0x18300C8);

static void __cdecl ObjShelterKaitendai_ExecATask_r(task* tp);
FastUsercallHookPtr<decltype(&ObjShelterKaitendai_ExecATask_r), noret, rEDI> ObjShelterKaitendai_ExecATask_t(0x59E760);

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
			ROTATEY(0, ang_spd);
			njCalcVector(0, &dir, &pos);
			njPopMatrixEx();
			njSubVector(&pos, &dir);

			fwp[i].pos_spd = pos;
			fwp[i].ang_spd.y = -ang_spd;
		}
	}
}

static void __cdecl ObjShelterKaitendai_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp);
	}
	else
	{
		ObjShelterKaitendai_ExecATask_t.Original(tp);
	}
}

void patch_shelter_kaitendai_init()
{
	ObjShelterKaitendai_ExecATask_t.Hook(ObjShelterKaitendai_ExecATask_r);
}

RegisterPatch patch_shelter_kaitendai(patch_shelter_kaitendai_init);
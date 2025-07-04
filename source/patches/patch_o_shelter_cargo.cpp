#include "pch.h"

static void ObjShelterCargoC_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjShelterCargoC_Exec_r)> ObjShelterCargoC_Exec_h(0x59C4D0);

static void ExecC_m(task* tp)
{
	auto twp = tp->twp;
	auto fwp = tp->fwp;

	MirenObjCheckCollisionP(twp, 100.0f);

	float spd = No2CargoSpd;

	if (spd != 0.0f)
	{
		twp->pos.z += spd;
		((NJS_OBJECT*)twp->counter.ptr)->pos[2] = twp->pos.z;
	}

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (CheckPlayerRideOnMobileLandObjectP(i, tp))
		{
			fwp[i].pos_spd.z = spd;
		}
	}

	tp->disp(tp);
}

static void ObjShelterCargoC_Exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecC_m(tp);
	}
	else
	{
		ObjShelterCargoC_Exec_h.Original(tp);
	}
}

void patch_shelter_cargo_init()
{
	ObjShelterCargoC_Exec_h.Hook(ObjShelterCargoC_Exec_r);
}

RegisterPatch patch_shelter_cargo(patch_shelter_cargo_init);
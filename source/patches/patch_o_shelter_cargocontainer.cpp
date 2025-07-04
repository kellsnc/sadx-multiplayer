#include "pch.h"

DataPointer(int, clear_flag, 0x3C72A7C);

static void ObjShelterCargocontainerC_Exec_r(task* tp); // "ExecC"
FastFunctionHookPtr<decltype(&ObjShelterCargocontainerC_Exec_r)> ObjShelterCargocontainerC_Exec_h(0x59BF30);

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

	if (!clear_flag)
	{
		EntryColliList(twp);
	}

	tp->disp(tp);
}

static void ObjShelterCargocontainerC_Exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecC_m(tp);
	}
	else
	{
		ObjShelterCargocontainerC_Exec_h.Original(tp);
	}
}

void patch_shelter_cargocontainer_init()
{
	ObjShelterCargocontainerC_Exec_h.Hook(ObjShelterCargocontainerC_Exec_r);
}

RegisterPatch patch_shelter_cargocontainer(patch_shelter_cargocontainer_init);
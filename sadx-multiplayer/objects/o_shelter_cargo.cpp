#include "pch.h"

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

static void ObjShelterCargoExecC_r(task* tp); // "ExecC"
Trampoline ObjShelterCargoExecC_t(0x59C4D0, 0x59C4D5, ObjShelterCargoExecC_r);
static void ObjShelterCargoExecC_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecC_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjShelterCargoExecC)(tp);
	}
}
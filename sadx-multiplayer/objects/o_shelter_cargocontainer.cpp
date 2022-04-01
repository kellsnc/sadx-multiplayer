#include "pch.h"

DataPointer(int, clear_flag, 0x3C72A7C);

static void ExecC_m(task* tp)
{
	auto twp = tp->twp;
	auto fwp = tp->fwp;

	CheckDyncolRange(twp, &twp->pos, 100.0f);

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

static void ObjShelterCargocontainerExecC_r(task* tp); // "ExecC"
Trampoline ObjShelterCargocontainerExecC_t(0x59BF30, 0x59BF35, ObjShelterCargocontainerExecC_r);
static void ObjShelterCargocontainerExecC_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecC_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjShelterCargocontainerExecC)(tp);
	}
}
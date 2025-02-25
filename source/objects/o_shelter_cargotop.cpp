#include "pch.h"

DataPointer(NJS_ACTION, action_cargo_crgfront, 0x17F1CBC);

static void ExecC_m(task* tp)
{
	auto twp = tp->twp;
	auto fwp = tp->fwp;

	float spd = No2CargoSpd;

	if (spd == 0.0f)
	{
		twp->timer.f += 1.0f;
	}
	else
	{
		if (spd > 0.0f)
		{
			twp->timer.f += 2.0f;
		}

		twp->pos.z += spd;
		((NJS_OBJECT*)twp->counter.ptr)->pos[2] = twp->pos.z;
	}

	const float max_frame = (float)(action_cargo_crgfront.motion->nbFrame - 1);

	if (twp->timer.f >= max_frame)
	{
		twp->timer.f -= max_frame;
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

static void ObjShelterCargotopExecC_r(task* tp); // "ExecC"
FastFunctionHookPtr<decltype(&ObjShelterCargotopExecC_r)> ObjShelterCargotopExecC_t(0x59C230, ObjShelterCargotopExecC_r);
static void ObjShelterCargotopExecC_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecC_m(tp);
	}
	else
	{
		ObjShelterCargotopExecC_t.Original(tp);
	}
}
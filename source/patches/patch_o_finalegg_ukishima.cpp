#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

// Moving platforms in Final Egg

static void __cdecl Ukishima_ExecATask_r(task* tp);
FastUsercallHookPtr<decltype(&Ukishima_ExecATask_r), noret, rEAX> Ukishima_ExecATask_t(0x5B7950);

static void ExecATask_m(task* tp)
{
	auto twp = tp->twp;
	auto fwp = tp->fwp;

	NJS_POINT3 v{};
	v.z = njSin(twp->ang.x) * twp->scl.z;

	njPushMatrix(_nj_unit_matrix_);
	ROTATEY(0, twp->ang.y);
	njCalcVector(0, &v, &v);
	njPopMatrixEx();

	NJS_POINT3 old_pos = twp->pos;

	twp->pos.x = v.x + twp->counter.f;
	twp->pos.z = v.z + twp->value.f;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		fwp[i].pos_spd.x = twp->pos.x - old_pos.x;
		fwp[i].pos_spd.y = twp->pos.y - old_pos.y;
		fwp[i].pos_spd.z = twp->pos.z - old_pos.z;
	}

	twp->flag |= 0x100; // Not using MirenObjCheckCollisionP for some reason

	auto object = reinterpret_cast<NJS_OBJECT*>(twp->cwp);
	object->ang[0] = 0;
	object->ang[1] = twp->ang.y;
	object->ang[2] = 0;
	object->pos[0] = object_cl_uki_cl_uki.pos[0] + twp->pos.x;
	object->pos[1] = object_cl_uki_cl_uki.pos[1] + twp->pos.y;
	object->pos[2] = object_cl_uki_cl_uki.pos[2] + twp->pos.z;

	if (++twp->wtimer >= 0x15ui16)
	{
		twp->wtimer = 0ui16;
	}
}

static void __cdecl Ukishima_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp);
	}
	else
	{
		Ukishima_ExecATask_t.Original(tp);
	}
}

void patch_finalegg_ukishima_init()
{
	Ukishima_ExecATask_t.Hook(Ukishima_ExecATask_r);
}

RegisterPatch patch_finalegg_ukishima(patch_finalegg_ukishima_init);
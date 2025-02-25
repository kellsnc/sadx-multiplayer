#include "pch.h"
#include "multiplayer.h"

// Cylinders in Hot Shelter
// They are made of 6 different tasks in 6 different source files with minor differences (likely copy pasted)
// I merged their logic function into one function

DataPointer(NJS_OBJECT, object_hagu01_cl_a_cl_a, 0x186A4CC);
DataPointer(NJS_OBJECT, object_hagu02_cl_b_cl_b, 0x186B0F4);
DataPointer(NJS_OBJECT, object_hagu03_cl_c_cl_c, 0x186C72C);
DataPointer(NJS_OBJECT, object_hagu04_cl_d_cl_d, 0x186DD64);

static void __cdecl ObjShelterHaguruma1_ExecATask_r(task* tp);
static void __cdecl ObjShelterHaguruma3_ExecATask_r(task* tp);
static void __cdecl ObjShelterHaguruma4_ExecATask_r(task* tp);
static void __cdecl ObjShelterHaguruma6_ExecATask_r(task* tp);

FastFunctionHookPtr<decltype(&ObjShelterHaguruma1_ExecATask_r)> ObjShelterHaguruma1_ExecATask_t(0x5A0310, ObjShelterHaguruma1_ExecATask_r);
FastFunctionHookPtr<decltype(&ObjShelterHaguruma3_ExecATask_r)> ObjShelterHaguruma3_ExecATask_t(0x5A0010, ObjShelterHaguruma3_ExecATask_r);
FastFunctionHookPtr<decltype(&ObjShelterHaguruma4_ExecATask_r)> ObjShelterHaguruma4_ExecATask_t(0x59FB20, ObjShelterHaguruma4_ExecATask_r);
FastFunctionHookPtr<decltype(&ObjShelterHaguruma6_ExecATask_r)> ObjShelterHaguruma6_ExecATask_t(0x59F820, ObjShelterHaguruma6_ExecATask_r);

static void ExecATask_m(task* tp, NJS_OBJECT* object)
{
	auto twp = tp->twp;

	MirenObjCheckCollisionP(twp, 148.0f);

	Angle add_ang = (Angle)twp->scl.x;

	if (GetSwitchOnOff(twp->counter.b[0]))
		add_ang = -add_ang;

	twp->ang.z += add_ang;

	auto dyncol = (NJS_OBJECT*)twp->value.ptr;
	dyncol->ang[0] = 0;
	dyncol->ang[1] = 0;
	dyncol->ang[2] = twp->ang.z;
	dyncol->pos[0] = object->pos[0] + twp->pos.x;
	dyncol->pos[1] = object->pos[1] + twp->pos.y;
	dyncol->pos[2] = object->pos[2] + twp->pos.z;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (CheckPlayerRideOnMobileLandObjectP(i, tp))
		{
			auto ptwp = playertwp[i];

			NJS_POINT3 o = { ptwp->pos.x - twp->pos.x, ptwp->pos.y - twp->pos.y, 0.0f };
			NJS_POINT3 v;

			njPushMatrix(_nj_unit_matrix_);
			njRotateZ_(add_ang);
			njCalcVector(0, &o, &v);
			njPopMatrixEx();
			njSubVector(&v, &o);

			ptwp->pos.x += v.x;
			ptwp->pos.y += v.y;
			ptwp->pos.z += v.z;
		}
	}
}

static void __cdecl ObjShelterHaguruma6_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp, &object_hagu02_cl_b_cl_b);
	}
	else
	{
		ObjShelterHaguruma6_ExecATask_t.Original(tp);
	}
}

static void __cdecl ObjShelterHaguruma4_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp, &object_hagu01_cl_a_cl_a);
	}
	else
	{
		ObjShelterHaguruma4_ExecATask_t.Original(tp);
	}
}

static void __cdecl ObjShelterHaguruma3_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp, &object_hagu03_cl_c_cl_c);
	}
	else
	{
		ObjShelterHaguruma3_ExecATask_t.Original(tp);
	}
}

static void __cdecl ObjShelterHaguruma1_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp, &object_hagu04_cl_d_cl_d);
	}
	else
	{
		ObjShelterHaguruma1_ExecATask_t.Original(tp);
	}
}
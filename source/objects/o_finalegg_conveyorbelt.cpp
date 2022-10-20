#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

static void ConveyorBelt_ExecATask_w();
static void ConveyorBelt_ReserveColli_w();
static void __cdecl ConveyorBelt_SideTaskExec_r(task* tp);

Trampoline ConveyorBelt_ExecATask_t(0x5BD480, 0x5BD487, ConveyorBelt_ExecATask_w);
Trampoline ConveyorBelt_ReserveColli_t(0x5BD3B0, 0x5BD3B5, ConveyorBelt_ReserveColli_w);
Trampoline ConveyorBelt_SideTaskExec_t(0x5BD6D0, 0x5BD6D5, ConveyorBelt_SideTaskExec_r);

#pragma region ExecATask
static void ConveyorBelt_ExecATask_o(task* tp)
{
	auto target = ConveyorBelt_ExecATask_t.Target();
	__asm
	{
		mov ebx, [tp]
		call target
	}
}

static void __cdecl ConveyorBelt_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		MirenObjCheckCollisionP(twp, twp->scl.z * 128.0f + 20.0f);
		twp->wtimer = (unsigned __int8)(LOBYTE(twp->wtimer) - LOBYTE(twp->ang.x));
		dsPlay_timer_v(288, (int)tp, 1, 0, 2, twp->pos.x, twp->pos.y, twp->pos.z);
	}
	else
	{
		ConveyorBelt_ExecATask_o(tp);
	}
}

static void __declspec(naked) ConveyorBelt_ExecATask_w()
{
	__asm
	{
		push ebx
		call ConveyorBelt_ExecATask_r
		pop ebx
		retn
	}
}
#pragma endregion

#pragma region ReserveColli
static void ConveyorBelt_ReserveColli_o(task* tp)
{
	auto target = ConveyorBelt_ReserveColli_t.Target();
	__asm
	{
		mov ebx, [tp]
		call target
	}
}

static void __cdecl ConveyorBelt_ReserveColli_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto fwp = tp->fwp;

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			((NJS_POINT3*)0x3C74668)->z = (float)twp->ang.x * twp->scl.z * 0.079f;
			fwp[i].ang_spd = { 0, 0, 0 };
			njPushMatrix(_nj_unit_matrix_);
			njRotateY_(twp->ang.y);
			njCalcVector(0, (NJS_POINT3*)0x3C74668, &fwp[i].pos_spd);
			njPopMatrixEx();
		}

		if (!CheckEditMode())
		{
			auto object = GetMobileLandObject();
			twp->counter.ptr = object;
			object->evalflags = *(Uint32*)0x19CEBA4;
			object->scl[0] = 1.0f;
			object->scl[1] = 1.0f;
			object->scl[2] = twp->scl.z;
			object->ang[0] = 0;
			object->ang[1] = twp->ang.y;
			object->ang[2] = 0;
			object->pos[0] = twp->pos.x;
			object->pos[1] = twp->pos.y;
			object->pos[2] = twp->pos.z;
			object->model = (NJS_MODEL_SADX*)0x19CEB78;
			object->child = 0;
			object->sibling = 0;
			RegisterCollisionEntry(0, tp, object);
		}
	}
	else
	{
		ConveyorBelt_ReserveColli_o(tp);
	}
}

static void __declspec(naked) ConveyorBelt_ReserveColli_w()
{
	__asm
	{
		push ebx
		call ConveyorBelt_ReserveColli_r
		pop ebx
		retn
	}
}
#pragma endregion

static void __cdecl ConveyorBelt_SideTaskExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		MirenObjCheckCollisionP(tp->twp, tp->ptp->twp->scl.z * 128.0f + 20.0f);
	}
	else
	{
		TARGET_STATIC(ConveyorBelt_SideTaskExec)(tp);
	}
}
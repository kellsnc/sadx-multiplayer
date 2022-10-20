#include "pch.h"
#include "multiplayer.h"

static void ObjShelterBridgeExec_w();
static void ObjShelterBridgeShrink_w();

Trampoline ObjShelterBridgeExec_t(0x5A0E90, 0x5A0E97, ObjShelterBridgeExec_w);
Trampoline ObjShelterBridgeShrink_t(0x5A0FD0, 0x5A0FD7, ObjShelterBridgeShrink_w);

#pragma region ExecATask
static void ObjShelterBridgeExec_o(task* tp)
{
	auto target = ObjShelterBridgeExec_t.Target();
	__asm
	{
		mov ebx, [tp]
		call target
	}
}

static void ObjShelterBridgeExec_m(task* tp)
{
	auto twp = tp->twp;

	auto sub_twp = reinterpret_cast<taskwk*>(twp->value.ptr);
	auto ang_spd = static_cast<Angle>(twp->scl.x);

	twp->ang.y += ang_spd;
	sub_twp->ang = { 0, twp->ang.y, 0 };

	NJS_VECTOR dir = { 0.0f, 0.0f, twp->scl.z };
	njPushMatrix(_nj_unit_matrix_);
	njTranslateEx(&twp->pos);
	njRotateY_(twp->ang.y);
	njCalcPoint(0, &dir, (NJS_VECTOR*)&sub_twp->counter);
	njPopMatrixEx();

	auto fwp = tp->fwp;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (CheckPlayerRideOnMobileLandObjectP(i, tp))
		{
			auto ptwp = playertwp[i];

			NJS_VECTOR pos;
			dir = { ptwp->pos.x - twp->pos.x, 0.0f, ptwp->pos.z - twp->pos.z };

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

static void __cdecl ObjShelterBridgeExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjShelterBridgeExec_m(tp);
	}
	else
	{
		ObjShelterBridgeExec_o(tp);
	}
}

static void __declspec(naked) ObjShelterBridgeExec_w()
{
	__asm
	{
		push ebx
		call ObjShelterBridgeExec_r
		pop ebx
		retn
	}
}
#pragma endregion

#pragma region ShrinkTask
static void ObjShelterBridgeShrink_o(task* tp)
{
	auto target = ObjShelterBridgeShrink_t.Target();
	__asm
	{
		mov edi, [tp]
		call target
	}
}

static void ObjShelterBridgeShrink_m(task* tp)
{
	auto twp = tp->twp;
	auto sub_twp = reinterpret_cast<taskwk*>(twp->value.ptr);

	twp->scl.z += twp->scl.y;

	NJS_VECTOR dir = { 0.0f, 0.0f, twp->scl.z };
	njPushMatrix(_nj_unit_matrix_);
	njTranslateEx(&twp->pos);
	njRotateY_(twp->ang.y);
	njCalcPoint(0, &dir, (NJS_VECTOR*)&sub_twp->counter);
	njPopMatrixEx();

	dir.z = twp->scl.y;

	auto fwp = tp->fwp;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (CheckPlayerRideOnMobileLandObjectP(i, tp))
		{
			auto ptwp = playertwp[i];

			njPushMatrix(_nj_unit_matrix_);
			njRotateY_(twp->ang.y);
			njCalcVector(0, &dir, &fwp[i].pos_spd);
			njPopMatrixEx();

			fwp[i].ang_spd.y = 0;
		}
	}
}

static void __cdecl ObjShelterBridgeShrink_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjShelterBridgeShrink_m(tp);
	}
	else
	{
		ObjShelterBridgeShrink_o(tp);
	}
}

static void __declspec(naked) ObjShelterBridgeShrink_w()
{
	__asm
	{
		push edi
		call ObjShelterBridgeShrink_r
		pop edi
		retn
	}
}
#pragma endregion
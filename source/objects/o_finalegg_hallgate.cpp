#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

enum : __int8 // not official enum
{
	MODE_WAIT,
	MODE_UP,
	MODE_STOP,
	MODE_DOWN // custom
};

DataPointer(NJS_MATRIX, MtxBuff, 0x3B0EFE0);

static void hallgate_set_colli_w();
static void __cdecl hallgate_switch_exec_r(task* tp);
static void __cdecl hallgate_o_switch_exec_r(task* tp);
static void __cdecl hallgate_exec_r(task* tp);

Trampoline hallgate_set_colli_t(0x5B6860, 0x5B6867, hallgate_set_colli_w);
Trampoline hallgate_switch_exec_t(0x5B6B30, 0x5B6B36, hallgate_switch_exec_r);
Trampoline hallgate_o_switch_exec_t(0x5B6910, 0x5B6915, hallgate_o_switch_exec_r);
Trampoline hallgate_exec_t(0x5B69D0, 0x5B69D6, hallgate_exec_r);

static void hallgate_set_colli_o(task* tp)
{
	auto target = hallgate_set_colli_t.Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

static void hallgate_set_colli_m(task* tp)
{
	auto twp = tp->twp;

	auto object = reinterpret_cast<NJS_OBJECT*>(twp->counter.ptr);
	object->pos[0] = twp->pos.x;
	object->pos[1] = twp->pos.y;
	object->pos[2] = twp->pos.z;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto fwp = &tp->fwp[i];
		fwp->pos_spd = twp->pos;
		njSubVector(&fwp->pos_spd, &twp->scl);
	}

	twp->scl = twp->pos;

	MirenObjCheckCollisionP(twp, object->basicdxmodel->r + 20.0f);
}

static void __cdecl hallgate_set_colli_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		hallgate_set_colli_m(tp);
	}
	else
	{
		hallgate_set_colli_o(tp);
	}
}

static void __declspec(naked) hallgate_set_colli_w()
{
	__asm
	{
		push eax
		call hallgate_set_colli_r
		pop eax
		retn
	}
}

static void hallgate_switch_exec_m(task* tp)
{
	auto twp = tp->twp;
	GetMMMatrix(twp->mode, MtxBuff);
	njGetTranslation(MtxBuff, &twp->pos);
	hallgate_set_colli_m(tp);
	EntryColliList(twp);

	if (twp->timer.l)
	{
		auto parent_twp = tp->ptp->twp;
		LoopTaskC(tp);
		if (parent_twp->mode == MODE_WAIT)
		{
			if (GetSwitchOnOff(1u))
			{
				PlayVoice(1590);
				dsPlay_oneshot(301, 0, 0, 0);
				parent_twp->mode = MODE_UP;
				twp->wtimer = 0ui16;
			}
		}
		else if (parent_twp->mode == MODE_STOP)
		{
			// Custom: reset if another player needs a lift
			if (IsPlayerInSphere(1145.0f, -655.0f, -600.0f, 100.0f))
			{
				SetSwitchOnOff(1u, 0);
				twp->wtimer = 0ui16;
				parent_twp->mode = MODE_DOWN;
			}
		}
	}
}

static void __cdecl hallgate_switch_exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		hallgate_switch_exec_m(tp);
	}
	else
	{
		TARGET_STATIC(hallgate_switch_exec)(tp);
	}
}

static void hallgate_o_switch_exec_m(task* tp)
{
	auto twp = tp->twp;
	auto parent_twp = tp->ptp->twp;

	twp->pos = parent_twp->pos;
	twp->pos.y = twp->pos.y + 6.85f;

	auto object = reinterpret_cast<NJS_OBJECT*>(twp->value.ptr);
	object->pos[0] = twp->pos.x;
	object->pos[1] = twp->pos.y;
	object->pos[2] = twp->pos.z;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto fwp = &tp->fwp[i];
		fwp->pos_spd = parent_twp->pos;
		njSubVector(&fwp->pos_spd, &parent_twp->scl);
	}

	ObjectSwitch(tp);
}

static void __cdecl hallgate_o_switch_exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		hallgate_o_switch_exec_m(tp);
	}
	else
	{
		TARGET_STATIC(hallgate_o_switch_exec)(tp);
	}
}

// For reset
static void __cdecl hallgate_exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (twp->mode == MODE_DOWN)
		{
			twp->counter.f -= 1.0f;

			if (twp->counter.f <= 0.0f)
			{
				dsPlay_oneshot(303, 0, 0, 0);
				twp->counter.f = 0.0f;
				twp->mode = MODE_WAIT;
				tp->ocp->ssCondition &= ~0x400;
			}
		}
	}

	TARGET_STATIC(hallgate_exec)(tp);
}
#include "pch.h"
#include "multiplayer.h"

enum : __int8
{
	MODE_INIT,
	MODE_NORMAL,
	MODE_OPEN,
	MODE_WAIT,
	MODE_CLOSE,
	MODE_UP,
	MODE_DOWN,
	MODE_SWITCH, // custom
	MODE_END
};

DataPointer(NJS_OBJECT, object_elevator_hontai_bmerge1, 0x1A37A38);

static void __cdecl ObjectFinalEggElevator_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectFinalEggElevator_r)> ObjectFinalEggElevator_t(0x5B7210);

static void ExecATask_m(taskwk* twp) // inline symbols
{
	MirenObjCheckCollisionP(twp, 60.0f);

	auto dyncol = (NJS_OBJECT*)twp->value.ptr;
	dyncol->ang[0] = 0;
	dyncol->ang[1] = twp->ang.y;
	dyncol->ang[2] = 0;
	dyncol->pos[0] = object_elevator_hontai_bmerge1.pos[0] + twp->pos.x;
	dyncol->pos[1] = object_elevator_hontai_bmerge1.pos[1] + twp->pos.y;
	dyncol->pos[2] = object_elevator_hontai_bmerge1.pos[2] + twp->pos.z;
}

static void Open_m(taskwk* twp) // inlined symbols
{
	twp->scl.x -= 0.02f;
	if (twp->scl.x < 0.06f)
	{
		twp->mode = twp->smode != MODE_END ? MODE_WAIT : MODE_END;
		twp->scl.x = 0.06f;
		PadReadOn();
	}
}

static void Wait_m(task* tp) // inlined symbols
{
	if (IsPlayerOnDyncol(tp))
	{
		auto twp = tp->twp;
		twp->mode = MODE_CLOSE;
		twp->wtimer = 300ui16;
		dsPlay_oneshot(291, 0, 0, 0);
	}
}

static void Close_m(task* tp) // inlined symbols
{
	auto twp = tp->twp;

	twp->scl.x += 0.01f;
	if (twp->scl.x > 1.0f)
	{
		twp->scl.x = 1.0f;

		if (IsPlayerOnDyncol(tp))
		{
			twp->mode = twp->smode;
			dsPlay_oneshot(300, 0, 0, 0);
			//SetFreeCameraMode(0);
		}
		else
		{
			twp->mode = MODE_OPEN;
			dsPlay_oneshot(291, 0, 0, 0);
		}
	}
}

static void UpMove_m(taskwk* twp)
{
	twp->pos.y += 0.6f;
	twp->counter.f += 1.0f;

	if (twp->counter.f == 30.0f)
	{
		dsPlay_oneshot(300, 0, 0, 0);
	}
	else if (twp->counter.f >= 60.0f)
	{
		dsPlay_oneshot(300, 0, 0, 0);
		twp->counter.f = 0.0f;
	}

	if (twp->pos.y >= 154.0f)
	{
		twp->mode = MODE_OPEN;
		twp->smode = MODE_END;
		dsPlay_oneshot(291, 0, 0, 0);
		//SetFreeCameraMode(1);
	}
}

static void DownMove_m(taskwk* twp)
{
	twp->pos.y -= 0.6f;
	twp->counter.f -= 1.0f;

	if (twp->counter.f == 30.0f)
	{
		dsPlay_oneshot(300, 0, 0, 0);
	}
	else if (twp->counter.f < 0.0f)
	{
		dsPlay_oneshot(300, 0, 0, 0);
		twp->counter.f = 59.0f;
	}

	switch (CurrentAct)
	{
	case 0:
		if (twp->pos.y <= -239.0f)
		{
			twp->mode = MODE_OPEN;
			dsPlay_oneshot(291, 0, 0, 0);
			//CancelAutoPilotP(0);
			//SetFreeCameraMode(1);
		}
		break;
	case 1:
		if (twp->pos.y <= -1858.0f)
		{
			ChangeActM(1);
		}
		break;
	case 2:
		if (twp->pos.y <= -2.0f)
		{
			twp->mode = MODE_OPEN;
			dsPlay_oneshot(291, 0, 0, 0);
			//CancelAutoPilotP(0);
			//SetFreeCameraMode(1);
		}
		break;
	}
}

static void ObjectFinalEggElevator_m(task* tp)
{
	auto twp = tp->twp;

	if (CheckRangeOutWithR(tp, 250000.0f))
	{
		return;
	}

	switch (twp->mode)
	{
	case MODE_INIT: // we let the original code be called for this one in case mods edit hardcoded positions
		break;
	case MODE_NORMAL:
		if (IsPlayerInSphere(&twp->pos, 80.0f))
		{
			twp->mode = MODE_OPEN;
		}
		EntryColliList(twp);
		break;
	case MODE_OPEN:
		Open_m(twp);
		break;
	case MODE_WAIT:
		Wait_m(tp);
		break;
	case MODE_CLOSE:
		Close_m(tp);

		if (twp->scl.x > 0.2f)
		{
			EntryColliList(twp);
		}
		break;
	case MODE_UP:
		UpMove_m(twp);
		EntryColliList(twp);
		break;
	case MODE_DOWN:
		DownMove_m(twp);
		EntryColliList(twp);
		break;
	case MODE_SWITCH:
		if (GetSwitchOnOff(2))
		{
			twp->mode = MODE_DOWN;
		}
		break;
	case MODE_END:
		if (CurrentAct == 0) // custom: reset the switch elevator
		{
			SetSwitchOnOff(2, 0);
			twp->mode = MODE_SWITCH;
			twp->smode = MODE_UP;
		}
		break;
	}

	ExecATask_m(twp);
	tp->disp(tp);
}

static void __cdecl ObjectFinalEggElevator_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode != MODE_INIT)
	{
		ObjectFinalEggElevator_m(tp);
	}
	else
	{
		ObjectFinalEggElevator_t.Original(tp);
	}
}

void patch_finalegg_elevator_init()
{
	ObjectFinalEggElevator_t.Hook(ObjectFinalEggElevator_r);
}

RegisterPatch patch_finalegg_elevator(patch_finalegg_elevator_init);
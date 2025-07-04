#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

// Small elevator in Final Egg

enum : __int8
{
	MODE_INIT,
	MODE_NORMAL,
	MODE_WAIT,
	MODE_DOWN,
	MODE_END
};

static void __cdecl ObjectFinalEggElevator2_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectFinalEggElevator2_r)> ObjectFinalEggElevator2_h(0x5B4B30);

static void End_m(task* tp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		tp->fwp[i].pos_spd.y = 0.0f;
	}
}

static void Down_m(task* tp)
{
	auto twp = tp->twp;
	if (twp->pos.y > -804.09998f)
	{
		twp->pos.y -= 1.0f;

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			tp->fwp[i].pos_spd.y = -1.0f;
		}

		dsPlay_timer_v(294, (int)tp, 1, 0, 2, twp->pos.x, twp->pos.y, twp->pos.z);
	}
	else
	{
		End_m(tp);
		twp->mode = MODE_END;
	}
}

static void Wait_m(task* tp)
{
	if (IsPlayerOnDyncol(tp))
	{
		auto twp = tp->twp;
		twp->mode = MODE_DOWN;
		twp->wtimer = 500ui16;
	}
}

static void ExecATask_m(taskwk* twp)
{
	MirenObjCheckCollisionP(twp, 40.0f);

	auto object = reinterpret_cast<NJS_OBJECT*>(twp->value.ptr);
	object->ang[0] = 0;
	object->ang[1] = twp->ang.y;
	object->ang[2] = 0;
	object->pos[0] = object_mdlelv1_cl_elvtr_cl_elvtr.pos[0] + twp->pos.x;
	object->pos[1] = object_mdlelv1_cl_elvtr_cl_elvtr.pos[1] + twp->pos.y;
	object->pos[2] = object_mdlelv1_cl_elvtr_cl_elvtr.pos[2] + twp->pos.z;
}

static void ObjectFinalEggElevator2_m(task* tp)
{
	if (CheckRangeOutWithR(tp, 250000.0f))
	{
		return;
	}

	auto twp = tp->twp;

	switch (twp->mode)
	{
	case MODE_NORMAL:
		ExecATask_m(twp);
		break;
	case MODE_WAIT:
		Wait_m(tp);
		ExecATask_m(twp);
		break;
	case MODE_DOWN:
		Down_m(tp);
		ExecATask_m(twp);
		break;
	case MODE_END:
		End_m(tp);
		break;
	}

	tp->disp(tp);
}

static void __cdecl ObjectFinalEggElevator2_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode != MODE_INIT)
	{
		ObjectFinalEggElevator2_m(tp);
	}
	else
	{
		ObjectFinalEggElevator2_h.Original(tp);
	}
}

void patch_finalegg_elevator2_init()
{
	ObjectFinalEggElevator2_h.Hook(ObjectFinalEggElevator2_r);
}

RegisterPatch patch_finalegg_elevator2(patch_finalegg_elevator2_init);
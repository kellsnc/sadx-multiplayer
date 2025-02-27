#include "pch.h"
#include "multiplayer.h"

static void __cdecl checkTPDoorMode_r(taskwk* twp);
static void __cdecl checkTPDoorMode2_r(taskwk* twp);
static void __cdecl ObjectTPDoubleDoor_r(task* tp);

FastUsercallHookPtr<decltype(&checkTPDoorMode_r), noret, rESI> checkTPDoorMode_t(0x61E460);
FastUsercallHookPtr<decltype(&checkTPDoorMode2_r), noret, rESI> checkTPDoorMode2_t(0x61E510);
FastFunctionHookPtr<decltype(&ObjectTPDoubleDoor_r)> ObjectTPDoubleDoor_t(0x61EAC0);

static void getModeWithFOV(taskwk* twp, taskwk* ptwp)
{
	twp->mode = 2 * (DiffAngle(twp->ang.y - NJM_DEG_ANG(twp->scl.z), NJM_RAD_ANG(atan2(twp->pos.x - ptwp->pos.x, twp->pos.z - ptwp->pos.z))) > 0x4000 ? 1 : 0) + 1;
}

static void checkTPDoorMode_m(taskwk* twp)
{
	auto ptwp = playertwp[GetClosestPlayerNum(&twp->pos)];

	if (ptwp)
	{
		auto dist = GetDistance(&twp->pos, &ptwp->pos);

		if (dist < 100.0f)
		{
			if (dist >= twp->scl.x)
			{
				getModeWithFOV(twp, ptwp);
			}
			else
			{
				twp->mode = 2;
			}
		}
	}
}

static void __cdecl checkTPDoorMode_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		checkTPDoorMode_m(twp);
	}
	else
	{
		checkTPDoorMode_t.Original(twp);
	}
}

static void checkTPDoorMode2_m(taskwk* twp)
{
	auto ptwp = playertwp[GetClosestPlayerNum(&twp->pos)];

	if (ptwp)
	{
		getModeWithFOV(twp, ptwp);
	}
}

static void __cdecl checkTPDoorMode2_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		checkTPDoorMode2_m(twp);
	}
	else
	{
		checkTPDoorMode2_t.Original(twp);
	}
}

static void ObjectTPDoubleDoor_m(task* tp)
{
	if (CheckRangeOutWithR(tp, 1020100.0f))
	{
		return;
	}

	auto twp = tp->twp;

	switch (twp->mode)
	{
	case 0i8:
		twp->mode = 1i8;

		twp->scl.z = 0.0f;
		twp->scl.y = 0.0f;

		if (twp->scl.x <= 0.0f)
		{
			if (twp->scl.x == 0.0f)
			{
				twp->smode = 1i8;
			}
			else
			{
				twp->mode = 3i8;
			}
		}
		else
		{
			twp->smode = 0i8;
		}

		twp->counter.ptr = (void*)0x027AF5EC;
		tp->disp = (TaskFuncPtr)0x61E910;
		CCL_Init(tp, (CCL_INFO*)0x27C5BD0, 4, 4u);
		SET_COLLI_RANGE(twp->cwp, 80.0f);
		twp->cwp->info[1].attr |= 0x10u;
		twp->cwp->info[2].attr |= 0x10u;
		twp->cwp->info[3].attr |= 0x10u;
		break;
	case 1i8:
		if (twp->smode)
		{
			if (twp->smode != 1)
			{
				twp->mode = 3i8;
			}
			else
			{
				if (IsPlayerInSphere(&twp->pos, 50.0f))
				{
					twp->mode = 2i8;
					twp->cwp->info->attr |= 0x10u;
					twp->cwp->info[1].attr &= ~0x10u;
					twp->cwp->info[2].attr &= ~0x10u;
					twp->cwp->info[3].attr &= ~0x10u;
				}
			}
		}
		else if (IsSwitchPressed(0) && IsSwitchPressed(1) && IsSwitchPressed(2))
		{
			twp->mode = 2i8;
			twp->cwp->info->attr |= 0x10u;
			twp->cwp->info[1].attr &= ~0x10u;
			twp->cwp->info[2].attr &= ~0x10u;
			twp->cwp->info[3].attr &= ~0x10u;
		}

		twp->value.l = AdjustAngle(twp->value.l, 0, 256);
		EntryColliList(twp);
		break;
	case 2i8:
		if (!IsPlayerInSphere(&twp->pos, 50.0f))
		{
			twp->mode = 1i8;
			twp->cwp->info->attr &= ~0x10u;
			twp->cwp->info[1].attr |= 0x10u;
			twp->cwp->info[2].attr |= 0x10u;
			twp->cwp->info[3].attr |= 0x10u;
		}

		twp->value.l = AdjustAngle(twp->value.l, 0xC000, 256);

		break;
	case 3i8:
		twp->value.l = AdjustAngle(twp->value.l, 0, 256);
		EntryColliList(twp);
		break;
	}

	tp->disp(tp);
}

static void __cdecl ObjectTPDoubleDoor_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectTPDoubleDoor_m(tp);
	}
	else
	{
		ObjectTPDoubleDoor_t.Original(tp);
	}
}

void patch_twinkle_door_init()
{
	checkTPDoorMode_t.Hook(checkTPDoorMode_r);
	checkTPDoorMode2_t.Hook(checkTPDoorMode2_r);
	ObjectTPDoubleDoor_t.Hook(ObjectTPDoubleDoor_r);
}

RegisterPatch patch_twinkle_door(patch_twinkle_door_init);
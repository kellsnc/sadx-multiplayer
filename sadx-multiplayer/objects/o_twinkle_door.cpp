#include "pch.h"
#include "multiplayer.h"

static void checkTPDoorMode_w();
static void checkTPDoorMode2_w();
static void __cdecl ObjectTPDoubleDoor_r(task* tp);

Trampoline checkTPDoorMode_t(0x61E460, 0x61E465, checkTPDoorMode_w);
Trampoline checkTPDoorMode2_t(0x61E510, 0x61E515, checkTPDoorMode2_w);
Trampoline ObjectTPDoubleDoor_t(0x61EAC0, 0x61EAC6, ObjectTPDoubleDoor_r);

static void getModeWithFOV(taskwk* twp, taskwk* ptwp)
{
	twp->mode = 2 * (DiffAngle(twp->ang.y - NJM_DEG_ANG(twp->scl.z), NJM_RAD_ANG(atan2(twp->pos.x - ptwp->pos.x, twp->pos.z - ptwp->pos.z))) > 0x4000 ? 1 : 0) + 1;
}

static void checkTPDoorMode_o(taskwk* twp)
{
	auto target = checkTPDoorMode_t.Target();
	__asm
	{
		mov esi, [twp]
		call target
	}
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
		checkTPDoorMode_o(twp);
	}
}

static void __declspec(naked) checkTPDoorMode_w()
{
	__asm
	{
		push esi
		call checkTPDoorMode_r
		pop esi
		retn
	}
}

static void checkTPDoorMode2_o(taskwk* twp)
{
	auto target = checkTPDoorMode2_t.Target();
	__asm
	{
		mov esi, [twp]
		call target
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
		checkTPDoorMode2_o(twp);
	}
}

static void __declspec(naked) checkTPDoorMode2_w()
{
	__asm
	{
		push esi
		call checkTPDoorMode2_r
		pop esi
		retn
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
		TARGET_STATIC(ObjectTPDoubleDoor)(tp);
	}
}
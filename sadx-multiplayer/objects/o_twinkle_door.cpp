#include "pch.h"
#include "multiplayer.h"

static void checkTPDoorMode_w();
static void checkTPDoorMode2_w();

Trampoline checkTPDoorMode_t(0x61E460, 0x61E465, checkTPDoorMode_w);
Trampoline checkTPDoorMode2_t(0x61E510, 0x61E515, checkTPDoorMode2_w);

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
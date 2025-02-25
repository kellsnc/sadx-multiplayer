#include "pch.h"
#include "multiplayer.h"
#include "emeraldhunt.h"

static void ObjectEmeraldPRegular_m(task* tp)
{
	if (CheckRangeOut(tp))
	{
		return;
	}

	auto twp = tp->twp;

	if (GetPlayerNumber() != 3 || !Knuckles_KakeraGame_Set_CheckEme(twp->ang.z, &twp->pos))
	{
		DeadOut(tp);
		return;
	}

	if ((twp->ang.z & 0xF0) == 32)
	{
		auto pnum = IsPlayerInSphere(&twp->pos, 30.0f) - 1;

		if (pnum >= 0 && Knuckles_Status(pnum) == 2)
		{
			Knuckles_KakeraGame_Set_PutEme_m(pnum, twp->ang.z, &twp->pos);
			DeadOut(tp);
			return;
		}

		tp->disp = nullptr;
		return;
	}
	else
	{
		auto pnum = IsPlayerInSphere(twp->pos.x, twp->pos.y - 5.0f, twp->pos.z, 10.0f) - 1;

		if (pnum >= 0)
		{
			Knuckles_KakeraGame_Set_PutEme_m(pnum, twp->ang.z, &twp->pos);
			DeadOut(tp);
			return;
		}

		tp->disp = CallObjectEmeraldDisplay;
		tp->disp(tp);
	}
}

static void __cdecl ObjectEmeraldPRegular_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectEmeraldPRegular_r)> ObjectEmeraldPRegular_t(0x4A2FD0, ObjectEmeraldPRegular_r);
static void __cdecl ObjectEmeraldPRegular_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectEmeraldPRegular_m(tp);
	}
	else
	{
		ObjectEmeraldPRegular_t.Original(tp);
	}
}
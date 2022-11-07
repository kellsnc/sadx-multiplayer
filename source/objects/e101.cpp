#include "pch.h"
#include "bosses.h"
#include "levels.h"
#include "patches.h"

static void __cdecl Rd_E101_r(task* tp);
TaskHook E101_Main_t(E101_Main, Rd_E101_r);

static void __cdecl Rd_E101_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		auto data1 = tp->twp;
		auto pNum = GetTheNearestPlayerNumber(&data1->pos);
		auto player = playertwp[pNum];

		if (player)
			E100CheckAndSetDamage(data1, player);

		E101_Main_t.Original(tp);
	}
}

#include "pch.h"
#include "gravity.h"

TaskHook SonicJiggle_t((intptr_t)Sonic_Jiggle_Main);
TaskHook SonicTheHedgehog_t(0x49A9B0);
TaskHook SonicDirectAhead_t((intptr_t)0x493C70);

static void __cdecl SonicDirectAhead_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	SonicDirectAhead_t.Original(tp);
}

static void __cdecl SonicJiggle_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	SonicJiggle_t.Original(tp);
}

static void __cdecl SonicTheHedgehog_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(tp->twp);
		gravity::SaveGlobalGravity();
		gravity::SwapGlobalToUserGravity(pnum);
		SonicTheHedgehog_t.Original(tp);
		gravity::RestoreGlobalGravity();
	}
	else
	{
		SonicTheHedgehog_t.Original(tp);
	}
}

void initSonicPatch()
{
	SonicTheHedgehog_t.Hook(SonicTheHedgehog_r);
	SonicJiggle_t.Hook(SonicJiggle_r);
	SonicDirectAhead_t.Hook(SonicDirectAhead_r);
}
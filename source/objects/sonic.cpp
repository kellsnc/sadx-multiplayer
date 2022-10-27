#include "pch.h"
#include "gravity.h"

TaskHook SonicJiggle_t((intptr_t)Sonic_Jiggle_Main);
TaskHook SonicTheHedgehog_t(0x49A9B0);
TaskHook SonicDirectAhead_t((intptr_t)0x493C70);
UsercallFunc(Bool, Sonic_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x495FA0, rEAX, rEAX, rEDI, stack4);

static Bool Sonic_CheckInput_r(playerwk* co2, taskwk* twp, motionwk2* data2)
{
	auto even = twp->ewp;

	if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
	{
		return Sonic_CheckInput_t.Original(co2, twp, data2);
	}

	if (twp->smode == 5)
	{
		if (CurrentLevel != LevelIDs_SkyDeck)
		{
			return 0;
		}
	}

	return Sonic_CheckInput_t.Original(co2, twp, data2);
}

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
	Sonic_CheckInput_t.Hook(Sonic_CheckInput_r);
}
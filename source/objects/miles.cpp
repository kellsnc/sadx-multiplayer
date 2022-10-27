#include "pch.h"
#include "e_cart.h"

TaskHook MilesJiggle_t((intptr_t)Tails_Jiggle_Main);
TaskHook MilesDirectAhead_t(0x45DAD0);
TaskHook MilesDirectAhead2_t(0x45DE20);
UsercallFuncVoid(Miles_RunActions_t, (playerwk* a1, motionwk2* a2, taskwk* a3), (a1, a2, a3), 0x45E5D0, rEAX, rECX, stack4);
UsercallFunc(Bool, Miles_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x45C100, rEAX, rEDI, rESI, stack4);

static Bool Miles_CheckInput_r(playerwk* co2, taskwk* twp, motionwk2* data2)
{
	auto even = twp->ewp;

	if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
	{
		return Miles_CheckInput_t.Original(co2, twp, data2);
	}

	if (twp->smode == 5)
	{
		if (CurrentLevel == LevelIDs_Casinopolis)
		{
			return 0;
		}
	}

	return Miles_CheckInput_t.Original(co2, twp, data2);
}

static void __cdecl MilesDirectAhead2_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	MilesDirectAhead2_t.Original(tp);
}

static void __cdecl MilesDirectAhead_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	MilesDirectAhead_t.Original(tp);
}

static void __cdecl MilesJiggle_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	MilesJiggle_t.Original(tp);
}

void Miles_RunAction_r(playerwk* co2, motionwk2* data2, taskwk* data1)
{
	switch (data1->mode)
	{
	case 43:
		KillPlayerInKart(data1, co2, 60, 28);
		break;
	}

	Miles_RunActions_t.Original(co2, data2, data1);
}

void initMilesPatches()
{
	MilesJiggle_t.Hook(MilesJiggle_r);
	MilesDirectAhead_t.Hook(MilesDirectAhead_r);
	MilesDirectAhead2_t.Hook(MilesDirectAhead2_r);
	Miles_RunActions_t.Hook(Miles_RunAction_r);
	Miles_CheckInput_t.Hook(Miles_CheckInput_r);
}
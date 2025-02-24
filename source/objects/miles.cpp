#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "gravity.h"
#include "result.h"
#include "e_cart.h"

FastFunctionHook<void, task*> MilesJiggle_t((intptr_t)Tails_Jiggle_Main);
FastFunctionHook<void, task*> MilesDirectAhead_t(0x45DAD0);
FastFunctionHook<void, task*> MilesDirectAhead2_t(0x45DE20);
FastFunctionHook<void, task*> MilesTalesPrower_t(0x461700);
FastUsercallHookPtr<void(*)(playerwk* pwp, motionwk2* mwp, taskwk* twp), noret, rEAX, rECX, stack4> Miles_RunActions_t(0x45E5D0);
FastUsercallHookPtr<Bool(*)(taskwk* twp, playerwk* pwp, motionwk2* mwp), rEAX, rESI, rEDI, stack4> Miles_CheckInput_t(0x45C100);

static Bool Miles_CheckInput_r(taskwk* twp, playerwk* pwp, motionwk2* mwp)
{
	if (multiplayer::IsActive())
	{
		auto even = twp->ewp;
		auto pnum = TASKWK_PLAYERID(twp);

		if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
		{
			return Miles_CheckInput_t.Original(twp, pwp, mwp);
		}

		switch (twp->smode)
		{
		case PL_OP_PARABOLIC:
			if (CurrentLevel == LevelIDs_Casinopolis)
				return FALSE;
			break;
		case PL_OP_PLACEWITHKIME:
			twp->mode = 62;
			pwp->mj.reqaction = CheckDefeat(pnum) ? 56 : 54;
			twp->ang.z = 0;
			twp->ang.x = 0;
			PClearSpeed(mwp, pwp);
			twp->flag &= ~0x2500;
			twp->timer.b[3] |= 8;
			CancelLookingAtP(pnum);
			return TRUE;
		}
	}
	
	return Miles_CheckInput_t.Original(twp, pwp, mwp);
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

void MilesTalesPrower_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(tp->twp);
		gravity::SaveGlobalGravity();
		gravity::SwapGlobalToUserGravity(pnum);
		MilesTalesPrower_t.Original(tp);
		gravity::RestoreGlobalGravity();
	}
	else
	{
		MilesTalesPrower_t.Original(tp);
	}
}

void initMilesPatches()
{
	MilesJiggle_t.Hook(MilesJiggle_r);
	MilesDirectAhead_t.Hook(MilesDirectAhead_r);
	MilesDirectAhead2_t.Hook(MilesDirectAhead2_r);
	Miles_RunActions_t.Hook(Miles_RunAction_r);
	Miles_CheckInput_t.Hook(Miles_CheckInput_r);
	MilesTalesPrower_t.Hook(MilesTalesPrower_r);
}
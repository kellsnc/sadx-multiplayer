#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "gravity.h"
#include "result.h"
#include "patch_player.h"
#include "patch_e_cart.h"

FastFunctionHook<void, task*> MilesJiggle_h((intptr_t)Tails_Jiggle_Main);
FastFunctionHook<void, task*> MilesDirectAhead_h(0x45DAD0);
FastFunctionHook<void, task*> MilesDirectAhead2_h(0x45DE20);
FastFunctionHook<void, task*> MilesTalesPrower_h(0x461700);
FastUsercallHookPtr<void(*)(playerwk* pwp, motionwk2* mwp, taskwk* twp), noret, rEAX, rECX, stack4> Miles_RunActions_h(0x45E5D0);
FastUsercallHookPtr<Bool(*)(taskwk* twp, playerwk* pwp, motionwk2* mwp), rEAX, rESI, rEDI, stack4> Miles_CheckInput_h(0x45C100);

static Bool Miles_CheckInput_r(taskwk* twp, playerwk* pwp, motionwk2* mwp)
{
	if (multiplayer::IsActive())
	{
		auto even = twp->ewp;
		auto pnum = TASKWK_PLAYERID(twp);

		if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
		{
			return Miles_CheckInput_h.Original(twp, pwp, mwp);
		}

		switch ((uint8_t)twp->smode)
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
	
	return Miles_CheckInput_h.Original(twp, pwp, mwp);
}

// Not yet supported in multiplayer
static void __cdecl MilesDirectAhead2_r(task* tp)
{
	if (multiplayer::IsActive() && !canselEvent)
	{
		FreeTask(tp);
		return;
	}

	MilesDirectAhead2_h.Original(tp);
}

// Not yet supported in multiplayer
static void __cdecl MilesDirectAhead_r(task* tp)
{
	if (multiplayer::IsActive() && !canselEvent)
	{
		FreeTask(tp);
		return;
	}

	MilesDirectAhead_h.Original(tp);
}

// Not yet supported in multiplayer
static void __cdecl MilesJiggle_r(task* tp)
{
	if (multiplayer::IsActive() && !canselEvent)
	{
		FreeTask(tp);
		return;
	}

	MilesJiggle_h.Original(tp);
}

void Miles_RunAction_r(playerwk* pwp, motionwk2* mwp, taskwk* twp)
{
	switch ((uint8_t)twp->mode)
	{
	case MD_MILES_S3A2_CART: // Allow death incarts
		if ((twp->flag & Status_DoNextAction) && twp->smode == PL_OP_KILLED)
		{
			twp->mode = MD_MILES_KILL;
			pwp->mj.reqaction = 28;
		}
		break;
	}

	Miles_RunActions_h.Original(pwp, mwp, twp);
}

void MilesTalesPrower_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(tp->twp);
		gravity::SaveGlobalGravity();
		gravity::SwapGlobalToUserGravity(pnum);
		MilesTalesPrower_h.Original(tp);
		gravity::RestoreGlobalGravity();
	}
	else
	{
		MilesTalesPrower_h.Original(tp);
	}
}

void patch_miles_init()
{
	MilesJiggle_h.Hook(MilesJiggle_r);
	MilesDirectAhead_h.Hook(MilesDirectAhead_r);
	MilesDirectAhead2_h.Hook(MilesDirectAhead2_r);
	Miles_RunActions_h.Hook(Miles_RunAction_r);
	Miles_CheckInput_h.Hook(Miles_CheckInput_r);
	MilesTalesPrower_h.Hook(MilesTalesPrower_r);
}

RegisterPatch patch_miles(patch_miles_init);
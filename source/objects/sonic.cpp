#include "pch.h"
#include "gravity.h"
#include "result.h"

TaskHook SonicJiggle_t((intptr_t)Sonic_Jiggle_Main);
TaskHook SonicTheHedgehog_t(0x49A9B0);
TaskHook SonicDirectAhead_t((intptr_t)0x493C70);
UsercallFunc(Bool, Sonic_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x495FA0, rEAX, rEAX, rEDI, stack4);

static Bool Sonic_CheckInput_r(playerwk* pwp, taskwk* twp, motionwk2* mwp)
{
	if (multiplayer::IsActive())
	{
		auto even = twp->ewp;
		auto pnum = TASKWK_PLAYERID(twp);

		if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
		{
			return Sonic_CheckInput_t.Original(pwp, twp, mwp);
		}

		switch (twp->smode)
		{
		case PL_OP_PARABOLIC:
			if (CurrentLevel == LevelIDs_Casinopolis)
				return FALSE;
			break;
		case PL_OP_PLACEWITHKIME:
			if (CheckDefeat(pnum))
			{
				twp->mode = 20;
				pwp->mj.reqaction = 87;
				twp->ang.z = 0;
				twp->ang.x = 0;
				PClearSpeed(mwp, pwp);
				twp->flag &= ~0x2500;
				twp->timer.b[3] |= 8;
				CancelLookingAtP(pnum);
				return TRUE;
			}
			break;
		}
	}
	
	return Sonic_CheckInput_t.Original(pwp, twp, mwp);
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
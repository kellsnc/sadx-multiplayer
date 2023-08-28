#include "pch.h"
#include "sadx_utils.h"
#include "gravity.h"
#include "result.h"
#include "collision.h"
#include "sonic.h"

FunctionHook<taskwk*, taskwk*, playerwk*> SonicCheckLSSTargetEnemy_t(0x492710);
TaskHook SonicDirectAhead_t((intptr_t)0x493C70);
TaskHook SonicJiggle_t((intptr_t)Sonic_Jiggle_Main);
UsercallFunc(Bool, Sonic_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x495FA0, rEAX, rEAX, rEDI, stack4);
TaskHook SonicTheHedgehog_t(0x49A9B0);

static taskwk* SonicCheckLSSTargetEnemy_m(taskwk* twp, playerwk* pwp)
{
	auto pnum = TASKWK_PLAYERID(twp);
	auto ael = GetTargetEnemyList(pnum);

	pwp->free.sw[3] = 0;

	if (!ael->twp)
	{
		return NULL;
	}

	Sint32 rival = GetRivalPlayerNumber(pnum);

	Float tgt_dist;
	taskwk* tgt_twp;

	if (rival == -1)
	{
		tgt_dist = 160000.0f;
		tgt_twp = NULL;
	}
	else
	{
		tgt_twp = playertwp[rival];
		NJS_VECTOR v;
		v.x = twp->pos.x - tgt_twp->pos.x;
		v.y = twp->pos.y - tgt_twp->pos.y;
		v.z = twp->pos.z - tgt_twp->pos.z;
		tgt_dist = njScalor2(&v);
	}

	while (ael->twp)
	{
		if ((ael->twp->cwp->id == CID_ENEMY || ael->twp->cwp->id == CID_ENEMY2) && tgt_dist > ael->dist)
		{
			tgt_twp = ael->twp;
			tgt_dist = ael->dist;
		}
		++ael;
	}

	return tgt_twp;
}

static taskwk* __cdecl SonicCheckLSSTargetEnemy_r(taskwk* twp, playerwk* pwp)
{
	if (multiplayer::IsActive())
	{
		return SonicCheckLSSTargetEnemy_m(twp, pwp);
	}
	else
	{
		return SonicCheckLSSTargetEnemy_t.Original(twp, pwp);
	}
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
	SonicCheckLSSTargetEnemy_t.Hook(SonicCheckLSSTargetEnemy_r);
	SonicDirectAhead_t.Hook(SonicDirectAhead_r);
	SonicJiggle_t.Hook(SonicJiggle_r);
	Sonic_CheckInput_t.Hook(Sonic_CheckInput_r);
	SonicTheHedgehog_t.Hook(SonicTheHedgehog_r);
}
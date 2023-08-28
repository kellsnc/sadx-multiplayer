#include "pch.h"
#include "sadx_utils.h"
#include "gravity.h"
#include "result.h"
#include "collision.h"
#include "sonic.h"

#define SPINTIMER(pwp) (pwp->free.sw[1])
#define HOMING_TIMER1(pwp) (pwp->free.sw[2])
#define HOMING_TIMER2(pwp) (pwp->free.sw[3])

FunctionHook<void, taskwk*, motionwk2*, playerwk*> SonicHomingOnRings_t(0x492AB0);
FunctionHook<taskwk*, taskwk*, playerwk*> SonicCheckLSSTargetEnemy_t(0x492710);
TaskHook SonicDirectAhead_t((intptr_t)0x493C70);
TaskHook SonicJiggle_t((intptr_t)Sonic_Jiggle_Main);
UsercallFunc(Bool, Sonic_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x495FA0, rEAX, rEAX, rEDI, stack4);
TaskHook SonicTheHedgehog_t(0x49A9B0);

static void SonicHomingOnRings_m(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	auto pnum = TASKWK_PLAYERID(twp);
	auto arl = GetTargetRingList(pnum);

	if (!arl->twp)
	{
		PGetInertia(twp, mwp, pwp);
		PGetSpeed(twp, mwp, pwp);
		return;
	}

	NJS_VECTOR forward = { 1.0f, 0.0f, 0.0f };
	PConvertVector_P2G(twp, &forward);
	njUnitVector(&forward);

	taskwk* tgt_twp = NULL;
	Float tgt_dist;

	while (arl->twp)
	{
		NJS_VECTOR vec = arl->twp->pos;
		njAddVector(&vec, &arl->twp->cwp->info->center);
		njSubVector(&vec, &twp->pos);
		njScalor(&vec);

		if (VectorAngle(&forward, &vec, 0) <= 0x3000 && (!tgt_twp || arl->dist < tgt_dist))
		{
			tgt_twp = arl->twp;
			tgt_dist = arl->dist;
		}

		++arl;
	}

	if (tgt_twp)
	{
		if (HOMING_TIMER1(pwp) < 5)
		{
			HOMING_TIMER1(pwp) = 5;
		}

		NJS_VECTOR vec = tgt_twp->pos;
		njAddVector(&vec, &tgt_twp->cwp->info->center);
		njSubVector(&vec, &twp->pos);

		twp->ang.y = AdjustAngle(twp->ang.y, njArcTan2(vec.z, vec.x), 0x1800);
		mwp->ang_aim.y = twp->ang.y;

		Float dist = njSqrt(tgt_dist);
		if (dist > 8.0f)
		{
			dist = 8.0f;
		}
		else if (dist < 2.0f)
		{
			dist = 2.0f;
		}

		if (njScalor(&vec) <= 1.0f)
		{
			vec = mwp->spd;
		}

		njUnitVector(&vec);
		mwp->spd.x = vec.x * dist;
		mwp->spd.y = vec.y * dist;
		mwp->spd.z = vec.z * dist;
		PConvertVector_G2P(twp, &vec);
		pwp->spd = vec;
	}
}

static void __cdecl SonicHomingOnRings_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (multiplayer::IsActive())
	{
		SonicHomingOnRings_m(twp, mwp, pwp);
	}
	else
	{
		SonicHomingOnRings_t.Original(twp, mwp, pwp);
	}
}

static taskwk* SonicCheckLSSTargetEnemy_m(taskwk* twp, playerwk* pwp)
{
	auto pnum = TASKWK_PLAYERID(twp);
	auto ael = GetTargetEnemyList(pnum);

	HOMING_TIMER2(pwp) = 0;

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
	SonicHomingOnRings_t.Hook(SonicHomingOnRings_r);
	SonicCheckLSSTargetEnemy_t.Hook(SonicCheckLSSTargetEnemy_r);
	SonicDirectAhead_t.Hook(SonicDirectAhead_r);
	SonicJiggle_t.Hook(SonicJiggle_r);
	Sonic_CheckInput_t.Hook(Sonic_CheckInput_r);
	SonicTheHedgehog_t.Hook(SonicTheHedgehog_r);
}
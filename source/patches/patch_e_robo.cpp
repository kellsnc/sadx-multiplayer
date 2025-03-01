#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "utils.h"
#include "multiplayer.h"

// Detachable-head robots

static const void* const loc_4A4240 = (void*)0x4A4240;
static inline void sub_4A4240(taskwk* twp, enemywk* ewp)
{
	__asm
	{
		mov eax, [ewp]
		mov ecx, [twp]
		call loc_4A4240
	}
}

static void __cdecl RoboHeadDisplayer_r(task* tp);
static void __cdecl RoboHeadChase_r(task* tp, enemywk* ewp);
static void __cdecl RoboDisplayer_r(task* tp);
static Bool __cdecl RoboHearSound_r(taskwk* twp);
static Bool __cdecl RoboSearchPlayer_r(taskwk* twp, enemywk* ewp);

FastFunctionHookPtr<decltype(&RoboHeadDisplayer_r)> RoboHeadDisplayer_h(0x4A4220, RoboHeadDisplayer_r);
FastUsercallHookPtr<decltype(&RoboHeadChase_r), noret, rEBX, rEAX> RoboHeadChase_h(0x4A4520, RoboHeadChase_r);
FastFunctionHookPtr<decltype(&RoboDisplayer_r)> RoboDisplayer_h(0x4A4DA0, RoboDisplayer_r);
FastUsercallHookPtr<decltype(&RoboHearSound_r), rEAX, rECX> RoboHearSound_h(0x4A5190, RoboHearSound_r);
FastUsercallHookPtr<decltype(&RoboSearchPlayer_r), rEAX, rESI, rEBX> RoboSearchPlayer_h(0x4A51F0, RoboSearchPlayer_r);

FastUsercallHookPtr<void(*)(taskwk* twp, enemywk* ewp), noret, rECX, rEAX> RoboSwing_h(0x4A5840);

static auto RoboCombo = GenerateUsercallWrapper<void(*)(enemywk* wk, taskwk* ewp)>(noret, 0x4A4AA0, rEAX, rECX);

void RoboBumpSide_r(taskwk* twp, char pnum)
{
	taskwk* ptwp = playertwp[pnum];
	Angle3 r; 
	NJS_POINT3 v;
	r.x = 0;
	r.y = njArcTan2(ptwp->pos.x - twp->pos.x, ptwp->pos.z - twp->pos.z);
	r.z = 0;
	v.x = 3.5f;
	v.y = 1.8f;
	v.z = 0.0f;
	SetVelocityAndRotationAndNoconTimeP(pnum, &v, &r, 60);
	ptwp->pos.y += 1.0f;
}

void RoboSwing_r(taskwk* twp, enemywk* ewp)
{
	if (!multiplayer::IsActive())
	{
		return RoboSwing_h.Original(twp, ewp);
	}

	auto pnum = GetTheNearestPlayerNumber(&twp->pos);
	auto cwp = twp->cwp;

	EnemyDist2FromPlayer(twp, pnum);

	ewp->aim = playertwp[pnum]->pos;
	EnemyCalcAimAngle(twp, ewp);
	twp->ang.y = BAMS_SubWrap(twp->ang.y, ewp->aim_angle, 0x900);

	if ((cwp->flag & 1) && cwp->my_num == 1 && cwp->hit_cwp->id == 0)
	{
		RoboBumpSide_r(twp, pnum);
	}

	switch (twp->smode)
	{
	case 0:
		ewp->angz_spd += 0x900;
		break;
	case 1:
		twp->cwp->info[1].attr &= ~0x10u;
		ewp->nframe += 0.45f;
		if (ewp->nframe > 29.0f)
		{
			ewp->nframe = 29.0f;
			twp->smode = 2;
			ewp->lframe = ewp->pframe;
		}
		twp->flag &= ~0x2000;
		break;
	case 2:
		twp->cwp->info[1].attr &= ~0x10u;
		if (twp->wtimer > 0x14u)
		{
			twp->smode = 3;
		}
		twp->flag &= ~0x2000;
		break;
	case 3:
		twp->cwp->info[1].attr |= 0x10u;
		ewp->angz_spd -= 0x900;
		twp->flag |= 0x2000;
		break;
	case 4:
		twp->flag &= ~0x2800;
		twp->mode = ewp->old_mode;
		twp->wtimer = 0;
		break;
	default:
		break;
	}

	ewp->acc.y = ewp->force.z / ((twp->pos.y - ewp->shadow.hit[2].onpos) * 1.4f + 3.0f) + ewp->acc.y;
	
	RoboCombo(ewp, twp);
	EnemyCheckGroundCollision(twp, ewp);
	RoboDraw(twp, ewp);
}

#pragma region RoboHeadDisplayer
static void __cdecl RoboHeadDisplayer_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		// Vanilla issue:
		auto twp = tp->twp;
		if (twp->mode > 0 && twp->mode < 6)
			RoboHeadDraw(twp, (enemywk*)tp->mwp);
	}
	else
	{
		RoboHeadDisplayer_h.Original(tp);
	}
}
#pragma endregion

#pragma region RoboHeadChase
static void RoboHeadChase_m(task* tp, enemywk* ewp)
{
	auto twp = tp->twp;
	ewp->aim = playertwp[GetClosestPlayerNum(&twp->pos)]->pos;
	EnemyTurnToAim(twp, ewp);
	ewp->acc.x = njCos(twp->ang.y) * ewp->force.x;
	ewp->acc.z = njSin(-twp->ang.y) * ewp->force.x;
	RoboHeadUp(twp, ewp);
	sub_4A4240(twp, ewp);
	EnemyCheckGroundCollision(twp, ewp);
	RoboHeadDraw(twp, ewp);
	if (RoboHeadCaptureBeam(tp))
	{
		ewp->old_mode = twp->mode;
		twp->mode = 3;
		twp->wtimer = 0;
		tp->ptp->twp->flag |= 0x200u;
		twp->flag |= 0x8000u;
		SetInputP(TASKWK_PLAYERID(twp->cwp->hit_cwp->mytask->twp), PL_OP_CAPTURED);
	}
	if (twp->wtimer > 0x12Cu)
	{
		ewp->old_mode = twp->mode;
		twp->mode = 3;
		twp->wtimer = 0;
		tp->ptp->twp->flag |= 0x200u;
	}
	if (twp->wtimer <= 0x3Cu)
	{
		twp->cwp->flag &= ~0x1u;
	}
	else
	{
		EntryColliList(twp);
	}
}

static void __cdecl RoboHeadChase_r(task* tp, enemywk* ewp)
{
	if (multiplayer::IsActive())
	{
		RoboHeadChase_m(tp, ewp);
	}
	else
	{
		RoboHeadChase_h.Original(tp, ewp);
	}
}
#pragma endregion

#pragma region RoboDisplayer
static void __cdecl RoboDisplayer_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		RoboDraw(tp->twp, (enemywk*)tp->mwp);
	}
	else
	{
		RoboDisplayer_h.Original(tp);
	}
}
#pragma endregion

#pragma region RoboHearSound
static int RoboHearSound_m(taskwk* twp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];
		if (!ptwp)
		{
			break;
		}

		auto ppwp = playerpwp[i];
		if (!ppwp)
		{
			break;
		}

		if (EnemyDist2FromPlayer(twp, i) < 22500.0f && PCheckJump(ptwp) || ppwp->p.jog_speed * 1.8f <= ppwp->spd.x)
		{
			return i + 1;
		}
	}

	return 0;
}

static Bool __cdecl RoboHearSound_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		return RoboHearSound_m(twp) != 0 ? TRUE : FALSE;
	}
	else
	{
		return RoboHearSound_h.Original(twp);
	}
}
#pragma endregion

#pragma region RoboSearchPlayer
static Bool RoboSearchPlayer_m(taskwk* twp, enemywk* ewp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];
		if (!ptwp)
		{
			break;
		}

		if (fabs(ptwp->pos.y - twp->pos.y) < 40.0f && EnemyDist2FromPlayer(twp, i) < 40000.0f
			&& DiffAngle(-16384 - NJM_RAD_ANG(-atan2(ptwp->pos.x - twp->pos.x, ptwp->pos.z - twp->pos.z)), ewp->sub_angle[4]) < 2730)
		{
			return TRUE;
		}
	}

	int pnum_heard = RoboHearSound_m(twp) - 1;
	if (pnum_heard >= 0)
	{
		twp->wtimer = 0;
		twp->btimer = 90;
		EnemyCalcPlayerAngle(twp, ewp, pnum_heard);
	}
	else
	{
		if (twp->wtimer > twp->btimer)
		{
			twp->wtimer = 0;
			twp->btimer = 60 - (int)(njRandom() * -70.0f);
			if (twp->btimer < 0x78u)
			{
				ewp->aim_angle = NJM_DEG_ANG(njRandom() * 360.0);
			}
		}
		if (twp->btimer > 0x78u)
		{
			ewp->aim_angle += 512;
		}
	}

	return FALSE;
}

static Bool __cdecl RoboSearchPlayer_r(taskwk* twp, enemywk* ewp)
{
	if (multiplayer::IsActive())
	{
		return RoboSearchPlayer_m(twp, ewp);
	}
	else
	{
		return RoboSearchPlayer_h.Original(twp, ewp);
	}
}
#pragma endregion

void patch_robo_init()
{
	RoboSwing_h.Hook(RoboSwing_r);
}

RegisterPatch patch_robo(patch_robo_init);
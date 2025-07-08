#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include <camera.h>

// Egg Hornet

FastFunctionHook<void, task*> Egm1_h(0x572010);
FastFunctionHook<int> EH_PosPlayerCheck_h(0x573310);
FastFunctionHook<int, taskwk*> EH_GetVsPlayerAng_h(0x573160);

FastUsercallHookPtr<void(*)(task* tp, taskwk* twp, bossextwk* egm), noret, rECX, rEAX, rESI> SetEgm1MoveRoute_h(0x5733E0);
FastUsercallHookPtr<void(*)(task* tp), noret, rEAX> SetEgm1Ud_h(0x5736D0);
FastUsercallHookPtr<void(*)(taskwk* twp), noret, rEAX> setEgm1Missile_h(0x573730);
FastUsercallHookPtr<void(*)(bossextwk* egm, taskwk* twp), noret, rECX, rEAX> SetEgm1AtkRoute_h(0x5735B0);

static const int timeLimit = 400;

FastUsercallHookPtr<void(*)(task* tp), noret, rEAX> BossCheckDamage_h(0x571CD0);
static void BossCheckDamage(task* tp)
{
	__int16 flag; 
	bossextwk* egm1wk; 
	taskwk* twp = tp->twp;
	egm1wk = (bossextwk*)tp->awp;
	flag = twp->flag;
	if ((flag & 4) != 0 && egm1wk->HitPoint > 0)
	{
		auto player = CCL_IsHitPlayer(twp);
		if (player)
		{
			NJS_POINT3 a2;
			a2.x = -2.0f;
			a2.y = 2.0f;
			a2.z = 0.0f;
			PConvertVector_P2G(player, &a2);
			SetVelocityP(player->counter.b[0], a2.x, a2.y, a2.z);
		}
	}

	BossCheckDamage_h.Original(tp);
}

FastUsercallHookPtr<void(*)(taskwk* tp), noret, rEAX> EggHornet_InitFight_h(0x571C90);
void EggHornet_InitFight(taskwk* twp)
{
	EggHornet_InitFight_h.Original(twp);

	if (multiplayer::IsActive())
	{
		if (ccsi_flag)
		{
			for (int i = 1; i < PLAYER_MAX; ++i) //todo rework to avoid this hacky stuff
			{
				//CameraSetEventCamera_m(i, CAMMD_CHAOS_STD, CAMADJ_NONE);
			}
		}
	}
}

void SetEgm1AtkRoute_r(bossextwk* egm, taskwk* twp)
{
	if (!multiplayer::IsActive())
	{
		return SetEgm1AtkRoute_h.Original(egm, twp);
	}

	auto player = playertwp[GetBossTargetPlayerRandom()];
	float diff = 20.0f;

	egm->FlyRoute[0] = twp->pos;
	egm->FlyRoute[2].x = player->pos.x;
	egm->FlyRoute[2].y = player->pos.y + 10.0f;

	if (player->pos.z >= 980.0f)
	{
		diff = -20.0f;
	}

	egm->FlyRoute[2].z = diff + player->pos.z;
	egm->FlyRoute[1].x = (egm->FlyRoute[0].x + egm->FlyRoute[2].x) * 0.5f + 9.0f;
	egm->FlyRoute[1].y = (egm->FlyRoute[0].y + egm->FlyRoute[2].y) * 0.5f + 200.0f;
	egm->FlyRoute[1].z = (egm->FlyRoute[2].z + egm->FlyRoute[0].z) * 0.5f;
	SetRouteDelta_0(egm, 4.0f);
}

void setEgm1Missile_r(taskwk* twp)
{
	if (!multiplayer::IsActive())
	{
		return setEgm1Missile_h.Original(twp);
	}

	Egm1MissilesPrm egm1Mis;

	egm1Mis.Parent_twp = twp;
	egm1Mis.DstPosPtr = &playertwp[GetBossTargetPlayerRandom()]->pos;
	egm1Mis.FireInterval = 14;
	egm1Mis.HomingInterval = 22;
	egm1Mis.MissileNum = 6;
	sub_575190(&egm1Mis);
}

void setEgm1Ud_r(task* a1)
{
	if (!multiplayer::IsActive())
	{
		return SetEgm1Ud_h.Original(a1);
	}

	auto egm = (bossextwk*)a1->awp;
	auto twp = a1->twp;
	egm->UdFrq += 546;
	egm->UdFrq = 0;
	twp->pos.y = egm->BasePosY - njSin(egm->UdFrq) * 10.0f;
	twp->ang.y = -0x4000 - njArcTan2(twp->pos.z - playertwp[GetBossTargetPlayerRandom()]->pos.z,
		twp->pos.x - playertwp[GetBossTargetPlayerRandom()]->pos.x);
}

int __cdecl EH_GetVsPlayerAng_r(taskwk* a1)
{
	if (!multiplayer::IsActive())
	{
		return EH_GetVsPlayerAng_h.Original(a1);
	}

	return -0x4000 - njArcTan2(a1->pos.z - playertwp[GetBossTargetPlayerRandom()]->pos.z, a1->pos.x - playertwp[GetBossTargetPlayerRandom()]->pos.x);
}

void SetEgm1MoveRoute_r(task* tp, taskwk* twp, bossextwk* egm)
{
	if (!multiplayer::IsActive())
	{
		return SetEgm1MoveRoute_h.Original(tp, twp, egm);
	}

	float posZ = 0.0f;
	int AngleCalc = 0;
	NJS_VECTOR posFlyRoute = { 0 };
	NJS_VECTOR newposFlyRoute = { 0 };

	auto player = playertwp[GetBossTargetPlayerRandom()];

	egm->FlyRoute[0] = twp->pos;

	Egm1LocEnum AimArea = egm->AimArea;

	if (AimArea)
	{
		if (AimArea == EGM1AREA_CENTER)
		{
			egm->FlyRoute[2].x = 863.0f;
			egm->FlyRoute[2].y = 190.0f;
			egm->FlyRoute[2].z = 1312.0f;
			goto LABEL_7;
		}
		egm->FlyRoute[2].x = 609.0f;
		posZ = 1228.0f;
	}
	else
	{
		egm->FlyRoute[2].x = 1098.0f;
		posZ = 1258.0f;
	}
	egm->FlyRoute[2].y = 190.0f;
	egm->FlyRoute[2].z = posZ;

LABEL_7:
	if (egm->ShotNum < 1)
	{
		egm->FlyRoute[2].z = player->pos.z + 250.0f;
	}
	newposFlyRoute.x = (egm->FlyRoute[2].x + egm->FlyRoute[0].x) * 0.5f;
	egm->FlyRoute[1].x = newposFlyRoute.x;
	newposFlyRoute.y = (egm->FlyRoute[2].y + egm->FlyRoute[0].y) * 0.5f;
	egm->FlyRoute[1].y = newposFlyRoute.y;
	newposFlyRoute.z = (egm->FlyRoute[0].z + egm->FlyRoute[2].z) * 0.5f;
	egm->FlyRoute[1].z = newposFlyRoute.z;
	posFlyRoute.z = newposFlyRoute.z;
	posFlyRoute.x = newposFlyRoute.x - egm->FlyRoute[0].x;
	posFlyRoute.y = newposFlyRoute.y - egm->FlyRoute[0].y;
	posFlyRoute.z = posFlyRoute.z - egm->FlyRoute[0].z;
	egm->RotRBase = njScalor(&posFlyRoute);
	AngleCalc = 0x8000 - (unsigned __int64)(atan2(posFlyRoute.z, posFlyRoute.x) * 65536.0f * -0.1591549762031479f);

	Egm1LocEnum OldAreaCopy = egm->OldArea;

	egm->RotAng = AngleCalc;
	egm->RotAngOld = AngleCalc;

	if (OldAreaCopy && (OldAreaCopy == EGM1AREA_RIGHT || egm->AimArea == EGM1AREA_LEFT))
	{
		egm->bwk.req_action = 9;
		egm->RotAngSpd = -273;
	}
	else
	{
		egm->bwk.req_action = 7;
		egm->RotAngSpd = 273;
	}
}

int EH_PosPlayerCheck_r()
{
	if (multiplayer::IsActive())
	{
		int result = 0;

		if (playertwp[GetBossTargetPlayerRandom()]->pos.x >= 752.0f)
		{
			result = playertwp[GetBossTargetPlayerRandom()]->pos.x <= 1008.0f;
		}
		else
		{
			result = 2;
		}

		return result;
	}
	else
	{
		return EH_PosPlayerCheck_h.Original();
	}
}

void Egm1_r(task* tp)
{
	Egm1_h.Original(tp);
}

void EggHornetDisplay(task* tp)
{
	tp->disp(tp);
}

void patch_egm1_init()
{
	//Egm1_h.Hook(Egm1_r);
	EH_PosPlayerCheck_h.Hook(EH_PosPlayerCheck_r);
	EH_GetVsPlayerAng_h.Hook(EH_GetVsPlayerAng_r);
	SetEgm1MoveRoute_h.Hook(SetEgm1MoveRoute_r);
	SetEgm1Ud_h.Hook(setEgm1Ud_r);
	setEgm1Missile_h.Hook(setEgm1Missile_r);
	SetEgm1AtkRoute_h.Hook(SetEgm1AtkRoute_r);
	EggHornet_InitFight_h.Hook(EggHornet_InitFight);

	BossCheckDamage_h.Hook(BossCheckDamage);
	
	WriteCall((void*)0x57204C, EggHornetDisplay);
	WriteData<5>((int*)0x571D36, 0x90); //remove set velocity when getting hit, we will manually do it
}

#ifdef MULTI_TEST
	RegisterPatch patch_egm1(patch_egm1_init);
#endif
#include "pch.h"
#include "bosses.h"

static FunctionHook<void, task*> eggHornet_t(0x572010);
static FunctionHook<int> EH_PosPlayerCheck_t(0x573310);
static FunctionHook<int, taskwk*> EH_GetVsPlayerAng_t(0x573160);

UsercallFuncVoid(SetEgm1MoveRoute_t, (task* tp, taskwk* data, bossextwk* egm), (tp, data, egm), 0x5733E0, rECX, rEAX, rESI);
UsercallFuncVoid(SetEgm1Ud_t, (task* tp), (tp), 0x5736D0, rEAX);
UsercallFuncVoid(setEgm1Missile_t, (taskwk* data), (data), 0x573730, rEAX);
UsercallFuncVoid(SetEgm1AtkRoute_t, (bossextwk* egm, taskwk* data), (egm, data), 0x5735B0, rECX, rEAX);

static const int timeLimit = 400;

void SetEgm1AtkRoute_r(bossextwk* egm, taskwk* data)
{
	if (!multiplayer::IsActive())
	{
		return SetEgm1AtkRoute_t.Original(egm, data);
	}

	auto player = playertwp[randomPnum];
	float diff = 20.0f;

	egm->FlyRoute[0] = data->pos;
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

void setEgm1Missile_r(taskwk* data)
{
	if (!multiplayer::IsActive())
	{
		return setEgm1Missile_t.Original(data);
	}

	Egm1MissilesPrm egm1Mis;

	egm1Mis.Parent_twp = data;
	egm1Mis.DstPosPtr = &playertwp[randomPnum]->pos;
	egm1Mis.FireInterval = 14;
	egm1Mis.HomingInterval = 22;
	egm1Mis.MissileNum = 6;
	sub_575190(&egm1Mis);
}

void setEgm1Ud_r(task* a1)
{
	if (!multiplayer::IsActive())
	{
		return SetEgm1Ud_t.Original(a1);
	}

	auto egm = (bossextwk*)a1->awp;
	auto data = a1->twp;
	egm->UdFrq += 546;
	egm->UdFrq = 0;
	data->pos.y = egm->BasePosY - njSin(egm->UdFrq) * 10.0f;
	data->ang.y = -16384
		- (unsigned __int64)(atan2(
			data->pos.z - playertwp[randomPnum]->pos.z,
			data->pos.x - playertwp[randomPnum]->pos.x)
			* 65536.0f
			* 0.1591549762031479f);
}

int __cdecl EH_GetVsPlayerAng_r(taskwk* a1)
{
	if (!multiplayer::IsActive())
	{
		return EH_GetVsPlayerAng_t.Original(a1);
	}

	return -16384
		- (unsigned __int64)(atan2(a1->pos.z - playertwp[randomPnum]->pos.z, a1->pos.x - playertwp[randomPnum]->pos.x)
			* 65536.0f
			* 0.1591549762031479f);
}

void SetEgm1MoveRoute_r(task* tp, taskwk* data, bossextwk* egm)
{
	if (!multiplayer::IsActive())
	{
		return SetEgm1MoveRoute_t.Original(tp, data, egm);
	}

	float posZ = 0.0f;
	int AngleCalc = 0;
	NJS_VECTOR posFlyRoute = { 0 };
	NJS_VECTOR newposFlyRoute = { 0 };

	auto player = playertwp[randomPnum];

	egm->FlyRoute[0] = data->pos;

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

		if (playertwp[randomPnum]->pos.x >= 752.0f)
		{
			result = playertwp[randomPnum]->pos.x <= 1008.0f;
		}
		else
		{
			result = 2;
		}

		return result;
	}
	else
	{
		return EH_PosPlayerCheck_t.Original();
	}
}

void eggHornet_r(task* tp)
{
	if (tp->twp && !tp->twp->mode)
		ResetBossRNG();

	eggHornet_t.Original(tp);
	Boss_SetNextPlayerToAttack(timeLimit);
}

void initEggHornetPatches()
{
	eggHornet_t.Hook(eggHornet_r);

	EH_PosPlayerCheck_t.Hook(EH_PosPlayerCheck_r);
	EH_GetVsPlayerAng_t.Hook(EH_GetVsPlayerAng_r);
	SetEgm1MoveRoute_t.Hook(SetEgm1MoveRoute_r);
	SetEgm1Ud_t.Hook(setEgm1Ud_r);
	setEgm1Missile_t.Hook(setEgm1Missile_r);
	SetEgm1AtkRoute_t.Hook(SetEgm1AtkRoute_r);
}
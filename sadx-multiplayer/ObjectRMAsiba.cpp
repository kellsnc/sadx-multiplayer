#include "pch.h"

TaskFunc(ObjectMountainAsiba, 0x60E150);
TaskFunc(OMasiba_Display, 0x60DEA0);

static void __cdecl ObjectMountainAsiba_r(task* obj);
Trampoline ObjectMountainAsiba_t(0x60E150, 0x60E156, ObjectMountainAsiba_r);

static auto Masiba_InitDyncol = GenerateUsercallWrapper<void (*)(task* a1)>(noret, 0x60DDB0, rEDI); // custom name original is "init_9"

static _BOOL1 sub_60DE20(taskwk* a1, char pnum)
{
	task* player; 
	taskwk* playerData;
	double calcPosX;
	double calcPosZ;
	double calcSin;
	float resultPos; 
	float calcPosY; 

	player = (task*)GetCharacterObject(pnum);

	if (player)
	{
		playerData = player->twp;
		calcPosX = playerData->pos.x - a1->pos.x;
		calcPosZ = playerData->pos.z - a1->pos.z;
		resultPos = calcPosX * calcPosX + calcPosZ * calcPosZ;
		calcPosY = playerData->pos.y - a1->pos.y;
		calcSin = njSin(9102) * 50.0;
		if (resultPos < 2500.0 && fabs(calcPosY) < calcSin)
		{
			return true;
		}
	}

	return false;
}

static void OMasiba_Exec_r(task* tp)
{	
	int result;
	int btimer;
	signed int totalTime;
	double posX;
	double calcPosX;
	double calcPosY;
	__int64 resultPosPlayerXY;
	double calcPosYAgain;
	double calcPosZ;
	__int64 resultPosPlayerYZ;
	int calcAngZ;

	taskwk* data = tp->twp;
	result = 0;
	int posPlay = 0;
	forcewk* fwp = tp->fwp;

	char pnum = GetTheNearestPlayerNumber(&data->pos);
	task* player = (task*)GetCharacterObject(pnum);

	if (!player)
		return;

	taskwk* playerData = player->twp;

	if (!rd_mountain_twp || rd_mountain_twp->scl.y == 0.0 && rd_mountain_twp->counter.f == 0.0)
		fwp->pos_spd.y = 0.0;
	else
		fwp->pos_spd.y = rd_mountain_twp->counter.f;

	if (sub_60DE20(data, pnum))
	{
		btimer = data->btimer;

		if (!data->btimer)
			data->value.f = -0.035;

		totalTime = btimer + 1;

		if (200 / totalTime >= 1)
			data->btimer = totalTime;
		else
			data->value.f = 0.0;

		posX = data->pos.x;

		data->wtimer += 200 / totalTime;

		calcPosX = playerData->pos.x - (float)posX;
		calcPosY = playerData->pos.y - data->pos.y;

		if (calcPosX <= 0.0)
			(resultPosPlayerXY) = (int)(atan2(fabs(calcPosX), calcPosY) * 65536.0 * 0.1591549762031479);
		else
			(resultPosPlayerXY) = (int)(atan2(calcPosX, calcPosY) * 65536.0 * -0.1591549762031479);

		posPlay = (int)(float)((float)resultPosPlayerXY * (float)0.083333336);

		calcPosZ = playerData->pos.z - data->pos.y;
		calcPosYAgain = playerData->pos.y - data->pos.y;

		if (calcPosZ <= 0.0)
			(resultPosPlayerYZ) = (int)(atan2(fabs(calcPosZ), calcPosYAgain) * 65536.0 * -0.1591549762031479);
		else
			(resultPosPlayerYZ) = (int)(atan2(calcPosZ, calcPosYAgain) * 65536.0 * 0.1591549762031479);

		result = (int)(float)((float)resultPosPlayerYZ * (float)0.083333336);
	}
	else
	{
		data->btimer = 0;
		data->wtimer = 0;
	}

	data->ang.x = AdjustAngle(data->ang.x, 2 * result, 15);
	calcAngZ = AdjustAngle(data->ang.z, 2 * posPlay, 15);
	data->ang.z = calcAngZ;
	data->pos.y += fwp->pos_spd.y;
	WithdrawCollisionEntry(tp, (obj*)data->counter.l);
	ReleaseMobileLandObject((obj*)data->counter.l);
	Masiba_InitDyncol(tp);

	if (CheckCollisionP(&data->pos, 90.0))
		data->flag |= 0x100u;
	else
		data->flag &= 0xFEFFu;
}

static void __cdecl ObjectMountainAsiba_r(task* obj)
{
	if (!multiplayer::IsActive())
	{
		TARGET_STATIC(ObjectMountainAsiba);
		return;
	}

	if (CheckRangeOut(obj))
		return;

	taskwk* data = obj->twp;

	if (data->mode)
	{
		if (data->mode == 1 && !ObjectSelectedDebug((ObjectMaster*)obj))
		{
			OMasiba_Exec_r(obj);
			OMasiba_Display(obj);
			return;
		}
	}
	else
	{
		obj->dest = (TaskFuncPtr)0x541360;
		obj->disp = OMasiba_Display;
		Masiba_InitDyncol(obj);
		data->mode = 1;

		if (rd_mountain_twp)
		{
			if (!ObjectSelectedDebug((ObjectMaster*)obj))
			{
				data->pos.y = rd_mountain_twp->pos.y;
			}
		}
	}

	OMasiba_Display(obj);
}
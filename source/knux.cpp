#include "pch.h"

UsercallFunc(signed int, Knux_CheckNact_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x476970, rEAX, rEDI, rESI, stack4);
TaskHook KnuxExec_t((intptr_t)Knuckles_Main);
static FunctionHook<void, taskwk*, motionwk2*, playerwk*> Knux_RunsActions_t(Knux_RunsActions);
TaskHook KnucklesChargeEffectExe_t((intptr_t)0x473FE0);

void __cdecl KnuEffectChargeUp_r(task* a1)
{
	Angle result = 0;
	float cos = 0.0f;
	task* ChargeUpTask = nullptr;
	NJS_VECTOR* chargeUpPos = nullptr;

	auto data = a1->twp;
	auto timer = data->timer.f + 0.055555552f;
	data->timer.f = *(double*)&timer;
	auto pData = playertwp[data->counter.b[0]];

	if (timer < 1.0f && pData)
	{
		result = data->ang.y + data->ang.x;
		data->ang.x = result;
		cos = njCos(result);
		data->pos.x = cos * data->scl.y + pData->pos.x;
		data->pos.y = pData->pos.y + data->scl.x;
		data->pos.z = njSin(data->ang.x) * data->scl.y + pData->pos.z;
		auto idk = (1.0f - data->timer.f) * data->value.f;
		KnuEffectDrawTsubu(&data->pos, idk);
		ChargeUpTask = CreateElementalTask(LoadObj_Data1, 6, KnuEffectChargeUpStay);
		if (ChargeUpTask)
		{
			chargeUpPos = &data->pos;
			ChargeUpTask->twp->value.f = idk;
			ChargeUpTask->disp = dispKnuEffectChargeUpStay;
		}
	}
	else
	{
		FreeTask(a1);
	}
}

void __cdecl KnuEffectPutChargeUp_r(float alpha, taskwk* pData)
{
	if (pData)
	{
		auto task = CreateElementalTask(LoadObj_Data1, 6, KnuEffectChargeUp_r);

		if (task)
		{
			float calcAlpha = 16.0f - (alpha * -4096.0f);
			if ((float)rand() * 0.000030517578f < 0.5f)
			{
				calcAlpha = (alpha * -4096.0f) - 16.0f;
			}
			task->twp->scl.x = rand() * 0.000030517578f * 8.0f + 1.0f;
			task->twp->scl.y = (rand() * 0.000030517578f + 1.0f) * 2.5f;
			task->twp->ang.x = (unsigned __int64)(rand() * 0.000030517578f * 65536.0f);
			task->twp->ang.y = calcAlpha;
			task->twp->value.f = alpha;
			task->twp->counter.b[0] = pData->counter.b[0]; //store pNum
			task->disp = dispKnuEffectChargeUpStay;
		}
	}
}

void __cdecl KnucklesChargeEffectExe_r(task* a1)
{
	if (!multiplayer::IsActive())
	{
		return KnucklesChargeEffectExe_t.Original(a1);
	}

	_BOOL1 v5 = false;
	float alph = 0.0f;
	float alpha = 0.0f;

	auto data = a1->twp;
	auto PlayerData = playertwp[(unsigned __int8)data->counter.b[0]];

	if (!PlayerData)
	{
		DestroyTask(a1);
		return;
	}

	auto timer = data->btimer + 1;
	data->btimer = timer;
	auto v4 = data->value.l; //????????????????????????????????????????

	if (v4 > 0)
	{
		v5 = v4 == 120;
		if (v4 >= 120)
		{
			if (v5) //never reached
			{
				alpha = (njSin(timer << 10) + 1.0f) * 0.5f;
				KnuEffectPutChargeComp(&PlayerData->cwp->info->center, alpha);
			}
		}

		if ((v4 & 3) == 0) //never reached
		{
			alph = v4 * 0.0083333338f;
			KnuEffectPutChargeUp_r(alph, PlayerData);
			return;
		}
	}
}

signed int Knux_CheckNAct_r(playerwk* co2, taskwk* data, motionwk2* data2)
{
	auto even = data->ewp;

	if (even->move.mode || even->path.list || ((data->flag & Status_DoNextAction) == 0))
	{
		return Knux_CheckNact_t.Original(co2, data, data2);
	}

	switch (data->smode)
	{
	case 5:
		data->mode = SDCannonMode;
		co2->mj.reqaction = 19;
		return 1;
	}

	return Knux_CheckNact_t.Original(co2, data, data2);
}

void Knux_RunsActions_r(taskwk* data1, motionwk2* data2, playerwk* co2) {

	switch (data1->mode)
	{
	case SDCannonMode:
		if (!KnucklesCheckInput(data1, data2, co2) && (data1->flag & 3) != 0)
		{
			if (PCheckBreak(data1) && co2->spd.x > 0.0f)
			{
				data1->mode = 10;
			}
			if (!KnucklesCheckStop(data1, co2))
			{
				data1->mode = 2;
			}

			data1->ang.x = data2->ang_aim.x;
			data1->ang.z = data2->ang_aim.z;
			co2->mj.reqaction = 2;
		}
		return;
	}

	Knux_RunsActions_t.Original(data1, data2, co2);
}

void KnuxExec_r(task* obj)
{
	auto data = obj->twp;
	motionwk2* data2 = (motionwk2*)obj->mwp;
	playerwk* co2 = (playerwk*)obj->mwp->work.l;

	switch (data->mode)
	{
	case SDCannonMode:
		CannonModePhysics(data, data2, co2);
		break;
	}

	KnuxExec_t.Original(obj);
}

void Init_KnuxPatches()
{
	KnuxExec_t.Hook(KnuxExec_r);
	Knux_CheckNact_t.Hook(Knux_CheckNAct_r);
	Knux_RunsActions_t.Hook(Knux_RunsActions_r);
	KnucklesChargeEffectExe_t.Hook(KnucklesChargeEffectExe_r);
}
#include "pch.h"
#include "ObjCylinderCmn.h"
#include "e_cart.h"

UsercallFunc(Bool, Knux_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x476970, rEAX, rEDI, rESI, stack4);
TaskHook KnuxExec_t((intptr_t)Knuckles_Main);
static FunctionHook<void, taskwk*, motionwk2*, playerwk*> Knux_RunsActions_t(Knux_RunsActions);
TaskHook KnucklesChargeEffectExe_t((intptr_t)0x473FE0);
static FunctionHook<void, NJS_VECTOR*, float> KnuEffectPutChargeComp_t((intptr_t)0x4C1330);
TaskHook KnucklesJiggle_t(0x473CE0);
TaskHook KnuxEyeTracker_t(0x475260);

static void __cdecl KnuxEyeTracker_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	KnuxEyeTracker_t.Original(tp);
}

static void __cdecl KnucklesJiggle_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	KnucklesJiggle_t.Original(tp);
}

void __cdecl KnuEffectPutChargeComp_r(NJS_VECTOR* position, float alpha)
{
	if (!multiplayer::IsActive())
	{
		return KnuEffectPutChargeComp_t.Original(position, alpha);
	}

	auto y = 0.0f;
	auto a = 0.0f;
	auto s = 0.0f;

	if (!MissedFrames)
	{
		a = alpha * -0.60000002;
		auto obj = (NJS_MODEL_SADX*)KNUCKLES_OBJECTS[47]->model;
		obj->mats->attr_texId = 1;
		SetMaterialAndSpriteColor_Float(a, 1.0, 1.0, 1.0);
		njPushMatrix(0);
		njTranslateV(0, position);
		s = 2.0f - alpha * alpha;
		njScale(0, s, s, s);
		y = alpha * 262144.0f;
		if ((unsigned int)(unsigned __int64)y)
		{
			njRotateY(0, (unsigned __int16)(unsigned __int64)y);
		}
		njSetTexture(&KNU_EFF_TEXLIST);
		ProcessModelNode_A_WrapperB(KNUCKLES_OBJECTS[47], QueuedModelFlagsB_SomeTextureThing);
		njPopMatrix(1u);
		ResetMaterial();
	}
}

void __cdecl KnuEffectChargeUp_r(task* tp)
{
	auto twp = tp->twp;
	twp->timer.f += 0.055555552f;
	auto pData = playertwp[TASKWK_PLAYERID(twp)];

	if (twp->timer.f < 1.0f && pData)
	{
		twp->ang.x += twp->ang.y;
		twp->pos.x = njCos(twp->ang.x) * twp->scl.y + pData->pos.x;
		twp->pos.y = pData->pos.y + twp->scl.x;
		twp->pos.z = njSin(twp->ang.x) * twp->scl.y + pData->pos.z;

		auto scl = (1.0f - twp->timer.f) * twp->value.f;
		KnuEffectDrawTsubu(&twp->pos, scl);

		task* ChargeUpTask = CreateElementalTask(LoadObj_Data1, 6, KnuEffectChargeUpStay);
		if (ChargeUpTask)
		{
			ChargeUpTask->twp->pos = twp->pos;
			ChargeUpTask->twp->value.f = scl;
			ChargeUpTask->disp = dispKnuEffectChargeUpStay;
		}
	}
	else
	{
		FreeTask(tp);
	}
}

void __cdecl KnuEffectPutChargeUp_r(Float alpha, taskwk* pData)
{
	if (pData)
	{
		auto task = CreateElementalTask(LoadObj_Data1, 6, KnuEffectChargeUp_r);

		if (task)
		{
			Float calcAlpha = 16.0f - (alpha * -4096.0f);
			if (njRandom() < 0.5f)
			{
				calcAlpha = (alpha * -4096.0f) - 16.0f;
			}
			task->twp->scl.x = njRandom() * 8.0f + 1.0f;
			task->twp->scl.y = (njRandom() + 1.0f) * 2.5f;
			task->twp->ang.x = njRandom() * 0x10000;
			task->twp->ang.y = calcAlpha;
			task->twp->value.f = alpha;
			TASKWK_PLAYERID(task->twp) = TASKWK_PLAYERID(pData);
			task->disp = dispKnuEffectChargeUpStay;
		}
	}
}

void __cdecl KnucklesChargeEffectDisp(task* tp)
{
	auto twp = tp->twp;
	auto wk = (int*)twp->value.ptr;
	auto timer = *wk;
	auto ptwp = playertwp[TASKWK_PLAYERID(twp)];

	if (ptwp && timer == 120)
	{
		tp->disp = KnucklesChargeEffectDisp;
		KnuEffectPutChargeComp(&ptwp->cwp->info->center, (njSin(twp->btimer << 10) + 1.0f) * 0.5f);
	}
}

void __cdecl KnucklesChargeEffectExe_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		return KnucklesChargeEffectExe_t.Original(tp);
	}

	auto twp = tp->twp;
	auto PlayerData = playertwp[TASKWK_PLAYERID(twp)];

	if (!PlayerData)
	{
		DestroyTask(tp);
		return;
	}

	twp->btimer += 1;

	auto wk = (int*)twp->value.ptr;
	auto timer = *wk;

	if (timer > 0 && timer < 120.)
	{
		if ((timer & 3) == 0)
		{
			KnuEffectPutChargeUp_r(timer / 120.0f, PlayerData);
			return;
		}
	}

	if (timer == 120)
	{
		tp->disp = KnucklesChargeEffectDisp;
		KnuEffectPutChargeComp(&PlayerData->cwp->info->center, (njSin(twp->btimer << 10) + 1.0f) * 0.5f);
	}
}

Bool Knux_CheckInput_r(playerwk* co2, taskwk* twp, motionwk2* data2)
{
	auto even = twp->ewp;

	if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
	{
		return Knux_CheckInput_t.Original(co2, twp, data2);
	}

	switch (twp->smode)
	{
	case 5:
		if (CurrentLevel == LevelIDs_SkyDeck)
		{
			twp->mode = SDCannonMode;
			co2->mj.reqaction = 19;
			return 1;
		}
		break;
	case 32:

		if (SetCylinderNextAction(twp, data2, co2))
			return 1;

		break;
	}

	return Knux_CheckInput_t.Original(co2, twp, data2);
}

void Knux_RunsActions_r(taskwk* data1, motionwk2* data2, playerwk* co2) {
	switch (data1->mode)
	{
	case 52: //cart
		KillPlayerInKart(data1, co2, 51, 27);
		break;
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
	case SDCylStd:
		if (KnucklesCheckInput(data1, data2, co2) || KnucklesCheckJump(data1, co2))
		{
			co2->htp = 0;
			return;
		}

		Mode_SDCylStdChanges(data1, co2);
		return;
	case SDCylDown:

		if (KnucklesCheckInput(data1, data2, co2) || KnucklesCheckJump(data1, co2))
		{
			co2->htp = 0;
			return;
		}

		Mode_SDCylDownChanges(data1, co2);

		return;
	case SDCylLeft:
		if (KnucklesCheckInput(data1, data2, co2) || KnucklesCheckJump(data1, co2))
		{
			co2->htp = 0;
			return;
		}

		if (Controllers[(unsigned __int8)data1->counter.b[0]].LeftStickX << 8 <= -3072)
		{
			if (data1->mode < SDCylStd || data1->mode > SDCylRight)
			{
				co2->htp = 0;
			}

			return;
		}
		data1->mode = SDCylStd;

		return;
	case SDCylRight:
		if (KnucklesCheckInput(data1, data2, co2) || KnucklesCheckJump(data1, co2))
		{
			co2->htp = 0;
			return;
		}

		if (Controllers[(unsigned __int8)data1->counter.b[0]].LeftStickX << 8 >= 3072)
		{
			if (data1->mode < SDCylStd || data1->mode > SDCylRight)
			{
				co2->htp = 0;
			}
			return;
		}

		data1->mode = SDCylStd;
		return;
	}

	Knux_RunsActions_t.Original(data1, data2, co2);
}

void KnuxExec_r(task* obj)
{
	auto twp = obj->twp;
	auto data = twp;
	motionwk2* data2 = (motionwk2*)obj->mwp;
	playerwk* co2 = (playerwk*)obj->mwp->work.l;

	switch (twp->mode)
	{
	case SDCannonMode:
		CannonModePhysics(twp, data2, co2);
		break;
	case SDCylStd:
		Mode_SDCylinderStd(data, co2);
		break;
	case SDCylDown:
		Mode_SDCylinderDown(data, co2);
		break;
	case SDCylLeft:
		Mode_SDCylinderLeft(data, co2);
		break;
	case SDCylRight:
		Mode_SDCylinderRight(data, co2);
		break;
	}

	KnuxExec_t.Original(obj);
}

void Init_KnuxPatches()
{
	KnuxExec_t.Hook(KnuxExec_r);
	Knux_CheckInput_t.Hook(Knux_CheckInput_r);
	Knux_RunsActions_t.Hook(Knux_RunsActions_r);
	KnucklesChargeEffectExe_t.Hook(KnucklesChargeEffectExe_r);

	KnuEffectPutChargeComp_t.Hook(KnuEffectPutChargeComp_r); //add support DC Conv
	KnucklesJiggle_t.Hook(KnucklesJiggle_r);
	KnuxEyeTracker_t.Hook(KnuxEyeTracker_r);
}
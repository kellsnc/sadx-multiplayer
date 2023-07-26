#include "pch.h"
#include "gravity.h"
#include "e_cart.h"
#include "result.h"
#include "ObjCylinderCmn.h"

UsercallFunc(Bool, Knux_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x476970, rEAX, rEDI, rESI, stack4);
TaskHook KnucklesTheEchidna_t((intptr_t)Knuckles_Main);
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

	if (timer > 0 && timer < 120)
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

Bool Knux_CheckInput_r(playerwk* pwp, taskwk* twp, motionwk2* mwp)
{
	if (multiplayer::IsActive())
	{
		auto even = twp->ewp;
		auto pnum = TASKWK_PLAYERID(twp);

		if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
		{
			return Knux_CheckInput_t.Original(pwp, twp, mwp);
		}

		switch (twp->smode)
		{
		case PL_OP_PARABOLIC:
			if (CurrentLevel != LevelIDs_Casinopolis)
			{
				twp->mode = SDCannonMode;
				pwp->mj.reqaction = 19;
				return TRUE;
			}
			break;
		case PL_OP_PLACEWITHKIME:
			if (CheckDefeat(pnum))
			{
				twp->mode = 16;
				pwp->mj.reqaction = 27;
				dsStop_num(SE_K_GLIDE);
				twp->ang.z = 0;
				twp->ang.x = 0;
				PClearSpeed(mwp, pwp);
				twp->flag &= ~0x2500;
				CancelLookingAtP(pnum);
				return TRUE;
			}
			break;
		case PL_OP_HOLDONPILLAR:
			if (SetCylinderNextAction(twp, mwp, pwp))
				return TRUE;
			break;
		}
	}
	
	return Knux_CheckInput_t.Original(pwp, twp, mwp);
}

void __cdecl Knux_RunsActions_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (multiplayer::IsActive())
	{
		switch (twp->mode)
		{
		case 52: //cart
			KillPlayerInKart(twp, pwp, 51, 27);
			break;
		case SDCannonMode:
			if (!KnucklesCheckInput(twp, mwp, pwp) && (twp->flag & 3) != 0)
			{
				if (PCheckBreak(twp) && pwp->spd.x > 0.0f)
				{
					twp->mode = 10;
				}
				if (!KnucklesCheckStop(twp, pwp))
				{
					twp->mode = 2;
				}

				twp->ang.x = mwp->ang_aim.x;
				twp->ang.z = mwp->ang_aim.z;
				pwp->mj.reqaction = 2;
			}
			return;
		case SDCylStd:
			if (KnucklesCheckInput(twp, mwp, pwp) || KnucklesCheckJump(twp, pwp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_SDCylStdChanges(twp, pwp);
			return;
		case SDCylDown:

			if (KnucklesCheckInput(twp, mwp, pwp) || KnucklesCheckJump(twp, pwp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_SDCylDownChanges(twp, pwp);

			return;
		case SDCylLeft:
			if (KnucklesCheckInput(twp, mwp, pwp) || KnucklesCheckJump(twp, pwp))
			{
				pwp->htp = 0;
				return;
			}

			if (Controllers[TASKWK_PLAYERID(twp)].LeftStickX << 8 <= -3072)
			{
				if (twp->mode < SDCylStd || twp->mode > SDCylRight)
				{
					pwp->htp = 0;
				}

				return;
			}
			twp->mode = SDCylStd;

			return;
		case SDCylRight:
			if (KnucklesCheckInput(twp, mwp, pwp) || KnucklesCheckJump(twp, pwp))
			{
				pwp->htp = 0;
				return;
			}

			if (Controllers[TASKWK_PLAYERID(twp)].LeftStickX << 8 >= 3072)
			{
				if (twp->mode < SDCylStd || twp->mode > SDCylRight)
				{
					pwp->htp = 0;
				}
				return;
			}

			twp->mode = SDCylStd;
			return;
		}
	}
	
	Knux_RunsActions_t.Original(twp, mwp, pwp);
}

void KnucklesTheEchidna_m(task* tp)
{
	auto twp = tp->twp;
	auto mwp = (motionwk2*)tp->mwp;
	auto pwp = (playerwk*)mwp->work.ptr;

	switch (twp->mode)
	{
	case SDCannonMode:
		CannonModePhysics(twp, mwp, pwp);
		break;
	case SDCylStd:
		Mode_SDCylinderStd(twp, pwp);
		break;
	case SDCylDown:
		Mode_SDCylinderDown(twp, pwp);
		break;
	case SDCylLeft:
		Mode_SDCylinderLeft(twp, pwp);
		break;
	case SDCylRight:
		Mode_SDCylinderRight(twp, pwp);
		break;
	}

	KnucklesTheEchidna_t.Original(tp);
}

void __cdecl KnucklesTheEchidna_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		gravity::SaveGlobalGravity();
		gravity::SwapGlobalToUserGravity(TASKWK_PLAYERID(tp->twp));
		KnucklesTheEchidna_m(tp);
		gravity::RestoreGlobalGravity();
	}
	else
	{
		KnucklesTheEchidna_t.Original(tp);
	}
}

void Init_KnuxPatches()
{
	KnucklesTheEchidna_t.Hook(KnucklesTheEchidna_r);
	Knux_CheckInput_t.Hook(Knux_CheckInput_r);
	Knux_RunsActions_t.Hook(Knux_RunsActions_r);
	KnucklesChargeEffectExe_t.Hook(KnucklesChargeEffectExe_r);

	KnuEffectPutChargeComp_t.Hook(KnuEffectPutChargeComp_r); //add support DC Conv
	KnucklesJiggle_t.Hook(KnucklesJiggle_r);
	KnuxEyeTracker_t.Hook(KnuxEyeTracker_r);
}

#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "gravity.h"
#include "patch_e_cart.h"
#include "result.h"
#include "collision.h"
#include "patch_player.h"
#include "patch_o_sky_cyl_cmn.h"

FastFunctionHook<void> KnuEffectPutCharge0_h(0x4C2210);
FastFunctionHook<void> KnuEffectPutCharge1_h(0x4C2260);
FastUsercallHookPtr<taskwk*(*)(taskwk* twp), rEAX, rEBX> KnucklesGetNearEnemyTWP_h(0x4756C0);
FastFunctionHook<void, task*> KnucklesChargeEffectExe_h((intptr_t)0x473FE0);
FastFunctionHook<void, NJS_VECTOR*, float> KnuEffectPutChargeComp_h((intptr_t)0x4C1330);
FastFunctionHook<void, task*> KnucklesJiggle_h(0x473CE0);
FastFunctionHook<void, task*> KnuxEyeTracker_h(0x475260);
FastUsercallHookPtr<Bool(*)(taskwk* twp, playerwk* pwp, motionwk2* mwp), rEAX, rESI, rEDI, stack4> Knux_CheckInput_h(0x476970);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> Knux_RunsActions_h(Knux_RunsActions);
FastFunctionHook<void, task*> KnucklesTheEchidna_h((intptr_t)Knuckles_Main);
FastFunctionHook<Bool, taskwk*> KnucklesCheckOutFDPolygon_h(0x4751B0);

static void __cdecl dispKnuEffectChargeScl_m(task* tp)
{
	taskwk* twp = tp->twp;
	taskwk* ptwp = playertwp[twp->btimer];

	if (!ptwp || loop_count)
	{
		return;
	}

	KNUCKLES_OBJECTS[47]->basicdxmodel->mats[0].attr_texId = 2;
	Float scl = (twp->scl.z - twp->scl.y) * twp->timer.f + twp->scl.y;
	SetMaterial((twp->counter.f - twp->value.f) * twp->timer.f + twp->value.f, 1.0f, 1.0f, 1.0f);
	njPushMatrix(0);
	njTranslateV(0, &ptwp->cwp->info->center);
	njScale(0, scl, scl, scl);
	njSetTexture(&KNU_EFF_TEXLIST);
	late_DrawObject(KNUCKLES_OBJECTS[47], LATE_LIG);
	njPopMatrix(1);
	ResetMaterial();
}

static void __cdecl KnuEffectChargeScl_m(task* tp)
{
	taskwk* twp = tp->twp;
	taskwk* ptwp = playertwp[twp->btimer];

	if (ptwp)
	{
		twp->timer.f += twp->scl.x;

		if (twp->timer.f < 1.0f)
		{
			dispKnuEffectChargeScl_m(tp);
			return;
		}
	}

	FreeTask(tp);
}

static void __cdecl KnuEffectPutCharge0_r()
{
	if (!multiplayer::IsActive())
	{
		KnuEffectPutCharge0_h.Original();
		return;
	}

	task* tp = CreateElementalTask(0x2, LEV_6, KnuEffectChargeScl_m);
	if (tp)
	{
		taskwk* twp = tp->twp;
		tp->disp = dispKnuEffectChargeScl_m;
		twp->scl.x = 0.055555552f; // 1/18
		twp->scl.y = 6.0f;
		twp->scl.z = 1.2f;
		twp->value.f = 0.0f;
		twp->counter.f = -1.0f;
		twp->btimer = TASKWK_PLAYERID(gpCharTwp);
	}
}

static void __cdecl KnuEffectPutCharge1_r()
{
	if (!multiplayer::IsActive())
	{
		KnuEffectPutCharge1_h.Original();
		return;
	}

	task* tp = CreateElementalTask(0x2, LEV_6, KnuEffectChargeScl_m);
	if (tp)
	{
		taskwk* twp = tp->twp;
		tp->disp = dispKnuEffectChargeScl_m;
		twp->scl.x = 0.041666668f; // 1/24
		twp->scl.y = 0.6f;
		twp->scl.z = 4.5f;
		twp->value.f = 0.0f;
		twp->counter.f = -1.0f;
		twp->btimer = TASKWK_PLAYERID(gpCharTwp);
	}
}

static taskwk* KnucklesGetNearEnemyTWP_m(taskwk* twp)
{
	auto ael = GetTargetEnemyList(TASKWK_PLAYERID(twp));

	taskwk* tgt_twp = NULL;
	Float tgt_dist = 160000.0f;
	Angle tgt_ang;

	if (ael->twp)
	{
		do
		{
			CCL_INFO* cinfo = ael->twp->cwp->info;
			NJS_VECTOR center = cinfo->center;
			if (!(cinfo->attr & 0x20))
			{
				njAddVector(&center, &ael->twp->pos);
			}
			njSubVector(&center, &twp->pos);
			if (DiffAngle(twp->ang.y, njArcTan2(center.z, center.x)) <= 0x7FFF && tgt_dist > ael->dist)
			{
				tgt_twp = ael->twp;
				tgt_dist = ael->dist;
				tgt_ang = njArcTan2(center.z, center.x);
			}
			++ael;
		}
		while (ael->twp);

		if (tgt_twp)
		{
			twp->ang.y = AdjustAngle(twp->ang.y, tgt_ang, 0x1000);
			KnuEffectHormTubePut(twp);
		}

		if (tgt_dist <= 225.0f)
		{
			SetEffectSpray(&twp->pos, NULL);
		}
	}

	return tgt_twp;
}

static taskwk* __cdecl KnucklesGetNearEnemyTWP_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		return KnucklesGetNearEnemyTWP_m(twp);
	}
	else
	{
		return KnucklesGetNearEnemyTWP_h.Original(twp);
	}
}

void __cdecl KnuEffectPutChargeComp_r(NJS_VECTOR* position, float alpha)
{
	if (!multiplayer::IsActive())
	{
		return KnuEffectPutChargeComp_h.Original(position, alpha);
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
		return KnucklesChargeEffectExe_h.Original(tp);
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

// Not yet supported in multiplayer
static void __cdecl KnuxEyeTracker_r(task* tp)
{
	if (multiplayer::IsActive() && !canselEvent)
	{
		FreeTask(tp);
		return;
	}

	KnuxEyeTracker_h.Original(tp);
}

// Not yet supported in multiplayer
static void __cdecl KnucklesJiggle_r(task* tp)
{
	if (multiplayer::IsActive() && !canselEvent)
	{
		FreeTask(tp);
		return;
	}

	KnucklesJiggle_h.Original(tp);
}

Bool Knux_CheckInput_r(taskwk* twp, playerwk* pwp, motionwk2* mwp)
{
	if (multiplayer::IsActive())
	{
		auto even = twp->ewp;
		auto pnum = TASKWK_PLAYERID(twp);

		if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
		{
			return Knux_CheckInput_h.Original(twp, pwp, mwp);
		}

		switch ((uint8_t)twp->smode)
		{
		case PL_OP_PARABOLIC:
			if (CurrentLevel != LevelIDs_Casinopolis)
			{
				twp->mode = MD_MULTI_PARA;
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
		case PL_OP_PINBALL:
			twp->mode = MD_MULTI_S9A1_PINB;
			pwp->mj.reqaction = 14;
			twp->flag = twp->flag & ~(Status_OnPath | Status_LightDash) | Status_Attack | Status_Ball;
			return TRUE;
		}
	}
	
	return Knux_CheckInput_h.Original(twp, pwp, mwp);
}

void __cdecl Knux_RunsActions_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (multiplayer::IsActive())
	{
		switch ((uint8_t)twp->mode)
		{
		case MD_KNUCK_S3A2_CART: // Allow death incarts
			if ((twp->flag & Status_DoNextAction) && twp->smode == PL_OP_KILLED)
			{
				twp->mode = MD_KNUCK_KILL;
				pwp->mj.reqaction = 27;
			}
			break;
		case MD_MULTI_PARA:
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
		case MD_MULTI_S6A1_WAIT:
			if (KnucklesCheckInput(twp, mwp, pwp) || KnucklesCheckJump(twp, pwp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_MD_MULTI_S6A1_WAITChanges(twp, pwp);
			return;
		case MD_MULTI_S6A1_SLID:

			if (KnucklesCheckInput(twp, mwp, pwp) || KnucklesCheckJump(twp, pwp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_MD_MULTI_S6A1_SLIDChanges(twp, pwp);

			return;
		case MD_MULTI_S6A1_LROT:
			if (KnucklesCheckInput(twp, mwp, pwp) || KnucklesCheckJump(twp, pwp))
			{
				pwp->htp = 0;
				return;
			}

			if (Controllers[TASKWK_PLAYERID(twp)].LeftStickX << 8 <= -3072)
			{
				if (twp->mode < MD_MULTI_S6A1_WAIT || twp->mode > MD_MULTI_S6A1_RROT)
				{
					pwp->htp = 0;
				}

				return;
			}
			twp->mode = MD_MULTI_S6A1_WAIT;

			return;
		case MD_MULTI_S6A1_RROT:
			if (KnucklesCheckInput(twp, mwp, pwp) || KnucklesCheckJump(twp, pwp))
			{
				pwp->htp = 0;
				return;
			}

			if (Controllers[TASKWK_PLAYERID(twp)].LeftStickX << 8 >= 3072)
			{
				if (twp->mode < MD_MULTI_S6A1_WAIT || twp->mode > MD_MULTI_S6A1_RROT)
				{
					pwp->htp = 0;
				}
				return;
			}

			twp->mode = MD_MULTI_S6A1_WAIT;
			return;
		case MD_MULTI_S9A1_PINB:
			if (!KnucklesCheckInput(twp, mwp, pwp) && KnucklesCheckBeInTheAir(pwp, twp))
			{
				twp->mode = MD_MULTI_S9A1_PINF;
			}
			break;
		case MD_MULTI_S9A1_PINF:
			if (!KnucklesCheckInput(twp, mwp, pwp) && (twp->flag & 3))
			{
				twp->mode = MD_MULTI_S9A1_PINB;
				twp->ang.x = mwp->ang_aim.x;
				twp->ang.z = mwp->ang_aim.z;
			}
			break;
		}
	}
	
	Knux_RunsActions_h.Original(twp, mwp, pwp);
}

void KnucklesTheEchidna_m(task* tp)
{
	auto twp = tp->twp;
	auto mwp = (motionwk2*)tp->mwp;
	auto pwp = (playerwk*)mwp->work.ptr;

	switch ((uint8_t)twp->mode)
	{
	case MD_MULTI_PARA:
		PGetGravity(twp, mwp, pwp);
		PGetSpeed(twp, mwp, pwp);
		PSetPosition(twp, mwp, pwp);
		PResetPosition(twp, mwp, pwp);
		break;
	case MD_MULTI_S6A1_WAIT:
		Mode_SDCylinderStd(twp, pwp);
		break;
	case MD_MULTI_S6A1_SLID:
		Mode_SDCylinderDown(twp, pwp);
		break;
	case MD_MULTI_S6A1_LROT:
		Mode_SDCylinderLeft(twp, pwp);
		break;
	case MD_MULTI_S6A1_RROT:
		Mode_SDCylinderRight(twp, pwp);
		break;
	case MD_MULTI_S9A1_PINB:
		PRotatedByGravityAsPinbal(twp, mwp, pwp);
		PSetPosition(twp, mwp, pwp);
		if ((twp->flag & 1) != 0)
		{
			mwp->spd.y = 0.0f;
		}
		PResetPosition(twp, mwp, pwp);
		break;
	case MD_MULTI_S9A1_PINF:
		PResetAngle(twp, mwp, pwp);
		PGetInertia(twp, mwp, pwp);
		PGetSpeed(twp, mwp, pwp);
		PSetPosition(twp, mwp, pwp);
		PResetPosition(twp, mwp, pwp);
		break;
	}

	KnucklesTheEchidna_h.Original(tp);
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
		KnucklesTheEchidna_h.Original(tp);
	}
}

// Knuckles has code to prevent gliding outside of the level area
// It does so by checking if Knuckles is above death planes
// We need to disable this behavior outside of Knuckles stages
Bool KnucklesCheckOutFDPolygon_r(taskwk* a1)
{
	if (multiplayer::IsActive() && CurrentCharacter != Characters_Knuckles)
	{
		return 0;
	}

	return KnucklesCheckOutFDPolygon_h.Original(a1);
}

void patch_knuckles_init()
{
	KnuEffectPutCharge0_h.Hook(KnuEffectPutCharge0_r);
	KnuEffectPutCharge1_h.Hook(KnuEffectPutCharge1_r);
	KnucklesGetNearEnemyTWP_h.Hook(KnucklesGetNearEnemyTWP_r);
	KnucklesChargeEffectExe_h.Hook(KnucklesChargeEffectExe_r);
	KnuEffectPutChargeComp_h.Hook(KnuEffectPutChargeComp_r);
	KnucklesJiggle_h.Hook(KnucklesJiggle_r);
	KnuxEyeTracker_h.Hook(KnuxEyeTracker_r);
	Knux_CheckInput_h.Hook(Knux_CheckInput_r);
	Knux_RunsActions_h.Hook(Knux_RunsActions_r);
	KnucklesTheEchidna_h.Hook(KnucklesTheEchidna_r);
	KnucklesCheckOutFDPolygon_h.Hook(KnucklesCheckOutFDPolygon_r);
}

RegisterPatch patch_knuckles(patch_knuckles_init);
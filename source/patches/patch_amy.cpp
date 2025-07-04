#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "gravity.h"
#include "patch_e_cart.h"
#include "result.h"
#include "patch_player.h"
#include "patch_o_sky_cyl_cmn.h"

FastUsercallHookPtr<Bool(*)(playerwk* pwp, motionwk2* mwp, taskwk* twp), rEAX, rECX, rEDI, rESI> Amy_CheckInput_h(0x487810);
FastFunctionHook<void, task*> AmyRose_h((intptr_t)Amy_Main);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> Amy_RunsActions_h(0x488880);
FastFunctionHook<void, task*> AmyJiggle_h(0x485C50);
FastFunctionHook<void, task*> AmySkirtShape_h(0x485F40);
FastFunctionHook<void, task*> AmyEyeTracker_h(0x486410);
FastFunctionHook<void, task*> AmyBirdExe_h(0x4C63F0);
FastFunctionHook<void, task*> LoadAmyBird_h(0x4C6790);
FastFunctionHook<void> AmySetRoboConChecker_h(0x486A40);

task* AmyBirdM[PLAYER_MAX] = { 0 };

void AmySetRoboConChecker_r()
{
	// Do not spawn Zero in non Amy layouts
	if (CurrentCharacter != Characters_Amy)
	{
		return;
	}
	AmySetRoboConChecker_h.Original();
}

void AmyBird_Del(task* obj)
{
	if (obj->twp)
	{
		char pnum = obj->twp->smode;
		auto task = AmyBirdM[pnum];

		if (task)
		{
			FreeTask(task);
		}

		AmyBirdM[pnum] = nullptr;
	}
}

void __cdecl AmyBird_Main_r(task* obj)
{
	if (!multiplayer::IsActive() || EV_MainThread_ptr)
	{
		return AmyBirdExe_h.Original(obj);
	}

	auto twp = obj->twp;
	auto pnum = twp->smode;
	auto Player = playertwp[pnum];

	if (!Player)
	{
		FreeTask(obj);
		return;
	}

	twp->ang.y = BAMS_SubWrap(twp->ang.y, 0x8000 - Player->ang.y, 1024);
	twp->ang.x += 16;
	twp->ang.z += 1024;

	NJS_VECTOR dest;

	dest.x = njCos(twp->ang.x) * 3.0f + Player->pos.x;
	dest.y = njSin(twp->ang.z) + 12.0f + Player->pos.y;
	dest.z = njSin(twp->ang.x) * 3.0f + Player->pos.z;

	float distance = sqrtf(powf(dest.x - twp->pos.x, 2) + powf(dest.y - twp->pos.y, 2) + powf(dest.z - twp->pos.z, 2));

	if (distance >= 200.0f) {
		twp->pos.x = dest.x;
		twp->pos.y = dest.y;
		twp->pos.z = dest.z;
	}
	else
	{
		twp->pos.x = (dest.x - twp->pos.x) * 0.25f + twp->pos.x;
		twp->pos.y = (dest.y - twp->pos.y) * 0.25f + twp->pos.y;
		twp->pos.z = (dest.z - twp->pos.z) * 0.25f + twp->pos.z;
	}

	twp->counter.f = FrameCounterUnpaused % Birdie_Anim.motion->nbFrame;
	obj->disp(obj);

	DrawShadow((EntityData1*)&twp, 0.40000001f);
	LoopTaskC(obj);
}

void Load_AmyBird_r(task* obj)
{
	obj->exec = AmyBird_Main_r;
	obj->disp = (TaskFuncPtr)0x4C62D0;
	obj->dest = AmyBird_Del;
	auto twp = obj->twp;
	auto pnum = twp->smode;
	twp->pos = playertwp[pnum]->pos;
	twp->pos.y += 12.0f;
	twp->ang.y = 0x8000 - twp->ang.y;
}

void __cdecl Init_AmyBird(task* tp)
{
	if (!multiplayer::IsActive() || EV_MainThread_ptr)
	{
		return LoadAmyBird_h.Original(tp);
	}

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (!AmyBirdM[i] && playertwp[i] && TASKWK_CHARID(playertwp[i]) == Characters_Amy)
		{
			AmyBirdM[i] = CreateElementalTask(LoadObj_Data1, 3, Load_AmyBird_r);
			AmyBirdM[i]->twp->smode = i;
		}
	}
}

// Not yet supported in multiplayer
static void __cdecl AmyEyeTracker_r(task* tp)
{
	if (multiplayer::IsActive() && !canselEvent)
	{
		FreeTask(tp);
		return;
	}

	AmyEyeTracker_h.Original(tp);
}

// Not yet supported in multiplayer
static void __cdecl AmyJiggle_r(task* tp)
{
	if (multiplayer::IsActive() && !canselEvent)
	{
		FreeTask(tp);
		return;
	}

	AmyJiggle_h.Original(tp);
}

// Not yet supported in multiplayer
static void __cdecl AmySkirtShape_r(task* tp)
{
	if (multiplayer::IsActive() && !canselEvent)
	{
		FreeTask(tp);
		return;
	}

	AmySkirtShape_h.Original(tp);
}

static Uint16 GetAmyVictoryAnim(unsigned __int8 a1)
{
	if (multiplayer::IsActive())
	{
		if (CurrentLevel < LevelIDs_Chaos0 || CurrentLevel > LevelIDs_E101R)
		{
			if (CheckDefeat(a1))
			{
				return 26;
			}
			else
			{
				return CurrentCharacter != Characters_Amy ? 42 : 32;
			}
		}
	}

	return GetStageNumber() >= LevelAndActIDs_Chaos0 ? 42 : 32;
}

Bool Amy_CheckInput_r(playerwk* pwp, motionwk2* mwp, taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		auto even = twp->ewp;
		auto pnum = TASKWK_PLAYERID(twp);

		if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
		{
			return Amy_CheckInput_h.Original(pwp, mwp, twp);
		}

		switch (twp->smode)
		{
		case PL_OP_PARABOLIC:
			if (CurrentLevel != LevelIDs_Casinopolis)
			{
				twp->mode = MD_MULTI_PARA;
				pwp->mj.reqaction = 18; //falling
				return TRUE;
			}
			break;
		case PL_OP_PLACEWITHKIME:
			twp->mode = 13;
			pwp->mj.reqaction = GetAmyVictoryAnim(pnum);
			twp->ang.z = 0;
			twp->ang.x = 0;
			PClearSpeed(mwp, pwp);
			twp->flag &= ~0x2500;
			CancelLookingAtP(pnum);
			return TRUE;
		case PL_OP_HOLDONPILLAR:
			if (SetCylinderNextAction(twp, mwp, pwp))
				return TRUE;
			break;
		}
	}
	
	return Amy_CheckInput_h.Original(pwp, mwp, twp);
}

void Amy_RunsActions_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (multiplayer::IsActive())
	{
		switch (twp->mode)
		{
		case MD_AMY_S3A2_CART: // Allow death incarts
			if ((twp->flag & Status_DoNextAction) && twp->smode == PL_OP_KILLED)
			{
				twp->mode = MD_AMY_KILL;
				pwp->mj.reqaction = 26;
			}
			break;
		case MD_MULTI_PARA:
			if (!AmyCheckInput(pwp, mwp, twp) && (twp->flag & 3) != 0)
			{
				if (PCheckBreak(twp) && pwp->spd.x > 0.0f)
				{
					twp->mode = 9;
				}
				if (!AmyCheckStop(twp, pwp))
				{
					twp->mode = 2;
				}

				twp->ang.x = mwp->ang_aim.x;
				twp->ang.z = mwp->ang_aim.z;
				pwp->mj.reqaction = 2;
			}
			return;
		case MD_MULTI_S6A1_WAIT:
			if (AmyCheckInput(pwp, mwp, twp) || AmyCheckJump(pwp, twp, mwp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_MD_MULTI_S6A1_WAITChanges(twp, pwp);
			return;
		case MD_MULTI_S6A1_SLID:

			if (AmyCheckInput(pwp, mwp, twp) || AmyCheckJump(pwp, twp, mwp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_MD_MULTI_S6A1_SLIDChanges(twp, pwp);

			return;
		case MD_MULTI_S6A1_LROT:
			if (AmyCheckInput(pwp, mwp, twp) || AmyCheckJump(pwp, twp, mwp))
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
			if (AmyCheckInput(pwp, mwp, twp) || AmyCheckJump(pwp, twp, mwp))
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
		}
	}

	Amy_RunsActions_h.Original(twp, mwp, pwp);
}

void AmyRose_m(task* tp)
{
	auto twp = tp->twp;
	auto mwp = (motionwk2*)tp->mwp;
	auto pwp = (playerwk*)mwp->work.ptr;

	switch (twp->mode)
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
	}

	AmyRose_h.Original(tp);
}

void __cdecl AmyRose_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		gravity::SaveGlobalGravity();
		gravity::SwapGlobalToUserGravity(TASKWK_PLAYERID(tp->twp));
		AmyRose_m(tp);
		gravity::RestoreGlobalGravity();
	}
	else
	{
		AmyRose_h.Original(tp);
	}
}

void patch_amy_init()
{
	AmyRose_h.Hook(AmyRose_r);
	Amy_CheckInput_h.Hook(Amy_CheckInput_r);
	Amy_RunsActions_h.Hook(Amy_RunsActions_r);
	AmyJiggle_h.Hook(AmyJiggle_r);
	AmySkirtShape_h.Hook(AmySkirtShape_r);
	AmyEyeTracker_h.Hook(AmyEyeTracker_r);
	LoadAmyBird_h.Hook(Init_AmyBird);
	AmyBirdExe_h.Hook(AmyBird_Main_r);
	AmySetRoboConChecker_h.Hook(AmySetRoboConChecker_r);
}

RegisterPatch patch_amy(patch_amy_init);
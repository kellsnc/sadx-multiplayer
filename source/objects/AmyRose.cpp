#include "pch.h"
#include "gravity.h"
#include "e_cart.h"
#include "result.h"
#include "ObjCylinderCmn.h"

UsercallFunc(Bool, Amy_CheckInput_t, (playerwk* a1, motionwk2* a2, taskwk* a3), (a1, a2, a3), 0x487810, rEAX, rECX, rEDI, rESI);
TaskHook AmyRose_t((intptr_t)Amy_Main);
static FunctionHook<void, taskwk*, motionwk2*, playerwk*> Amy_RunsActions_t((intptr_t)0x488880);
TaskHook AmyJiggle_t((intptr_t)0x485C50);
TaskHook AmySkirtShape_t(0x485F40);
TaskHook AmyEyeTracker_t(0x486410);
TaskHook AmyBirdExe_t(0x4C63F0);
TaskHook LoadAmyBird_t(0x4C6790);
FunctionHook<void> AmySetRoboConChecker_t(0x486A40);

task* AmyBirdM[PLAYER_MAX] = { 0 };

void AmySetRoboConChecker_r()
{
	// Do not spawn Zero in non Amy layouts
	if (CurrentCharacter != Characters_Amy)
	{
		return;
	}
	AmySetRoboConChecker_t.Original();
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
		return AmyBirdExe_t.Original(obj);
	}

	auto data = obj->twp;
	auto pnum = data->smode;
	auto Player = playertwp[pnum];

	if (!Player)
	{
		FreeTask(obj);
		return;
	}

	data->ang.y = BAMS_SubWrap(data->ang.y, 0x8000 - Player->ang.y, 1024);
	data->ang.x += 16;
	data->ang.z += 1024;

	NJS_VECTOR dest;

	dest.x = njCos(data->ang.x) * 3.0f + Player->pos.x;
	dest.y = njSin(data->ang.z) + 12.0f + Player->pos.y;
	dest.z = njSin(data->ang.x) * 3.0f + Player->pos.z;

	float distance = sqrtf(powf(dest.x - data->pos.x, 2) + powf(dest.y - data->pos.y, 2) + powf(dest.z - data->pos.z, 2));

	if (distance >= 200.0f) {
		data->pos.x = dest.x;
		data->pos.y = dest.y;
		data->pos.z = dest.z;
	}
	else
	{
		data->pos.x = (dest.x - data->pos.x) * 0.25f + data->pos.x;
		data->pos.y = (dest.y - data->pos.y) * 0.25f + data->pos.y;
		data->pos.z = (dest.z - data->pos.z) * 0.25f + data->pos.z;
	}

	data->counter.f = FrameCounterUnpaused % Birdie_Anim.motion->nbFrame;
	obj->disp(obj);

	DrawShadow((EntityData1*)&data, 0.40000001f);
	LoopTaskC(obj);
}

void Load_AmyBird_r(task* obj)
{
	obj->exec = AmyBird_Main_r;
	obj->disp = (TaskFuncPtr)0x4C62D0;
	obj->dest = AmyBird_Del;
	auto data = obj->twp;
	auto pnum = data->smode;
	data->pos = playertwp[pnum]->pos;
	data->pos.y += 12.0f;
	data->ang.y = 0x8000 - data->ang.y;
}

void __cdecl Init_AmyBird(task* tp)
{
	if (!multiplayer::IsActive() || EV_MainThread_ptr)
	{
		return LoadAmyBird_t.Original(tp);
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

static void __cdecl AmyEyeTracker_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	AmyEyeTracker_t.Original(tp);
}

static void __cdecl AmyJiggle_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	AmyJiggle_t.Original(tp);
}

static void __cdecl AmySkirtShape_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	AmySkirtShape_t.Original(tp);
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
			return Amy_CheckInput_t.Original(pwp, mwp, twp);
		}

		switch (twp->smode)
		{
		case PL_OP_PARABOLIC:
			if (CurrentLevel != LevelIDs_Casinopolis)
			{
				twp->mode = SDCannonMode;
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
	
	return Amy_CheckInput_t.Original(pwp, mwp, twp);
}

void Amy_RunsActions_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (multiplayer::IsActive())
	{
		switch (twp->mode)
		{
		case 48: //cart
			KillPlayerInKart(twp, pwp, 47, 26);
			break;
		case SDCannonMode:
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
		case SDCylStd:
			if (AmyCheckInput(pwp, mwp, twp) || AmyCheckJump(pwp, twp, mwp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_SDCylStdChanges(twp, pwp);
			return;
		case SDCylDown:

			if (AmyCheckInput(pwp, mwp, twp) || AmyCheckJump(pwp, twp, mwp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_SDCylDownChanges(twp, pwp);

			return;
		case SDCylLeft:
			if (AmyCheckInput(pwp, mwp, twp) || AmyCheckJump(pwp, twp, mwp))
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
			if (AmyCheckInput(pwp, mwp, twp) || AmyCheckJump(pwp, twp, mwp))
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

	Amy_RunsActions_t.Original(twp, mwp, pwp);
}

void AmyRose_m(task* tp)
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

	AmyRose_t.Original(tp);
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
		AmyRose_t.Original(tp);
	}
}

void Init_AmyPatches()
{
	AmyRose_t.Hook(AmyRose_r);
	Amy_CheckInput_t.Hook(Amy_CheckInput_r);
	Amy_RunsActions_t.Hook(Amy_RunsActions_r);
	AmyJiggle_t.Hook(AmyJiggle_r);
	AmySkirtShape_t.Hook(AmySkirtShape_r);
	AmyEyeTracker_t.Hook(AmyEyeTracker_r);
	LoadAmyBird_t.Hook(Init_AmyBird);
	AmyBirdExe_t.Hook(AmyBird_Main_r);
	AmySetRoboConChecker_t.Hook(AmySetRoboConChecker_r);
}
#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "gravity.h"
#include "camera.h"
#include "fishing.h"
#include "e_cart.h"
#include "result.h"
#include "ObjCylinderCmn.h"

FastFunctionHook<void, task*> BigTheCat_t((intptr_t)Big_Main);
FastUsercallHookPtr<void(*)(playerwk* pwp, taskwk* twp, motionwk2* mwp), noret, rEAX, stack4, stack4> BigChkMode_t(0x48E640);
FastUsercallHookPtr<Bool(*)(playerwk* pwp, taskwk* twp, motionwk2* mwp), rEAX, rEAX, rEDI, stack4> Big_CheckInput_t(0x48D400);
FastFunctionHook<void, task*> Big_Jiggle_t((intptr_t)0x48C720);
FastFunctionHook<void, task*> BigEyeTracker_t(0x48E2E0);

void __cdecl BigEyeTracker_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	BigEyeTracker_t.Original(tp);
}

void __cdecl Big_Jiggle_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	Big_Jiggle_t.Original(tp);
}

void __cdecl bigActMissSet_r(taskwk* twp, motionwk2* mwp, playerwk* pwp);
Trampoline bigActMissSet_t(0x48CD50, 0x48CD55, bigActMissSet_r);
void __cdecl bigActMissSet_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(twp);

		CameraReleaseCollisionCamera_m(pnum);

		auto cam_pos = GetCameraPosition(pnum);
		if (cam_pos)
		{
			cam_pos->y = twp->pos.y + 10.0f;
			cam_pos->x = (cam_pos->x - twp->pos.x) * 0.01f + twp->pos.x;
			cam_pos->z = (cam_pos->z - twp->pos.z) * 0.01f + twp->pos.z;
		}

		pwp->mj.reqaction = 63;
		twp->mode = MD_BIG_MISS;

		dsPlay_oneshot(848, 0, 0, 0);
		dsPlay_oneshot(1322, 0, 0, 0);
	}
	else
	{
		TARGET_STATIC(bigActMissSet)(twp, mwp, pwp);
	}
}

void __cdecl sub_48CDE0_r();
Trampoline sub_48CDE0_t(0x48CDE0, 0x48CDE5, sub_48CDE0_r);
void __cdecl sub_48CDE0_r()
{
	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(gpCharTwp);

		if (GetLevelType() == 1)
		{
			CameraSetEventCamera_m(pnum, CAMMD_KNUCKLES2, CAMADJ_TIME);
		}
		else
		{
			CameraSetEventCameraFunc_m(pnum, CameraFishing, CAMADJ_SLOW, CDM_LOOKAT);
		}
	}
	else
	{
		TARGET_STATIC(sub_48CDE0)();
	}
}

void __cdecl sub_48CE10_r();
Trampoline sub_48CE10_t(0x48CE10, 0x48CE17, sub_48CE10_r);
void __cdecl sub_48CE10_r()
{
	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(gpCharTwp);
		auto etc = GetBigEtc(pnum);

		if (etc->Big_Fish_Flag & LUREFLAG_FISH)
		{
			CameraReleaseCollisionCamera_m(pnum);
		}
		else
		{
			CameraReleaseEventCamera_m(pnum);
		}
	}
	else
	{
		TARGET_STATIC(sub_48CE10)();
	}
}

Bool Big_CheckInput_r(playerwk* pwp, taskwk* twp, motionwk2* mwp)
{
	if (multiplayer::IsActive())
	{
		auto even = twp->ewp;
		auto pnum = TASKWK_PLAYERID(twp);

		if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
		{
			return Big_CheckInput_t.Original(pwp, twp, mwp);
		}

		switch (twp->smode)
		{
		case PL_OP_PARABOLIC:
			if (CurrentLevel == LevelIDs_Casinopolis)
				return FALSE;
			break;
		case PL_OP_PLACEWITHKIME:
			if (CheckDefeat(pnum))
			{
				twp->mode = 14;
				pwp->mj.reqaction = 63;
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
	
	return Big_CheckInput_t.Original(pwp, twp, mwp);
}

void __cdecl BigChkMode_r(playerwk* pwp, taskwk* twp, motionwk2* mwp)
{
	if (multiplayer::IsActive())
	{
		switch (twp->mode)
		{
		case 55: //cart
			KillPlayerInKart(twp, pwp, 58, 99);
			break;
		case SDCylStd:
			if (BigCheckInput(pwp, twp, mwp) || BigCheckJump(pwp, twp))
			{
				pwp->htp = 0;
				break;
			}

			Mode_SDCylStdChanges(twp, pwp);
			return;
		case SDCylDown:

			if (BigCheckInput(pwp, twp, mwp) || BigCheckJump(pwp, twp))
			{
				pwp->htp = 0;
				break;
			}

			Mode_SDCylDownChanges(twp, pwp);

			return;
		case SDCylLeft:
			if (BigCheckInput(pwp, twp, mwp) || BigCheckJump(pwp, twp))
			{
				pwp->htp = 0;
				break;
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
			if (BigCheckInput(pwp, twp, mwp) || BigCheckJump(pwp, twp))
			{
				pwp->htp = 0;
				break;
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

	BigChkMode_t.Original(pwp, twp, mwp);
}

void BigTheCat_m(task* tp)
{
	auto twp = tp->twp;
	auto mwp = (motionwk2*)tp->mwp;
	auto pwp = (playerwk*)mwp->work.ptr;

	auto pnum = TASKWK_PLAYERID(tp->twp);

	gravity::SaveGlobalGravity();
	gravity::SwapGlobalToUserGravity(pnum);

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

	auto etc = GetBigEtc(pnum);
	if (pnum >= 1 && etc)
	{
		auto backup_ptr = Big_Lure_Ptr;
		auto backup_flag = Big_Fish_Flag;
		auto backup_fish = Big_Fish_Ptr;
		Big_Lure_Ptr = etc->Big_Lure_Ptr;
		Big_Fish_Flag = etc->Big_Fish_Flag;
		Big_Fish_Ptr = etc->Big_Fish_Ptr;
		BigTheCat_t.Original(tp);
		etc->Big_Lure_Ptr = Big_Lure_Ptr;
		etc->Big_Fish_Flag = Big_Fish_Flag;
		Big_Lure_Ptr = backup_ptr;
		Big_Fish_Flag = backup_flag;
		Big_Fish_Ptr = backup_fish;
	}
	else
	{
		BigTheCat_t.Original(tp);
	}

	gravity::RestoreGlobalGravity();
}

void __cdecl BigTheCat_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		BigTheCat_m(tp);
	}
	else
	{
		BigTheCat_t.Original(tp);
	}
}

void Init_BigPatches()
{
	BigTheCat_t.Hook(BigTheCat_r);
	Big_CheckInput_t.Hook(Big_CheckInput_r);
	BigChkMode_t.Hook(BigChkMode_r);
	Big_Jiggle_t.Hook(Big_Jiggle_r);
	BigEyeTracker_t.Hook(BigEyeTracker_r);
}
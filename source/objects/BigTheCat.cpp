#include "pch.h"
#include "multiplayer.h"
#include "camera.h"
#include "fishing.h"
#include "ObjCylinderCmn.h"
#include "e_cart.h"


TaskHook BigTheCat_t((intptr_t)Big_Main);

Trampoline* BigChkMode_t = nullptr; //doesn't want to work with FuncHook for some weird reason
UsercallFunc(Bool, Big_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x48D400, rEAX, rEAX, rEDI, stack4);
TaskHook Big_Jiggle_t((intptr_t)0x48C720);
TaskHook BigEyeTracker_t(0x48E2E0);

static void __cdecl BigEyeTracker_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	BigEyeTracker_t.Original(tp);
}

static void __cdecl Big_Jiggle_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	Big_Jiggle_t.Original(tp);
}

static void __cdecl bigActMissSet_r(taskwk* twp, motionwk2* mwp, playerwk* pwp);
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

static void __cdecl sub_48CDE0_r();
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

static void __cdecl sub_48CE10_r();
Trampoline sub_48CE10_t(0x48CE10, 0x48CE17, sub_48CE10_r);
static void __cdecl sub_48CE10_r()
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

Bool Big_CheckInput_r(playerwk* co2, taskwk* data, motionwk2* data2)
{
	auto even = data->ewp;

	if (even->move.mode || even->path.list || ((data->flag & Status_DoNextAction) == 0))
	{
		return Big_CheckInput_t.Original(co2, data, data2);
	}


	switch (data->smode)
	{

	case 32:

		if (SetCylinderNextAction(data, data2, co2))
			return 1;

		break;

	}

	return Big_CheckInput_t.Original(co2, data, data2);
}

#define NAKED __declspec(naked)
#pragma region CHK MODE

BOOL BigChkMode_o(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	auto target = BigChkMode_t->Target();
	signed int result;
	__asm
	{
		push[mwp]
		push[twp]
		mov eax, [pwp]
		call target
		add esp, 8
		mov result, eax
	}
	return result;
}

static void __cdecl BigChkMode_r(playerwk* co2, taskwk* data1, motionwk2* data2)
{
	switch (data1->mode)
	{
	case 55: //cart
		KillPlayerInKart(data1, co2, 58, 99);
		break;
	case SDCylStd:
		if (BigCheckInput(co2, data1, data2) || BigCheckJump(co2, data1))
		{
			co2->htp = 0;
			break;
		}

		Mode_SDCylStdChanges(data1, co2);
		return;
	case SDCylDown:

		if (BigCheckInput(co2, data1, data2) || BigCheckJump(co2, data1))
		{
			co2->htp = 0;
			break;
		}


		Mode_SDCylDownChanges(data1, co2);

		return;
	case SDCylLeft:
		if (BigCheckInput(co2, data1, data2) || BigCheckJump(co2, data1))
		{
			co2->htp = 0;
			break;
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
		if (BigCheckInput(co2, data1, data2) || BigCheckJump(co2, data1))
		{
			co2->htp = 0;
			break;
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


	BigChkMode_o(data1, data2, co2);
}

static void NAKED BigChkMode_jmp()
{
	__asm
	{
		push[esp + 08h] // mwp
		push[esp + 08h] // twp
		push eax // pwp
		call BigChkMode_r
		pop eax
		add esp, 8
		retn
	}
}

#pragma endregion

static void __cdecl BigTheCat_r(task* tp)
{
	auto data = tp->twp;
	auto data2 = (motionwk2*)tp->mwp;
	auto co2 = (playerwk*)tp->mwp->work.l;

	switch (data->mode)
	{
	case SDCannonMode:
		CannonModePhysics(data, data2, co2);
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

	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(tp->twp);
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
	BigChkMode_t = new Trampoline(0x48E640, 0x48E645, BigChkMode_jmp);
	Big_Jiggle_t.Hook(Big_Jiggle_r);
	BigEyeTracker_t.Hook(BigEyeTracker_r);
}
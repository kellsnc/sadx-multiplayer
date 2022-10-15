#include "pch.h"
#include "multiplayer.h"
#include "camera.h"
#include "fishing.h"

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


TaskHook BigTheCat_t((intptr_t)Big_Main);

static void __cdecl BigTheCat_r(task* tp)
{
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

}
#include "pch.h"
#include "multiplayer.h"
#include "hud_fishing.h";
#include "fishing.h"

enum : __int32
{
	MODE_LURE_INIT = 0x0,
	MODE_LURE_SETUP = 0x1,
	MODE_LURE_WAIT = 0x2,
	MODE_LURE_NORMAL = 0x3,
	MODE_LURE_FISHING_INIT = 0x4,
	MODE_LURE_FISHING = 0x5,
	MODE_LURE_HIT = 0x6,
	MODE_LURE_RETURN = 0x7,
	MODE_LURE_RETURN_GET = 0x8,
	MODE_LURE_END = 0x9,
};

enum : __int32
{
	LUREFLAG_800 = 0x800,
	LUREFLAG_8000 = 0x8000,
};

MAKEVARMULTI(BIGETC, bigetc, 0x3C524E8);

Trampoline* dispFishingLure_t       = nullptr;
Trampoline* dispFishingLureSwitch_t = nullptr;
Trampoline* fishingLureCtrl_t       = nullptr;
Trampoline* SetFishingLureTask_t    = nullptr;
Trampoline* fishingCursorCtrl_t     = nullptr;
Trampoline* SetFishingCursorTask_t  = nullptr;

BIGETC* GetBigEtc(int pnum)
{
	return bigetc_m[pnum];
}

#pragma region fishingLureCtrl
static void __cdecl dispFishingLure_m(task* tp)
{
	auto twp = tp->twp;
	auto pnum = twp->btimer;
	auto ptwp = playertwp[pnum];

	if (!MissedFrames && ptwp && TASKWK_CHARID(ptwp) == Characters_Big && (ulGlobalMode != MD_GAME_FADEOUT_CHANGE2 || !GetMiClearStatus()))
	{
		auto etc = GetBigEtc(pnum);

		njSetTexture(&FISHING_TEXLIST);
		njControl3D_Backup();
		njControl3D_Remove(NJD_CONTROL_3D_CONSTANT_MATERIAL);
		njControl3D_Add(NJD_CONTROL_3D_OFFSET_MATERIAL);
		njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
		ResetMaterial();

		if (!(etc->Big_Fish_Flag & LUREFLAG_800))
		{
			njPushMatrixEx();
			njTranslateEx(&twp->pos);
			njRotateY_(-twp->ang.y);
			njRotateX_(twp->ang.x);
			njRotateZ_(twp->ang.z);
			if (twp->mode < 3.0f)
				njTranslate(0, 0.6f, -0.3f, 0.0f);
			njScale(0, 0.3f, 0.3f, 0.3f);
			dsDrawObject(lure_kind[getLureKind()]);
			njPopMatrixEx();
		}

		if (twp->mode >= 3)
		{
			String_Dsp((String*)tp->awp->work.ul[2]);
			DrawFishingMeter(pnum, etc->reel_length_d, etc->reel_tension, twp->mode == 3 || twp->mode > 6 ? 0 : etc->reel_angle);
		}

		njControl3D_Restore();
	}
}

static void __cdecl dispFishingLure_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		dispFishingLure_m(tp);
	}
	else
	{
		TARGET_DYNAMIC(dispFishingLure)(tp);
	}
}

static void __cdecl dispFishingLureSwitch_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		dispFishingLure_m(tp);
	}
	else
	{
		TARGET_DYNAMIC(dispFishingLureSwitch)(tp);
	}
}
#pragma endregion

#pragma region fishingLureCtrl
static void CalcRodPos_m(taskwk* ptwp, playerwk* ppwp, NJS_POINT3* rod_pos_p)
{
	NJS_VECTOR v = { 0.0f, ppwp->p.center_height, 0.0f };
	njPushMatrix(_nj_unit_matrix_);
	njRotateZ_(ptwp->ang.z);
	njRotateX_(ptwp->ang.x);
	njRotateY_(0x8000ui16 - LOWORD(ptwp->ang.y));
	njCalcVector(0, &v, rod_pos_p);

	float dist = (ppwp->equipment & Upgrades_PowerRod) != 0 ? 17.0f : 16.0f;
	
	v.x = ppwp->user0_pos.x - dist * ppwp->user0_vec.x;
	v.y = ppwp->user0_pos.y - dist * ppwp->user0_vec.y;
	v.z = ppwp->user0_pos.z - dist * ppwp->user0_vec.z;

	if (ppwp->equipment & Upgrades_PowerRod)
	{
		v.y -= 1.8f;
		v.z += 0.6f;
	}
	else
	{
		v.y -= 0.6f;
	}

	njCalcVector(0, &v, &v);

	rod_pos_p->x = ptwp->pos.x + rod_pos_p->x + v.x;
	rod_pos_p->y = ptwp->pos.y + rod_pos_p->y + v.y;
	rod_pos_p->z = ptwp->pos.z + rod_pos_p->z + v.z;
	
	njPopMatrixEx();
}

static void setLurePosWait_m(task* tp, BIGETC* etc, taskwk* ptwp, NJS_POINT3* rod_pos_p)
{
	auto twp = tp->twp;
	auto mwp = tp->mwp;

	twp->pos = *rod_pos_p;
	twp->ang.y = tp->mwp->ang_aim.y;
	mwp->ang_aim.y = ptwp->ang.y;

	if (twp->ang.x >= 0x5800 && twp->ang.x <= 0xA800)
	{
		if (twp->ang.x < 0x6000)
		{
			mwp->ang_aim.x = 0xA000;
		}
	}
	else
	{
		twp->ang.x = 0x5800;
	}

	if (twp->ang.x <= 0xA000)
	{
		if (mwp->ang_aim.x >= 0x8000)
		{
			twp->ang.x += 0x200;
		}
		else
		{
			twp->ang.x -= 0x200;
		}
	}
	else
	{
		mwp->ang_aim.x = 0x6000;
		twp->ang.x -= 0x200;
	}

	if (twp->ang.z >= 0x2800 && twp->ang.z <= 0x5800)
	{
		if (twp->ang.z < 0x3000)
		{
			mwp->ang_aim.z = 0x5000;
		}
		if (twp->ang.z <= 0x5000)
		{
			if (mwp->ang_aim.z >= 0x4000)
			{
				twp->ang.z += 0x80;
			}
			else
			{
				twp->ang.z -= 0x80;
			}
		}
		else
		{
			mwp->ang_aim.z = 0x3000;
			twp->ang.z -= 0x80;
		}
	}
	else
	{
		mwp->ang_aim.z = 0x3000;
		twp->ang.z = 0x5780;
	}
}

static void setLureSetup_m(task* tp, BIGETC* etc)
{
	tp->twp->mode = MODE_LURE_SETUP;
	auto awp = tp->awp;
	awp->work.uw[2] = 0i16;
	awp->work.uw[3] = 0i16;
	etc->reel_angle = 0;
	etc->reel_tension = 0.0f;
	etc->reel_tension_aim = 0.0f;
	etc->reel_length_d = 0.0f;
	etc->reel_length = 0.0f;
	if (etc->Big_Fish_Flag & 1)
		RestoreLastSong();
	dsStop_num(845);
}

static void fishingLureCtrl_m(task* tp)
{
	auto twp = tp->twp;
	auto pnum = tp->twp->btimer;
	auto ptwp = playertwp[pnum];

	if (!pnum)
	{
		FreeTask(tp);
		return;
	}

	auto ppwp = playerpwp[pnum];
	auto etc = GetBigEtc(pnum);

	NJS_POINT3 rod_pos;
	CalcRodPos_m(ptwp, ppwp, &rod_pos);

	task* cursor_tp = (task*)ppwp->free.ptr[1];
	NJS_POINT3* pt = (NJS_POINT3*)tp->awp->work.ul[3];

	switch (twp->mode)
	{
	case MODE_LURE_INIT:
		setLureSetup_m(tp, etc);
		tp->dest = exitFishingLure;
		tp->disp = dispFishingLure;
		break;
	case MODE_LURE_SETUP:
		setLurePosWait_m(tp, etc, ptwp, &rod_pos);

		if (etc->Big_Fish_Flag & LUREFLAG_8000 && cursor_tp)
		{
			etc->Big_Fish_Flag &= ~LUREFLAG_8000;
			twp->mode = MODE_LURE_WAIT;
			twp->timer.l = 10;

			pt->x = cursor_tp->twp->pos.x;
			pt->y = cursor_tp->awp->work.f[1];
			pt->z = cursor_tp->twp->pos.z;
			FreeTask(cursor_tp);
			return;
		}
		break;
	case MODE_LURE_WAIT:
		setLurePosWait_m(tp, etc, ptwp, &rod_pos);

		if (--twp->timer.l <= 0)
		{
			twp->mode = MODE_LURE_NORMAL;
			dsPlay_oneshot(842, 0, 0, 0);
			dsPlay_oneshot(1318, 0, 0, 0);
		}

		break;
	}

	tp->disp(tp);
}

static void __cdecl fishingLureCtrl_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		auto pnum = tp->twp->btimer;

		if (pnum)
		{
			fishingLureCtrl_m(tp);
		}
		else
		{
			TARGET_DYNAMIC(fishingLureCtrl)(tp);
		}
	}
	else
	{
		TARGET_DYNAMIC(fishingLureCtrl)(tp);
	}
}

static task* __cdecl SetFishingLureTask_r(task* tp)
{
	auto lure_tp = TARGET_DYNAMIC(SetFishingLureTask)(tp);
	auto pnum = lure_tp->twp->btimer = TASKWK_PLAYERID(tp->twp);
	bigetc_m[pnum]->Big_Lure_Ptr = lure_tp;
	bigetc_m[pnum]->Big_Fish_Flag = 0;
	return lure_tp;
}
#pragma endregion

#pragma region fishingCursorCtrl
static auto MoveFishingCursor = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x46F850, rEAX);

static void fishingCursorCtrl_m(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 1)
	{
		auto pnum = TASKWK_PLAYERID(twp);
		auto ptwp = playertwp[pnum];

		if (!ptwp || (ptwp->mode < 32 || ptwp->mode > 37) && (ptwp->mode < 40 || ptwp->mode > 45))
		{
			FreeTask(tp);
			return;
		}

		auto etc = GetBigEtc(pnum);
		etc->water_level = tp->awp->work.f[1];

		twp->ang.y += 0x200;

		//setCameraFishingTgt_m(tp);
		MoveFishingCursor(tp);

		ptwp->ang.y = 0x4000 - NJM_RAD_ANG(atan2f(twp->pos.x - ptwp->pos.x, twp->pos.z - ptwp->pos.z));
		playermwp[pnum]->ang_aim.y = ptwp->ang.y;
	}

	tp->disp(tp);
}

static void __cdecl fishingCursorCtrl_r(task* tp)
{
	if (multiplayer::IsEnabled() && tp->twp->mode != 0)
	{
		fishingCursorCtrl_m(tp);
	}
	else
	{
		TARGET_DYNAMIC(fishingCursorCtrl)(tp);
	}
}

static task* __cdecl SetFishingCursorTask_r(task* tp)
{
	auto cursor_tp = TARGET_DYNAMIC(SetFishingCursorTask)(tp);
	TASKWK_PLAYERID(cursor_tp->twp) = TASKWK_PLAYERID(tp->twp);
	return cursor_tp;
}
#pragma endregion

static void BigStateInit_r()
{
	for (auto& i : bigetc_m)
	{
		i->Big_Sakana_Weight = 0;
		i->Big_Sakana_Weight_High = 0i16;
		i->Big_Sakana_Kind_High = 0;
		i->Big_Sakana_Weight_Limit = BigChkHeavyWeight();
		i->big_item_pos.z = 0.0f;
		i->big_item_pos.y = 0.0f;
		i->big_item_pos.x = 0.0f;
	}
}

void InitFishing()
{
	dispFishingLure_t       = new Trampoline(0x470580, 0x470588, dispFishingLure_r);
	dispFishingLureSwitch_t = new Trampoline(0x4703F0, 0x4703F8, dispFishingLureSwitch_r);
	fishingLureCtrl_t       = new Trampoline(0x471580, 0x471589, fishingLureCtrl_r);
	SetFishingLureTask_t    = new Trampoline(0x471ED0, 0x471ED6, SetFishingLureTask_r);
	fishingCursorCtrl_t     = new Trampoline(0x46FA10, 0x46FA18, fishingCursorCtrl_r);
	SetFishingCursorTask_t  = new Trampoline(0x470330, 0x470336, SetFishingCursorTask_r);

	WriteJump((void*)0x470120, BigStateInit_r);

	WriteData<2>((void*)0x490C52, 0x90ui8); // force fishingLureCtrl to load for every player
	WriteData<6>((void*)0x470425, 0x90ui8); // force things to display even if player 0 is not Big
}
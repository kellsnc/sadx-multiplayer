#include "pch.h"
#include "multiplayer.h"
#include "hud_fishing.h";
#include "fishing.h"

// Note: pointers hacks are temporary until everything is rewritten

MAKEVARMULTI(BIGETC, bigetc, 0x3C524E8);

Trampoline* dispFishingLure_t      = nullptr;
Trampoline* fishingLureCtrl_t      = nullptr;
Trampoline* SetFishingLureTask_t   = nullptr;
Trampoline* fishingCursorCtrl_t    = nullptr;
Trampoline* SetFishingCursorTask_t = nullptr;

BIGETC* GetBigEtc(int pnum)
{
	return bigetc_m[pnum];
}

static void __cdecl dispFishingLure_r(task* tp)
{
	if (multiplayer::IsEnabled())
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

			if ((etc->Big_Fish_Flag & 0x800) == 0)
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
	else
	{
		TARGET_DYNAMIC(dispFishingLure)(tp);
	}
}

static void __cdecl fishingLureCtrl_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		auto pnum = tp->twp->btimer;
		TARGET_DYNAMIC(fishingLureCtrl)(tp);
		tp->disp(tp);
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
	dispFishingLure_t      = new Trampoline(0x470580, 0x470588, dispFishingLure_r);
	fishingLureCtrl_t      = new Trampoline(0x471580, 0x471589, fishingLureCtrl_r);
	SetFishingLureTask_t   = new Trampoline(0x471ED0, 0x471ED6, SetFishingLureTask_r);
	fishingCursorCtrl_t    = new Trampoline(0x46FA10, 0x46FA18, fishingCursorCtrl_r);
	SetFishingCursorTask_t = new Trampoline(0x470330, 0x470336, SetFishingCursorTask_r);

	WriteJump((void*)0x470120, BigStateInit_r);

	WriteData<2>((void*)0x490C52, 0x90ui8); // force fishingLureCtrl to load for every player
	WriteData<6>((void*)0x470425, 0x90ui8); // force things to display even if player 0 is not Big
}
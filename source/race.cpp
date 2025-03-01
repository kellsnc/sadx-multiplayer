#include "pch.h"
#include "splitscreen.h"
#include "utils.h"
#include "result.h"
#include "camera.h"
#include "race.h"

/*

Multiplayer Twinkle Circuit manager

*/

FastFunctionHook<void, task*> TwinkleCircuitZoneTask_h(0x4DBCF0);
FastFunctionHook<void, task*> Rd_MiniCart_h(0x4DAA80);

// RACE MANAGER:

struct RaceWkM // multiplayer version of RaceWk in symbols
{
	RaceWk work[PLAYER_MAX];
	Sint32 winner;
};

enum RACEMD // guessed
{
	RACEMD_WAIT0,
	RACEMD_WAIT1,
	RACEMD_INTRO,
	RACEMD_GAME,
	RACEMD_CKPT,
	RACEMD_GOAL,
	RACEMD_END,
};

bool cartGoalFlagM[PLAYER_MAX];

static void DrawTimer(int time, float x, float y, float s)
{
	auto t = CartStateCentiSecToDispTime(time);
	OBJ_MINI_CART_SPRITE_Lap.tanim = cartsprite_score;
	OBJ_MINI_CART_SPRITE_Lap.p.x = x;
	OBJ_MINI_CART_SPRITE_Lap.p.y = y;
	OBJ_MINI_CART_SPRITE_Lap.sx = OBJ_MINI_CART_SPRITE_Lap.sy = s;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_Lap, t.min / 10, 22048.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, QueuedModelFlagsB_SomeTextureThing);
	OBJ_MINI_CART_SPRITE_Lap.p.x += 16 * s;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_Lap, t.min % 10, 22048.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, QueuedModelFlagsB_SomeTextureThing);
	OBJ_MINI_CART_SPRITE_Lap.p.x += 16 * s;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_Lap, 10, 22048.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, QueuedModelFlagsB_SomeTextureThing);
	OBJ_MINI_CART_SPRITE_Lap.p.x += 16 * s;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_Lap, t.sec / 10, 22048.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, QueuedModelFlagsB_SomeTextureThing);
	OBJ_MINI_CART_SPRITE_Lap.p.x += 16 * s;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_Lap, t.sec % 10, 22048.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, QueuedModelFlagsB_SomeTextureThing);
	OBJ_MINI_CART_SPRITE_Lap.p.x += 16 * s;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_Lap, 10, 22048.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, QueuedModelFlagsB_SomeTextureThing);
	OBJ_MINI_CART_SPRITE_Lap.p.x += 16 * s;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_Lap, t.sec100 / 10, 22048.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, QueuedModelFlagsB_SomeTextureThing);
	OBJ_MINI_CART_SPRITE_Lap.p.x += 16 * s;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_Lap, t.sec100 % 10, 22048.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, QueuedModelFlagsB_SomeTextureThing);
}

static void dispRaceSingle(RaceWk* rwp, int num)
{
	auto ratio = SplitScreen::GetScreenRatio(num);
	const float scaleY = VerticalStretch * ratio->h;
	const float scaleX = HorizontalStretch * ratio->w;
	const float scale = min(scaleY, scaleX);
	const float screenX = HorizontalResolution * ratio->x;
	const float screenY = VerticalResolution * ratio->y;

	// TIME:
	NJS_SPRITE sp = SNUM_CO_SPRITE;
	sp.sx = sp.sy = scale;
	sp.p.x = 16.0f * scaleX + screenX;
	sp.p.y = 32.0f * scaleY + screenY;
	njDrawSprite2D_Queue(&sp, 0, 22045.0f, NJD_SPRITE_ALPHA, QueuedModelFlagsB_SomeTextureThing);

	// MAIN TIMER:
	DrawTimer(rwp->totalIntrpt, 16 * scaleX + 48 * scale + screenX, 33.0f * scaleY + screenY, scale);

	// LAP COUNT:
	OBJ_MINI_CART_SPRITE_LAPS_A.p.x = 640.0f * scaleX - 16.0f * scale - 96.0f * scale + screenX;
	OBJ_MINI_CART_SPRITE_LAPS_A.p.y = 96.0f * scaleY + screenY;
	OBJ_MINI_CART_SPRITE_LAPS_A.sx = OBJ_MINI_CART_SPRITE_LAPS_A.sy = scale;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_LAPS_A, 0, 22045.998f, NJD_SPRITE_ALPHA, QueuedModelFlagsB_SomeTextureThing);

	OBJ_MINI_CART_SPRITE_LAPS_B.p.x = 640.0f * scaleX - 16.0f * scale - 96.0f * scale + 39.0f * scale + HorizontalResolution * ratio->x;
	OBJ_MINI_CART_SPRITE_LAPS_B.p.y = 99.0f * scaleY + VerticalResolution * ratio->y;
	OBJ_MINI_CART_SPRITE_LAPS_B.sx = OBJ_MINI_CART_SPRITE_LAPS_B.sy = scale;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_LAPS_B, rwp->displayLap, 22046.0f, NJD_SPRITE_ALPHA, QueuedModelFlagsB_SomeTextureThing);

	// HISTORY:
	OBJ_MINI_CART_SPRITE_TimeBest.p.x = 640.0f * scaleX - 240.5f * scale + 48.0f * scale + screenX;
	OBJ_MINI_CART_SPRITE_TimeBest.p.y = 32.0f * scaleY + screenY;
	OBJ_MINI_CART_SPRITE_TimeBest.sx = OBJ_MINI_CART_SPRITE_TimeBest.sy = scale;

	// HISTORY TIMERS
	for (int i = 0; i < 3; ++i)
	{
		njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_TimeBest, i, 22046.0, NJD_SPRITE_ALPHA, QueuedModelFlagsB_SomeTextureThing);

		if (rwp->subTotal_a[i] == 0)
		{
			SetMaterial(0.7f, 0.7f, 0.7f, 0.7f);
		}

		DrawTimer(rwp->subTotal_a[i], OBJ_MINI_CART_SPRITE_TimeBest.p.x + 48.0f * scale, OBJ_MINI_CART_SPRITE_TimeBest.p.y, scale);

		___njSetConstantMaterial(&SpriteColor_White);

		OBJ_MINI_CART_SPRITE_TimeBest.p.y += 16 * scaleY;
	}
}

static void __cdecl dispRaceM(task* tp)
{
	if (!MissedFrames && HideHud >= 0)
	{
		auto wk = (RaceWkM*)tp->mwp;

		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);

		//MirenEffSaveState:
		ghDefaultBlendingMode();
		___njSetConstantMaterial(&SpriteColor_White);
		njControl3D_Backup();
		BackupConstantAttr();

		njSetTexture(&OBJ_MINI_CART_TEXLIST);

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (SplitScreen::IsScreenEnabled(i))
			{
				dispRaceSingle(&wk->work[i], i);
			}
		}

		// MirenEffLoadState:
		ghDefaultBlendingMode();
		njControl3D_Restore();
		RestoreConstantAttr();
		ResetMaterial();

		SplitScreen::RestoreViewPort();
	}
}

static void __cdecl execRaceM(task* tp)
{
	auto wk = (RaceWkM*)tp->mwp;
	bool finished = true;

	switch (wk->work->mode)
	{
	case RACEMD_INTRO:
		if (--wk->work->timer <= 0)
		{
			++wk->work->dialState;

			if (wk->work->dialState < 4)
			{
				MirenSoundPlayOneShotSE(SE_CT_READY, NULL, (VolumeInfo*)0x7E75D4);
				wk->work->timer = 60;
			}
			else
			{
				MirenSoundPlayOneShotSE(SE_CT_START, NULL, (VolumeInfo*)0x7E75E0);
				wk->work->mode = RACEMD_GAME;
				PadReadOnP(-1);
			}
		}
		break;
	case RACEMD_GAME:
	case RACEMD_CKPT:
		if (wk->work->timer && --wk->work->timer <= 0)
		{
			wk->work->mode = RACEMD_GAME;
		}

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (playertp[i])
			{
				auto rwp = &wk->work[i];

				if (playerpwp[i]->item & Powerups_Dead)
				{
					rwp->lastChekPoint = 0;
				}

				if (cartGoalFlagM[i] == true)
				{
					continue;
				}

				finished = false;

				if (rwp->totalIntrpt < 360000)
				{
					rwp->totalIntrpt += 2;
				}

				if (rwp->lapIntrpt_a[rwp->displayLap] < 360000)
				{
					rwp->lapIntrpt_a[rwp->displayLap] += 2;
				}
			}
		}

		if (finished == true)
		{
			SleepTimer();
			CartGoalFlag = TRUE;
			wk->work->mode = RACEMD_GOAL;
			wk->work->timer = 180;
		}

		break;
	case RACEMD_GOAL:
		if (--wk->work->timer <= 0)
		{
			SetWinnerMulti(wk->winner);
			SetFinishAction();
			wk->work->mode = RACEMD_END;
		}
		break;
	}

	tp->disp(tp);
}

static void __cdecl initRaceM(task* tp, void* param_p)
{
	auto wk = (RaceWkM*)tp->mwp;

	memset(wk, 0, sizeof(RaceWkM));
	wk->winner = -1;
	wk->work->mode = RACEMD_WAIT1;
	wk->work->timer = 60;
	
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto* rwp = &wk->work[i];
		rwp->currentLap = -1;
		rwp->displayLap = 0;
		rwp->dialState = 0;
		rwp->lastChekPoint = 2;
		rwp->bestTotalTime = CartDataGetBestTotalTime();

		for (int lap = 0; lap < 3; ++lap)
		{
			rwp->bestSubTotal_a[lap] = CartDataGetBestSubTotalTime(lap);
		}

		cartGoalFlagM[i] = false;
	}

	RaceManageTask_p = tp;
	CartInitLetter();
}

static const TaskInfo infoM = { sizeof(RaceWkM), 1, initRaceM, execRaceM, dispRaceM, nullptr };

static void CartRaceSetManageTask_m()
{
	MirenSetTask(LEV_2, &infoM, 0);
}

// ROUND TASK:

static void __cdecl deadRoundM(task* tp)
{
	ResetMleriRangeRad();
}

static void LoadAdditionalCarts()
{
	for (int i = 1; i < PLAYER_MAX; ++i)
	{
		if (playertwp[i])
		{
			auto tp = CreateElementalTask(LoadObj_Data1 | LoadObj_UnknownA | LoadObj_UnknownB, LEV_3, EnemyCart);
			if (tp && tp->twp)
			{
				tp->twp->btimer = i;
				tp->twp->scl.y = 1.0f;
				tp->twp->pos = { 1513.0f, 9.0f, 74.0f };
				tp->twp->ang.y = 0xC000;
			}
		}
	}
}

static void SetCartCameraDemo_m()
{
	SetCartCameraDemo(); // too messy to rewrite lol

	for (int i = 1; i < PLAYER_MAX; ++i)
	{
		CameraSetEventCamera_m(i, CAMMD_CART, CAMADJ_NONE);

		auto param = GetCamAnyParam(i);
		*param = *GetCamAnyParam(0);
	}
}

static void __cdecl execRoundM(task* task_p)
{
	auto twp = task_p->twp;

	if (twp->mode == 0)
	{
		LoadAdditionalCarts();
		CartRaceSetManageTask_m();
		SetCartCameraDemo_m();
		PadReadOn();
		PadReadOffP(-1);
		EnablePause();
		//CartActInitActDiff();
		SetFrameRateMode(1, 1);
		twp->mode = 1;
	}
	else
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			SetFreeCameraMode_m(i, FALSE);
		}
	}
}

static void __cdecl initRoundM(task* task_p, void* param_p)
{
	task_p->mwp->work.b[0] = 0;
	auto bgm_tp = CreateElementalTask(2u, LEV_0, (TaskFuncPtr)0x4DAA40);
	bgm_tp->twp->mode = MusicIDs_circuit;
	bgm_tp->twp->wtimer = 10;

	//InitSnatchPlayerInfo(v3);
	SetMleriRangeRad(2000.0f);
	//InitStartDushCheck();
	execRoundM(task_p);
}

static const TaskInfo RdTaskInfoM = { 1, 2, initRoundM, execRoundM, 0, deadRoundM };

void __cdecl Rd_MiniCart_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		MirenInitTask(tp, &RdTaskInfoM, nullptr);
	}
	else
	{
		Rd_MiniCart_h.Original(tp);
	}
}

static void goalRaceM(taskwk* pltwp, int pnum)
{
	MirenSoundPlayOneShotSE(SE_CT_GOAL, NULL, (VolumeInfo*)0x7E75EC);

	cartGoalFlagM[pnum] = true;

	switch (TASKWK_CHARID(pltwp))
	{
	case Characters_Sonic:
		if (!MetalSonicFlag)
		{
			PlayVoice(1838);
		}
		break;
	case Characters_Tails:
		PlayVoice(1801);
		break;
	case Characters_Knuckles:
		PlayVoice(1788);
		break;
	case Characters_Amy:
		PlayVoice(1731);
		break;
	case Characters_Gamma:
		PlayVoice(1768);
		break;
	case Characters_Big:
		PlayVoice(1746);
		break;
	default:
		return;
	}
}

static void __cdecl TwinkleCircuitZoneTask_r(task* tp) // custom name
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pltwp = CCL_IsHitPlayer(twp);

		if (pltwp)
		{
			auto cpt = tp->mwp->work.b[0];
			auto pnum = TASKWK_PLAYERID(pltwp);

			auto wk = (RaceWkM*)RaceManageTask_p->mwp;
			auto rwp = &wk->work[pnum];

			if (cpt == 2)
			{
				if (!rwp->lastChekPoint && rwp->currentLap > -1)
				{
					--rwp->currentLap;
				}
			}
			else if (rwp->lastChekPoint == 2 && rwp->currentLap < 3)
			{
				if (++rwp->currentLap < 3)
				{
					if (rwp->currentLap > rwp->displayLap)
					{
						rwp->subTotal_a[rwp->displayLap] = rwp->totalIntrpt;
						rwp->displayLap = rwp->currentLap;
						wk->work->timer = 180;
						wk->work->mode = RACEMD_CKPT;
					}
				}
				else
				{
					rwp->subTotal_a[rwp->displayLap] = rwp->totalIntrpt;
					rwp->displayLap = 2;
					goalRaceM(pltwp, pnum);
					if (wk->winner == -1) wk->winner = pnum;
				}
			}

			rwp->lastChekPoint = cpt;
		}

		EntryColliList(twp);
	}
	else
	{
		TwinkleCircuitZoneTask_h.Original(tp);
	}
}

void InitRace()
{
	TwinkleCircuitZoneTask_h.Hook(TwinkleCircuitZoneTask_r);
	Rd_MiniCart_h.Hook(Rd_MiniCart_r);
}
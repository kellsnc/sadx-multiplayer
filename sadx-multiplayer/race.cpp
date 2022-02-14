#include "pch.h"
#include "splitscreen.h"
#include "utils.h"
#include "race.h"

/*

Multiplayer Twinkle Circuit manager

*/

Trampoline* TwinkleCircuitZoneTask_t = nullptr;
Trampoline* Rd_MiniCart_t = nullptr;

// RACE MANAGER:

struct RacerWk // custom
{
	char lastChekPoint;
	char currentLap;
	char displayLap;
	int totalIntrpt;
	int lapIntrpt_a[3];
	int subTotal_a[3];
	int bestSubTotal_a[3];
	int bestLapTime_a[3];
};

#pragma pack(push, 1)
struct RaceWkM // multiplayer version of RaceWk in symbols
{
	char mode;
	__int16 timer;
	char dialState;
	RacerWk racers[PLAYER_MAX];
};
#pragma pack(pop)

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

static void DrawTimer(int time, int x, int y, float s)
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

static void dispRaceSingle(RacerWk* rwp, int num)
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
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_LAPS_A, 0, 22045.998, NJD_SPRITE_ALPHA, QueuedModelFlagsB_SomeTextureThing);

	OBJ_MINI_CART_SPRITE_LAPS_B.p.x = 640.0f * scaleX - 16.0f * scale - 96.0f * scale + 39.0f * scale + HorizontalResolution * ratio->x;
	OBJ_MINI_CART_SPRITE_LAPS_B.p.y = 99.0f * scaleY + VerticalResolution * ratio->y;
	OBJ_MINI_CART_SPRITE_LAPS_B.sx = OBJ_MINI_CART_SPRITE_LAPS_B.sy = scale;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_LAPS_B, rwp->displayLap, 22046.0, NJD_SPRITE_ALPHA, QueuedModelFlagsB_SomeTextureThing);

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
				dispRaceSingle(&wk->racers[i], i);
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

	switch (wk->mode)
	{
	case RACEMD_WAIT0:
	case RACEMD_WAIT1:
		if (!Cart_demo_flag)
		{
			wk->mode = RACEMD_INTRO;
		}
		break;
	case RACEMD_INTRO:
		if (--wk->timer <= 0)
		{
			++wk->dialState;

			if (wk->dialState < 4)
			{
				dsPlay_oneshot(701, 0, 0, 0);
				wk->timer = 60;
			}
			else
			{
				dsPlay_oneshot(702, 0, 0, 0);
				wk->mode = RACEMD_GAME;
				ToggleControllers(true);
			}
		}
		break;
	case RACEMD_GAME:
	case RACEMD_CKPT:
		if (wk->timer && --wk->timer <= 0)
		{
			wk->mode = RACEMD_GAME;
		}

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (!playertp[i])
			{
				continue;
			}

			auto rwp = &wk->racers[i];

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

		if (finished == true)
		{
			CartGoalFlag = TRUE;
			wk->mode = RACEMD_GOAL;
			wk->timer = 180;
		}

		break;
	case RACEMD_GOAL:
		if (--wk->timer <= 0)
		{
			wk->mode = RACEMD_END;
		}
		break;
	}
	
	tp->disp(tp);
}

static void __cdecl initRaceM(task* tp, void* param_p)
{
	auto wk = (RaceWkM*)tp->mwp;

	if (wk)
	{
		memset(wk, 0, sizeof(RaceWkM));
	}

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		cartGoalFlagM[i] = false;
	}

	RaceManageTask_p = tp;
	CartInitLetter(); 
}

static const TaskInfo infoM = { sizeof(RaceWkM), 1, initRaceM, execRaceM, dispRaceM, nullptr };

// ROUND TASK:

static void __cdecl deadRoundM(task* tp)
{
	ResetMleriRangeRad();
}

static void __cdecl execRoundM(task* tp)
{
	
}

static void LoadAdditionalCarts()
{
	for (int i = 1; i < multiplayer::GetPlayerCount(); ++i)
	{
		auto tp = CreateElementalTask(LoadObj_Data1 | LoadObj_UnknownA | LoadObj_UnknownB, LEV_3, EnemyCart);

		if (tp && tp->twp)
		{
			tp->twp->btimer = i;

			if (playertwp[i])
			{
				tp->twp->scl.y = 1.0f;
				tp->twp->pos = { 1513.0f, 9.0f, 74.0f };
				tp->twp->ang.y = 0xC000;
			}
		}
	}
}

static void __cdecl initRoundM(task* tp, void* param_p)
{
	InitFreeCamera();
	LoadAdditionalCarts();
	EnableControl();
	ToggleControllers(false);
	MirenSetTask(LEV_2, &infoM, 0);
	SetFrameRateMode(1, 1);
	EnablePause();
	PlayMusic(MusicIDs_TwinkleCircuit);
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
		TARGET_DYNAMIC(Rd_MiniCart)(tp);
	}
}

static void goalRaceM(taskwk* pltwp, int pnum)
{
	dsPlay_oneshot(703, 0, 0, 0);

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
			auto racewk = (RaceWkM*)RaceManageTask_p->mwp;
			auto wk = &racewk->racers[pnum];

			if (cpt == 2)
			{
				if (!wk->lastChekPoint && wk->currentLap > -1)
				{
					--wk->currentLap;
				}
			}
			else if (wk->lastChekPoint == 2 && wk->currentLap < 3)
			{
				if (++wk->currentLap < 3)
				{
					if (wk->currentLap > wk->displayLap)
					{
						wk->subTotal_a[wk->displayLap] = wk->totalIntrpt;
						wk->displayLap = wk->currentLap;
						racewk->timer = 180;
						racewk->mode = RACEMD_CKPT;
					}
				}
				else
				{
					wk->subTotal_a[wk->displayLap] = wk->totalIntrpt;
					wk->displayLap = 2;
					goalRaceM(pltwp, pnum);
				}
			}

			wk->lastChekPoint = cpt;
		}

		EntryColliList(twp);
	}
	else
	{
		TARGET_DYNAMIC(TwinkleCircuitZoneTask)(tp);
	}
}

void InitRace()
{
	TwinkleCircuitZoneTask_t = new Trampoline(0x4DBCF0, 0x4DBCF8, TwinkleCircuitZoneTask_r);
	Rd_MiniCart_t = new Trampoline(0x4DAA80, 0x4DAA86, Rd_MiniCart_r);
}
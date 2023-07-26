#include "pch.h"
#include "camera.h"
#include "teleport.h"

static void __cdecl KillHimP_r(int pNum);
static void __cdecl KillHimByFallingDownP_r(int pno);
static void __cdecl KillPlayerFallingDownStageP_r(task* tp);
static void __cdecl BreathCounterP_r(task* tp);

static FunctionHook<void, int> KillHimP_t(KillHimP, KillHimP_r);
static FunctionHook<void, int> KillHimByFallingDownP_t(KillHimByFallingDownP, KillHimByFallingDownP_r);
TaskHook KillPlayerFallingDownStageP_t(0x44AE80, KillPlayerFallingDownStageP_r);
TaskHook BreathCounterP_t(0x446B10, BreathCounterP_r);

void __cdecl GamePlayerMissedFree(task* tp)
{
	BYTEn(tp->ptp->twp->counter.l, tp->awp->work.ul[1]) = FALSE;
}

void KillAndWarpPlayers(char pNum)
{
	auto twp = playertwp[pNum];
	auto pwp = playerpwp[pNum];

	if (!CheckEditMode() && playertp[pNum])
	{
		if (GetNumPlayerM(pNum) <= 0)
		{
			if (multiplayer::IsCoopMode() && (GameMode == GameModes_Adventure_Field || GameMode == GameModes_Adventure_ActionStg))
			{
				ScreenFade_Start_();
				ssGameMode = MD_GAME_FADEOUT_OVER;
			}
			else
			{
				SetChangeGameMode(GAMEMD_CLEAR);
			}
			
			TempEraseSound();
		}
		else
		{
			if (multiplayer::IsBattleMode() || pNum == 0)
			{
				AddNumPlayerM(pNum, -1); // Remove one life
			}

			SetNumRingM(pNum, 0);
			TeleportPlayerToStart(pNum);

			// Reset time for Gamma in coop
			if (multiplayer::IsCoopMode() && continue_data.continue_flag)
			{
				SetTime2(continue_data.minutes, continue_data.second, continue_data.frame);
			}

			CameraReleaseEventCamera_m(pNum);

			if (isPlayerInCart(pNum))
			{
				CameraSetEventCamera_m(pNum, CAMMD_CART, CAMADJ_NONE);
			}

			// Don't reset mode if player is riding something
			if (!isPlayerInCart(pNum) && !isPlayerOnSnowBoard(pNum))
			{
				SetInputP(pNum, PL_OP_LETITGO);
				twp->mode = 1;
			}

			if (pwp)
			{
				if (pwp->equipment & Upgrades_SuperSonic)
				{
					SetInputP(pNum, PL_OP_NORMAL);
				}

				pwp->item = 0;
			}
		}
	}
}

void __cdecl GamePlayerMissed_r(task* tp)
{
	auto awp = tp->awp;
	auto pNum = awp->work.ul[1];
	auto twp = playertwp[pNum];
	auto pwp = playerpwp[pNum];

	if (!twp)
	{
		FreeTask(tp);
		return;
	}

	pwp->item |= Powerups_Dead;

	if (++awp->work.ul[0] > 0x78)
	{
		KillAndWarpPlayers(pNum);
		FreeTask(tp);
	}
}

static void __cdecl KillHimP_r(int pNum)
{
	if (multiplayer::IsActive())
	{
		if (playerpwp[pNum] && !(playerpwp[pNum]->item & Powerups_Dead))
		{
			playertwp[pNum]->flag |= 0x1000;
			playerpwp[pNum]->item |= Powerups_Dead;
			SetInputP(pNum, PL_OP_KILLED);
			auto tp = CreateElementalTask(LoadObj_UnknownB, 0, GamePlayerMissed_r);
			tp->awp->work.ul[1] = pNum;
		}
	}
	else
	{
		KillHimP_t.Original(pNum);
	}
}

void ExecFallingDownP_r(int pNum)
{
	auto ptwp = playertwp[pNum];
	auto ppwp = playerpwp[pNum];

	CameraSetEventCameraFunc_m(pNum, CameraStay, CAMADJ_NONE, CDM_NONE);

	if (!ppwp || !(ppwp->item & Powerups_Dead))
	{
		switch (TASKWK_CHARID(ptwp))
		{
		case Characters_Sonic:
			if (MetalSonicFlag)
			{
				PlayVoice(2046);
			}
			else
			{
				dsPlay_oneshot(1503, 0, 0, 0);
			}
			break;
		case Characters_Tails:
			dsPlay_oneshot(1465, 0, 0, 0);

			if (ptwp)
			{
				ptwp->flag |= 0x10u;
				SetInputP(pNum, PL_OP_LETITGO);
			}
			break;
		case Characters_Knuckles:
			dsPlay_oneshot(1453, 0, 0, 0);

			if (ptwp)
			{
				ptwp->flag |= 0x10u;
				SetInputP(pNum, PL_OP_LETITGO);
			}
			break;
		case Characters_Amy:
			dsPlay_oneshot(1396, 0, 0, 0);
			break;
		case Characters_Gamma:
			dsPlay_oneshot(1433, 0, 0, 0);
			break;
		case Characters_Big:
			dsPlay_oneshot(1412, 0, 0, 0);
			break;
		}
	}
}

static void __cdecl KillHimByFallingDownP_r(int pno)
{
	if (multiplayer::IsActive())
	{
		auto ctp = CreateElementalTask(LoadObj_UnknownB, LEV_0, GamePlayerMissed_r);
		ctp->awp->work.ul[1] = pno;
		ExecFallingDownP_r(pno);
	}
	else
	{
		KillHimByFallingDownP_t.Original(pno);
	}
}

static void __cdecl KillPlayerFallingDownStageP_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		return KillPlayerFallingDownStageP_t.Original(tp);
	}

	LoopTaskC(tp);

	auto dzl = KillingCollisionModelsListList[CurrentLevel];

	if (!dzl)
	{
		return;
	}

	auto dz = dzl[CurrentAct];

	if (dz)
	{
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			auto ptwp = playertwp[i];

			if (ptwp == nullptr || BYTEn(tp->twp->counter.l, i) == TRUE)
			{
				continue;
			}

			zxsdwstr carry;
			carry.pos = ptwp->pos;

			auto dz_ = dz;

			while (dz_->character)
			{
				if (GetZxShadowOnFDPolygon(&carry, dz_->object))
				{
					if (!carry.lower.findflag)
					{
						if (carry.upper.findflag)
						{
							if (fabs(carry.pos.y - carry.upper.onpos) <= 30.0f)
							{
								BYTEn(tp->twp->counter.l, i) = TRUE;

								auto ctp = CreateChildTask(LoadObj_UnknownB, GamePlayerMissed_r, tp);
								ctp->dest = GamePlayerMissedFree;
								ctp->awp->work.ul[1] = i;

								ExecFallingDownP_r(i);
								break;
							}
						}
					}
				}

				++dz_;
			}
		}
	}
}

static void __cdecl BreathCounterP_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		return BreathCounterP_t.Original(tp);
	}

	int DrownVoice = 1506;
	auto data = tp->awp;
	auto timer = data->work.ul[1]++;
	static const int timeOver = 760;
	_BOOL1 isTimeUp = timer == timeOver;
	char pnum = data->work.ub[0];
	auto player = playertwp[pnum];

	if (!player)
	{
		FreeTask(tp);
		return;
	}

	if (timer < timeOver)
	{
		if (playerpwp[pnum]->breathtimer < 60)
		{
			FreeTask(tp);
			return;
		}
	}
	if (isTimeUp)
	{
		SetInputP(pnum, 39); //drown	

		switch (player->counter.b[1])
		{
		case Characters_Tails:
			DrownVoice = 1468;
			break;
		case Characters_Knuckles:
			DrownVoice = 1452;
			break;
		case Characters_Amy:
			DrownVoice = 1397;
			break;
		case Characters_Big:
			DrownVoice = 1417;
			break;
		}
		if (MetalSonicFlag)
		{
			PlayVoice(2046);
		}
		else
		{
			PlaySound(DrownVoice, 0, 0, 0);
		}
	}
	else if (timer == 920)
	{
		playerpwp[pnum]->breathtimer = 0;
		KillAndWarpPlayers(pnum);
	}
	else if (GetDebugMode())
	{
		FreeTask(tp);
		playerpwp[pnum]->breathtimer = 0;
	}
	else
	{
		if (timer <= 1)
		{
			tp->dest = (void(__cdecl*)(task*))j_RestoreLastSong;
			tp->disp = (void(__cdecl*)(task*))0x440D20;
			PlayJingle(96);		
		}

		tp->disp(tp);
	}
}
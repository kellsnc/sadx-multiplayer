#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "splitscreen.h"
#include "camera.h"
#include "teleport.h"

FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> PGetRotation_h(0x44BB60);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> PGetAcceleration_h(0x44C270);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*, float> PGetAccelerationSnowBoard_h(0x448550);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> PGetAccelerationForBuilding_h(0x448150);
FastUsercallHookPtr<void(*)(taskwk* twp), noret, rESI> SonicMotionCheckEdition(0x492170);
FastFunctionHook<void, taskwk*> PPlayADXAsWaiting_h(0x442360); //idle
FastFunctionHook<void, int> KillHimP_h(KillHimP);
FastFunctionHook<void, int> KillHimByFallingDownP_h(KillHimByFallingDownP);
FastFunctionHook<void, task*> KillPlayerFallingDownStageP_h(0x44AE80);
FastFunctionHook<void, task*> BreathCounterP_h(0x446B10);

void __cdecl PGetRotation_r(taskwk* twp, motionwk2* mwp, playerwk* pwp) // todo: rewrite
{
	if (splitscreen::IsActive() && pwp->attr & 0x20000)
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			PGetRotation_h.Original(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	PGetRotation_h.Original(twp, mwp, pwp);
}

void __cdecl PGetAcceleration_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (splitscreen::IsActive() && pwp->attr & 0x20000)
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			PGetAcceleration_h.Original(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	PGetAcceleration_h.Original(twp, mwp, pwp);
}

void __cdecl PGetAccelerationSnowBoard_r(taskwk* twp, motionwk2* mwp, playerwk* pwp, float Max_Speed)
{
	if (splitscreen::IsActive())
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			camera_twp->smode = GetStageNumber() == 0x802 && twp->pos.x > -5400.0f && twp->pos.y > -3900.0f ? 1 : 0;
			PGetAccelerationSnowBoard_h.Original(twp, mwp, pwp, Max_Speed);
			camera_twp->ang = backup;
			camera_twp->smode = 0;
			return;
		}
	}

	PGetAccelerationSnowBoard_h.Original(twp, mwp, pwp, Max_Speed);
}

void __cdecl PGetAccelerationForBuilding_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (splitscreen::IsActive())
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			PGetAccelerationForBuilding_h.Original(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	PGetAccelerationForBuilding_h.Original(twp, mwp, pwp);
}

void __cdecl SonicMotionCheckEdition_r(taskwk* twp)
{
	if (splitscreen::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(twp);
		auto& per = perG[pnum];

		if (per.press & Buttons_B)
		{
			SetPlayerInitialPosition(twp);
		}

		Float lx = (per.x1 << 8);
		Float ly = (per.y1 << 8);

		if (per.on & Buttons_X)
		{
			if (lx > 3072.0f || lx < -3072.0f || ly > 3072.0f || ly < -3072.0f)
			{
				if (ly == 0.0f)
				{
					twp->pos.y = twp->pos.y - 0.0 * 5.0;
				}
				else
				{
					twp->pos.y = twp->pos.y - ly / njSqrt(ly * ly) * 5.0f;
				}
			}
		}
		else
		{
			if (lx > 3072.0 || lx < -3072.0 || ly > 3072.0f || ly < -3072.0f)
			{
				auto camera_ang = GetCameraAngle(pnum);

				if (!camera_ang)
				{
					camera_ang = &camera_twp->ang;
				}

				Float ang = -camera_ang->y - -njArcTan2(ly, lx);
				twp->pos.x = njCos(ang) * 5.0f + twp->pos.x;
				twp->pos.z = njSin(ang) * 5.0f + twp->pos.z;
			}
		}
	}
	else
	{
		SonicMotionCheckEdition.Original(twp);
	}
}

// Make idle voice works for other players
void __cdecl PPlayADXAsWaiting_r(taskwk* pTwp)
{
	if (!multiplayer::IsActive() || !pTwp || !pTwp->counter.b[0])
	{
		return PPlayADXAsWaiting_h.Original(pTwp);
	}

	const uint8_t charID = pTwp->counter.b[1];
	const Sint16 act = ssActNumber;
	auto lvlID = (ssActNumber | (ssStageNumber << 8)) >> 8;

	if (EV_CheckCansel())
		return;

	if (charID == Characters_Amy && lvlID == LevelIDs_EggCarrierInside && act == 2)
	{
		lvlID = 0;
	}

	__int16* voiceList = plADXNamePlayingAsWaiting[lvlID];
	int voiceID = 0;

	switch (charID)
	{
	case Characters_Sonic:
		voiceID = lvlID != 19 ? 390 : 396;
		break;
	case Characters_Tails:
		voiceID = 391;
		break;
	case Characters_Knuckles:
		voiceID = 392;
		break;
	case Characters_Amy:
		voiceID = 393;
		break;
	case Characters_Gamma:
		voiceID = 394;
		break;
	case Characters_Big:
		voiceID = 395;
		break;
	default:
		voiceID = ssActNumber;
		break;
	}

	if (voiceList)
	{
		if (njRandom() >= 0.5f)
		{
			int curVoice = *voiceList;
			int16_t* curLevelVoiceList = nullptr;

			if (*voiceList != -1)
			{
				while (1)
				{
					curLevelVoiceList = voiceList + 1;
					int16_t index = *curLevelVoiceList;

					if (curVoice == charID)
					{
						break;
					}

					curVoice = curLevelVoiceList[index + 1];
					voiceList = &curLevelVoiceList[index + 1];

					if (curVoice == -1)
					{
						PlayVoice(voiceID);
						return;
					}
				}

				int16_t v9 = *curLevelVoiceList;
				int16_t* index = curLevelVoiceList + 1;

				if (v9 == 1)
				{
					PlayVoice(*index);
					return;
				}

				int v13 = (njRandom() * v9);

				if (IsLevelChaoGarden())
				{
					if (v13 == v9)
					{
						v13--;
					}
				}
				else
				{
					v13 = ssAct;
				}

				int v15 = v9 - 1;
				if (v15 < v13)
				{
					v13 = v15;
				}

				voiceID = index[v13];
			}
		}
	}

	PlayVoice(voiceID);
}

void __cdecl HoldOnIcicleP_r(Uint8 pno, task* ttp)
{
	auto ptwp = playertwp[pno];

	if (!(ptwp->flag & Status_HoldObject))
	{
		SetFreeCameraMode_m(pno, FALSE);
		SetInputP(pno, PL_OP_HOLDONICICLE);
		playerpwp[pno]->htp = ttp;
	}
}

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
			AddNumPlayerM(pNum, -1); // Remove one life
			SetNumRingM(pNum, 0);
			TeleportPlayerToStart(pNum);

			// Reset time for Gamma in coop
			if (multiplayer::IsCoopMode() && continue_data.continue_flag)
			{
				SetTime2(continue_data.minutes, continue_data.second, continue_data.frame);
			}

			CameraReleaseEventCamera_m(pNum);

			if (IsPlayerInCart(pNum))
			{
				CameraSetEventCamera_m(pNum, CAMMD_CART, CAMADJ_NONE);
			}

			// Don't reset mode if player is riding something
			if (!IsPlayerInCart(pNum) && !IsPlayerOnSnowboard(pNum))
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
		KillHimP_h.Original(pNum);
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
		KillHimByFallingDownP_h.Original(pno);
	}
}

static void __cdecl KillPlayerFallingDownStageP_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		return KillPlayerFallingDownStageP_h.Original(tp);
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
				if (dz_->object && GetZxShadowOnFDPolygon(&carry, dz_->object))
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
		return BreathCounterP_h.Original(tp);
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

// Trick the game to draw shadows on other screens
Bool ChkPause_Shadow_Hack()
{
	return ChkPause() || splitscreen::numScreen != 0;
}

// The game sometimes wrongly check for player 1 while the rest of the code doesn't
// We force these cases to check for any player
Bool CheckPlayerRideOnMobileLandObjectP_P1_Hack(unsigned __int8 pno, task* ttp)
{
	if (multiplayer::IsActive())
	{
		return IsPlayerOnDyncol(ttp);
	}
	else
	{
		return CheckPlayerRideOnMobileLandObjectP(pno, ttp);
	}
}

void patch_player_init()
{
	PGetRotation_h.Hook(PGetRotation_r);
	PGetAcceleration_h.Hook(PGetAcceleration_r);
	PGetAccelerationSnowBoard_h.Hook(PGetAccelerationSnowBoard_r);
	PGetAccelerationForBuilding_h.Hook(PGetAccelerationForBuilding_r);
	SonicMotionCheckEdition.Hook(SonicMotionCheckEdition_r);
	PPlayADXAsWaiting_h.Hook(PPlayADXAsWaiting_r); //patch idle voice multiplayer
	WriteJump(HoldOnIcicleP, HoldOnIcicleP_r); // Disable free camera for the proper player on icicles

	KillHimP_h.Hook(KillHimP_r);
	KillHimByFallingDownP_h.Hook(KillHimByFallingDownP_r);
	KillPlayerFallingDownStageP_h.Hook(KillPlayerFallingDownStageP_r);
	BreathCounterP_h.Hook(BreathCounterP_r);

	// Game draws shadow in logic sub but also in display sub *if* game is paused.
	// To keep compatibility with mods (like SA2 Sonic), I just force the display for the other screens.
	// Better alternative is to skip display in logic and always draw in display, but I chose max compatibility.
	WriteCall((void*)0x494B57, ChkPause_Shadow_Hack); // Sonic
	WriteCall((void*)0x461420, ChkPause_Shadow_Hack); // Tails
	WriteCall((void*)0x472674, ChkPause_Shadow_Hack); // Knuckles
	WriteCall((void*)0x4875F9, ChkPause_Shadow_Hack); // Amy
	WriteCall((void*)0x48BA5A, ChkPause_Shadow_Hack); // Big
	WriteCall((void*)0x480702, ChkPause_Shadow_Hack); // Gamma

	// Patch CheckPlayerRideOnMobileLandObjectP occurences that don't need full rewrites
	WriteCall((void*)0x4CB36C, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Switch
	WriteCall((void*)0x52130E, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OStation
	WriteCall((void*)0x522F7E, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OSidelift
	WriteCall((void*)0x523185, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OSidelift
	WriteCall((void*)0x526AAE, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OBlift
	WriteCall((void*)0x52A5CE, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OStation
	WriteCall((void*)0x59DC87, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OUkijima
	WriteCall((void*)0x59DCA0, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OUkijima
	WriteCall((void*)0x5AC110, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Cargo
	WriteCall((void*)0x5AC153, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Cargo
	WriteCall((void*)0x5E6B26, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Aokiswitch
	WriteCall((void*)0x5FA25C, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Edge
	WriteCall((void*)0x5E6B26, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Connect
	WriteCall((void*)0x5E6B26, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Talap
	WriteCall((void*)0x601FB0, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // ExplosionRock
	WriteCall((void*)0x63D865, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
	WriteCall((void*)0x63D90D, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
	WriteCall((void*)0x63D940, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
	WriteCall((void*)0x63DFB6, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
	WriteCall((void*)0x63DFED, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
}

RegisterPatch patch_player(patch_player_init);
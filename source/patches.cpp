#include "pch.h"
#include "camera.h"
#include "splitscreen.h"
#include "objects/bosses.h"

#include "FastFunctionHook.hpp"

/*

General patches to allow compatibility for 4+ players

*/

FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> PGetRotation_t(0x44BB60);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> PGetAcceleration_t(0x44C270);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*, float> PGetAccelerationSnowBoard_t(0x448550);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> PGetAccelerationForBuilding_t(0x448150);
FastFunctionHook<void, task*> Ring_t(0x450370);
FastFunctionHook<void, task*> Tobitiri_t(0x44FD10);
FastFunctionHook<Float, taskwk*, int> EnemyDist2FromPlayer_t(0x4CD610);
FastFunctionHook<Angle, taskwk*, enemywk*, Uint8> EnemyCalcPlayerAngle_t(0x4CD670);
FastFunctionHook<Angle, taskwk*, enemywk*, Uint8> EnemyTurnToPlayer_t(0x4CD6F0);
FastFunctionHook<void, task*> TikalDisplay_t(0x7B33A0);
FastFunctionHook<void, task*> SpinnaDisplayer_t(0x4AFD80);
FastFunctionHook<void> ListGroundForDrawing_t(0x43A900);
FastFunctionHook<int, taskwk*, enemywk*> EnemyCheckDamage_t((intptr_t)OhNoImDead);
FastFunctionHook<task*, NJS_POINT3*, NJS_POINT3*, float> SetCircleLimit_t(0x7AF3E0);
FastUsercallHookPtr<void(*)(taskwk* twp), noret, rESI> SonicMotionCheckEdition(0x492170);
FastUsercallHookPtr<Bool(*)(taskwk* twp), rEAX, rEDI> PlayerVacumedRing_t(0x44FA90);
FastFunctionHook<void, task*> OGate2_Main_t(0x59C850);
FastFunctionHook<void, taskwk*> PPlayADXAsWaiting_t(0x442360); //idle

FastFunctionHook<void, task*> OTpRing_t((intptr_t)OTpRing);

void __cdecl PGetRotation_r(taskwk* twp, motionwk2* mwp, playerwk* pwp) // todo: rewrite
{
	if (SplitScreen::IsActive() && pwp->attr & 0x20000)
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			PGetRotation_t.Original(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	PGetRotation_t.Original(twp, mwp, pwp);
}

void __cdecl PGetAcceleration_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (SplitScreen::IsActive() && pwp->attr & 0x20000)
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			PGetAcceleration_t.Original(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	PGetAcceleration_t.Original(twp, mwp, pwp);
}

void __cdecl PGetAccelerationSnowBoard_r(taskwk* twp, motionwk2* mwp, playerwk* pwp, float Max_Speed)
{
	if (SplitScreen::IsActive())
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			camera_twp->smode = GetStageNumber() == 0x802 && twp->pos.x > -5400.0f && twp->pos.y > -3900.0f ? 1 : 0;
			PGetAccelerationSnowBoard_t.Original(twp, mwp, pwp, Max_Speed);
			camera_twp->ang = backup;
			camera_twp->smode = 0;
			return;
		}
	}

	PGetAccelerationSnowBoard_t.Original(twp, mwp, pwp, Max_Speed);
}

void __cdecl PGetAccelerationForBuilding_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (SplitScreen::IsActive())
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			PGetAccelerationForBuilding_t.Original(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	PGetAccelerationForBuilding_t.Original(twp, mwp, pwp);
}

void __cdecl SonicMotionCheckEdition_r(taskwk* twp)
{
	if (SplitScreen::IsActive())
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

bool GrabRingMulti(taskwk* twp, task* tp)
{
	auto player = CCL_IsHitPlayer(twp);

	if (player)
	{
		int pID = TASKWK_PLAYERID(player);

		if (!(playerpwp[pID]->item & 0x4000))
		{
			twp->mode = 2;
			AddNumRingM(pID, 1);
			dsPlay_oneshot(7, 0, 0, 0);
			tp->disp = RingDoneDisplayer;
			return true;
		}
	}

	return false;
}

// Patch for other players to collect rings
void __cdecl Ring_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1)
		{
			if (GrabRingMulti(twp, tp))
				return;
		}
	}

	Ring_t.Original(tp);
}

void __cdecl OTpRing_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1)
		{
			if (GrabRingMulti(twp, tp))
				return;
		}
	}

	OTpRing_t.Original(tp);
}

// Patch for other players to collect scattered rings
void __cdecl Tobitiri_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1 || twp->mode == 2)
		{
			auto player = CCL_IsHitPlayer(twp);

			if (player)
			{
				twp->mode = 4;
				twp->pos.y += 3.44f;
				twp->scl.x = -2.0f;
				twp->scl.z = -4.0f;
				twp->counter.l = 0;

				ResetParticle((EntityData1*)twp, (NJS_SPRITE*)0x3B42FC0);

				int pID = TASKWK_PLAYERID(player);

				if (!(playerpwp[pID]->item & 0x4000))
				{
					AddNumRingM(pID, 1);
					dsPlay_oneshot(7, 0, 0, 0);
					tp->disp = RingDoneDisplayer;
					return;
				}
			}
		}
	}

	Tobitiri_t.Original(tp);
}

// Patch for other players to get kill score
BOOL __cdecl EnemyCheckDamage_r(taskwk* twp, enemywk* ewp)
{
	if (!multiplayer::IsActive())
	{
		return EnemyCheckDamage_t.Original(twp, ewp);
	}

	if (twp->flag & 4)
	{
		ewp->buyoscale = 0.35f;

		auto hit_twp = CCL_IsHitPlayer(twp);

		if (hit_twp)
		{
			auto pID = TASKWK_PLAYERID(hit_twp);

			ewp->flag |= 0x1000;
			AddEnemyScoreM(pID, 10);

			if (twp->cwp->hit_cwp->info[twp->cwp->hit_num].attr & 0x1000)
			{
				ewp->flag |= 0x800;
				ewp->velo.y = 0.7f;
			}
			else
			{
				ewp->flag &= ~0x400;
				EnemyBumpPlayer(pID);
			}
		}

		hit_twp = CCL_IsHitBullet(twp);

		if (hit_twp)
		{
			ewp->flag |= 0x1000;
			AddEnemyScoreM(hit_twp->btimer, 10); // we put the player id in btimer

			if (twp->cwp->hit_cwp->info[twp->cwp->hit_num].attr & 0x1000)
			{
				ewp->flag |= 0x800;
			}
			else
			{
				ewp->flag &= ~0x400;
			}
		}

		return TRUE;
	}
	else
	{
		return CheckItemExplosion(&twp->pos);
	}
}

// EBuyon is the only object to manually call AddEnemyScore
void __cdecl EBuyon_ScorePatch(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto player = CCL_IsHitPlayer(tp->twp);

		if (player)
		{
			AddEnemyScoreM(TASKWK_PLAYERID(player), 100);
		}
	}
	else
	{
		AddEnemyScore(100);
	}
}

// Despite taking player id, it always gets 0 so let's check closest player
float __cdecl EnemyDist2FromPlayer_r(taskwk* twp, int num)
{
	if (multiplayer::IsActive() && num == 0)
	{
		return EnemyDist2FromPlayer_t.Original(twp, GetClosestPlayerNum(&twp->pos));
	}
	else
	{
		return EnemyDist2FromPlayer_t.Original(twp, num);
	}
}

// Despite taking player id, it always gets 0 so let's check closest player
Angle __cdecl EnemyCalcPlayerAngle_r(taskwk* twp, enemywk* ewp, Uint8 pnum)
{
	if (multiplayer::IsActive() && pnum == 0)
	{
		return EnemyCalcPlayerAngle_t.Original(twp, ewp, GetClosestPlayerNum(&twp->pos));
	}
	else
	{
		return EnemyCalcPlayerAngle_t.Original(twp, ewp, pnum);
	}
}

// Despite taking player id, it always gets 0 so let's check closest player
Angle __cdecl EnemyTurnToPlayer_r(taskwk* twp, enemywk* ewp, Uint8 pnum)
{
	if (multiplayer::IsActive() && pnum == 0)
	{
		return EnemyTurnToPlayer_t.Original(twp, ewp, GetClosestPlayerNum(&twp->pos));
	}
	else
	{
		return EnemyTurnToPlayer_t.Original(twp, ewp, pnum);
	}
}

// Trick the game to draw shadows on other screens
BOOL IsGamePausedOrNot1stScreen()
{
	return IsGamePaused() || SplitScreen::numScreen != 0;
}

// Add shadow rendering in Tikal's display routine because they forgot it
void TikalDisplay_r(task* tp)
{
	TikalDisplay_t.Original(tp);

	if (IsGamePausedOrNot1stScreen())
	{
		DrawCharacterShadow(tp->twp, &((playerwk*)tp->mwp->work.ptr)->shadow);
	}
}

static BOOL PlayerVacumedRing_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		// Get closest players with magnetic field
		taskwk* pltwp_ = nullptr;
		playerwk* plpwp = nullptr;
		float dist = 10000000.0f;

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto pltwp = playertwp[i];
			plpwp = playerpwp[i];

			if (pltwp && plpwp && plpwp->item & Powerups_MagneticBarrier)
			{
				NJS_VECTOR v
				{
					twp->pos.x - pltwp->pos.x,
					twp->pos.y - pltwp->pos.y,
					twp->pos.z - pltwp->pos.z
				};

				auto curdist = njScalor(&v);

				if (curdist < dist)
				{
					dist = curdist;
					pltwp_ = pltwp;
				}
			}
		}

		// found one
		if (pltwp_ && (dist < 50.0f || twp->wtimer))
		{
			NJS_VECTOR dir = { 0.0f, 7.0f, 0.0f };
			njPushMatrix(_nj_unit_matrix_);
			if (pltwp_->ang.z) njRotateZ(0, pltwp_->ang.z);
			if (pltwp_->ang.x) njRotateX(0, pltwp_->ang.x);
			if (pltwp_->ang.y) njRotateY(0, pltwp_->ang.y);
			njCalcPoint(0, &dir, &dir);
			njPopMatrixEx();

			dir.x += pltwp_->pos.x;
			dir.y += pltwp_->pos.y;
			dir.z += pltwp_->pos.z;

			// clamp
			if (dist > 50.0f)
			{
				dist = 50.0f;
			}

			dist = min(5.0f, max(0.85f, dist * 0.026f));

			if (plpwp)
			{
				dist *= (njScalor(&plpwp->spd) * 0.5f + 1.0f);
			}

			CalcAdvanceAsPossible(&twp->pos, &dir, dist, &twp->pos);
			++twp->wtimer;

			twp->counter.f = twp->counter.f + 3.0f;
			EntryColliList(twp);
			return TRUE;
		}

		return FALSE;
	}
	else
	{
		return PlayerVacumedRing_t.Original(twp);
	}
}

void SpinnaDrawShield(taskwk* twp)
{
	for (int i = 0; i < 16; ++i)
	{
		Angle mod1 = NJM_DEG_ANG(180.0 * (0.5 - njRandom()));
		Angle mod2 = NJM_DEG_ANG(360.0 * njRandom());
		float mod3 = static_cast<float>((njRandom() + 0.5) * 10.0);

		NJS_VECTOR posm;
		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		NJS_VECTOR pos1 = twp->pos;
		NJS_VECTOR pos2 = twp->pos;

		njAddVector(&pos1, &posm);

		pos1.y += 5.0f;
		pos2.y += 5.0f;
		DrawLineV(&pos2, &pos1);

		pos2 = pos1;
		mod1 += NJM_DEG_ANG(120.0 * (0.5 - njRandom()));
		mod2 += NJM_DEG_ANG(80.0 * (0.5 - njRandom()));
		mod3 = static_cast<float>((njRandom() + 1.0) * 4.0);

		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		njAddVector(&pos1, &posm);
		DrawLineV(&pos2, &pos1);

		pos2 = pos1;
		mod1 += NJM_DEG_ANG(120.0 * (0.5 - njRandom()));
		mod2 += NJM_DEG_ANG(80.0 * (0.5 - njRandom()));
		mod3 = static_cast<float>((njRandom() + 1.0) * 2.0);

		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		njAddVector(&pos1, &posm);
		DrawLineV(&pos2, &pos1);
	}
}

void __cdecl SpinnaDisplayer_r(task* tp)
{
	if (SplitScreen::IsActive())
	{
		if (!MissedFrames)
		{
			auto twp = tp->twp;

			SpinnaDraw(twp, (enemywk*)tp->mwp);

			if (SplitScreen::GetCurrentScreenNum() > 0 && twp->smode == 0 && twp->wtimer != 0)
			{
				SpinnaDrawShield(twp);
			}
		}
	}
	else
	{
		SpinnaDisplayer_t.Original(tp);
	}
}

BOOL __cdecl dsCheckViewV_r(NJS_POINT3* ft, float radius)
{
	if (multiplayer::IsActive())
	{
		return true;
	}
	else
	{
		return dsCheckViewV(ft, radius);
	}
}

void __cdecl ListGroundForDrawing_r()
{
	if (SplitScreen::IsActive())
	{
		auto cam_pos = GetCameraPosition(SplitScreen::GetCurrentScreenNum());
		auto cam_ang = GetCameraAngle(SplitScreen::GetCurrentScreenNum());

		if (!cam_pos || !cam_ang)
		{
			return;
		}

		numDisplayEntry = 0;

		NJS_POINT3 center = { 0.0f, 0.0f, MaxDrawDistance * -0.5f };

		njPushMatrix(_nj_unit_matrix_);
		njRotateY_(cam_ang->y);
		njRotateX_(cam_ang->x);
		njCalcPoint(0, &center, &center);
		njPopMatrixEx();

		center.x += cam_pos->x;
		center.y += cam_pos->y;
		center.z += cam_pos->z;

		for (int i = 0; i < pObjLandTable->ssCount; ++i)
		{
			auto col = &pObjLandTable->pLandEntry[i];

			if (!col->blockbit || col->blockbit & MaskBlock)
			{
				if (col->slAttribute & ColFlags_Visible)
				{
					if (col->slAttribute & ColFlags_UseSkyDrawDist || GetDistance(&center, (NJS_POINT3*)&col->xCenter) < col->xWidth + MaxDrawDistance)
					{
						pDisplayEntry[numDisplayEntry++] = col;
					}
				}
			}
		}
	}
	else
	{
		ListGroundForDrawing_t.Original();
	}
}

static void __cdecl InitTimer_r()
{
	TimeFrames = 0;
	TimeSeconds = 0;
	TimeMinutes = 0;
	GameTimer = 0;

	if (CurrentCharacter == Characters_Gamma && !multiplayer::IsBattleMode())
	{
		TimeMinutes = 3;
		TimeSeconds = 0;
	}
}

static void __cdecl InitTime_r()
{
	ulGlobalTimer = 0;
	InitTimer_r();
}

bool CheckAnyPlayerRideOnMobileLandObjectP(unsigned __int8 pno, task* ttp)
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

task* __cdecl SetCircleLimit_r(NJS_POINT3* pos, NJS_POINT3* center, float radius)
{
	if (multiplayer::IsActive())
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (playertwp[i])
			{
				task* tp = CreateElementalTask((LoadObj_Data1), 0, CircleLimit);

				if (tp)
				{
					auto twp = tp->twp;
					twp->pos = *center;
					twp->scl.x = radius;
					twp->counter.l = (unsigned int)&playertwp[i]->pos;
				}
			}
		}
	}

	return SetCircleLimit_t.Original(pos, center, radius);
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

void __cdecl SetTimeFrame_r(Sint8 minutes, Sint8 second, Sint8 frame)
{
	if (multiplayer::IsActive() && GameState == 15)
		return;

	TimeMinutes = minutes,
	TimeSeconds = second;
	TimeFrames = frame;
}

void __cdecl OGate2_Main_r(task* tp)
{
	auto twp = tp->twp;

	//if none of the player is Amy 
	if (!isOnePlayerSpecifiedChar(Characters_Amy))
	{
		if (twp->mode < 2)
			twp->mode = 2; //force the door to open
	}

	OGate2_Main_t.Original(tp);
}

void __cdecl FixShakeoffGarbageAction(Uint8 pno, Uint8 mode)
{
	if (!multiplayer::IsActive())
	{
		SetInputP(pno, mode);
	}
}

Sint32 __cdecl Casino_FixKnuxCheck(Uint8 pno)
{
	if (multiplayer::IsCoopMode() && CurrentCharacter != Characters_Knuckles)
	{
		return Characters_Sonic;
	}

	return GetPlayerCharacterName(pno);
}

//make idle voice works for other players
void __cdecl PPlayADXAsWaiting_r(taskwk* pTwp)
{
	if (!multiplayer::IsActive() || !pTwp || !pTwp->counter.b[0])
	{
		return PPlayADXAsWaiting_t.Original(pTwp);
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

void InitMiscPatches()
{
	Ring_t.Hook(Ring_r);
	Tobitiri_t.Hook(Tobitiri_r);
	OTpRing_t.Hook(OTpRing_r);
	PlayerVacumedRing_t.Hook(PlayerVacumedRing_r);
	ListGroundForDrawing_t.Hook(ListGroundForDrawing_r);

	// Player
	PGetRotation_t.Hook(PGetRotation_r);
	PGetAcceleration_t.Hook(PGetAcceleration_r);
	PGetAccelerationSnowBoard_t.Hook(PGetAccelerationSnowBoard_r);
	PGetAccelerationForBuilding_t.Hook(PGetAccelerationForBuilding_r);
	SonicMotionCheckEdition.Hook(SonicMotionCheckEdition_r);
	PPlayADXAsWaiting_t.Hook(PPlayADXAsWaiting_r); //patch idle voice multiplayer

	// Misc
	WriteJump(HoldOnIcicleP, HoldOnIcicleP_r); // Disable free camera for the proper player on icicles
	WriteJump(LoadPlayerMotionData, _advertise_prolog); // Fix missing animations with testspawn
	WriteData((uint8_t*)0x500017, (uint8_t)PLAYER_MAX); // Patch launch ramp EC for 8 players
	WriteCall((void*)0x44EE0A, SetTimeFrame_r); // Don't reset time with death in multiplayer
	WriteCall((void*)0x5C5906, FixShakeoffGarbageAction); // This make the game crashes as Tails.

	// Score patches
	EnemyCheckDamage_t.Hook(EnemyCheckDamage_r);
	WriteCall((void*)0x7B3273, EBuyon_ScorePatch); // EBuyon: add 100 points to proper player
	WriteData<5>((void*)0x7B326D, 0x90); // EBuyon: remove original 100 points for player 0

	// Enemy player checks
	EnemyDist2FromPlayer_t.Hook(EnemyDist2FromPlayer_r);
	EnemyCalcPlayerAngle_t.Hook(EnemyCalcPlayerAngle_r);
	EnemyTurnToPlayer_t.Hook(EnemyTurnToPlayer_r);
	WriteData((char*)0x4CCB3F, (char)PLAYER_MAX); // EnemySearchPlayer

	// Enemies
	SpinnaDisplayer_t.Hook(SpinnaDisplayer_r);

	// Bosses
	SetCircleLimit_t.Hook(SetCircleLimit_r);

	//open door in hot shelter if not amy
	OGate2_Main_t.Hook(OGate2_Main_r);

	// Character shadows:
	// Game draws shadow in logic sub but also in display sub *if* game is paused.
	// To keep compatibility with mods (like SA2 Sonic), I just force the display for the other screens.
	// Better alternative is to skip display in logic and always draw in display, but I chose max compatibility.
	WriteCall((void*)0x494B57, IsGamePausedOrNot1stScreen); // Sonic
	WriteCall((void*)0x461420, IsGamePausedOrNot1stScreen); // Tails
	WriteCall((void*)0x472674, IsGamePausedOrNot1stScreen); // Knuckles
	WriteCall((void*)0x4875F9, IsGamePausedOrNot1stScreen); // Amy
	WriteCall((void*)0x48BA5A, IsGamePausedOrNot1stScreen); // Big
	WriteCall((void*)0x480702, IsGamePausedOrNot1stScreen); // Gamma
	TikalDisplay_t.Hook(TikalDisplay_r);

	// Springs for 4+ players
	WriteData((uint8_t*)0x7A4DC4, (uint8_t)PLAYER_MAX); // ObjectSpring
	WriteData((uint8_t*)0x7A4FF7, (uint8_t)PLAYER_MAX); // ObjectSpringB
	WriteData((uint8_t*)0x79F77C, (uint8_t)PLAYER_MAX); // spring_h_exec

	// dsCheckViewV in exec functions
	WriteCall((void*)0x4E138F, dsCheckViewV_r); // wv hane, bigfloot, saku...
	WriteCall((void*)0x5D3D54, dsCheckViewV_r); // ObjectCasinoCris
	WriteCall((void*)0x5E8327, dsCheckViewV_r); // ObjectRuinFire
	WriteCall((void*)0x4AB41A, dsCheckViewV_r); // EnemySaru
	WriteCall((void*)0x7AA307, dsCheckViewV_r); // EnemyPondSkater

	// Normal start timer for Gamma in multiplayer
	WriteJump((void*)0x425FF0, InitTimer_r);
	WriteJump((void*)0x427F10, InitTime_r);

	// Patch CheckPlayerRideOnMobileLandObjectP occurences that don't need full rewrites
	WriteCall((void*)0x4CB36C, CheckAnyPlayerRideOnMobileLandObjectP); // Switch
	WriteCall((void*)0x52130E, CheckAnyPlayerRideOnMobileLandObjectP); // OStation
	WriteCall((void*)0x522F7E, CheckAnyPlayerRideOnMobileLandObjectP); // OSidelift
	WriteCall((void*)0x523185, CheckAnyPlayerRideOnMobileLandObjectP); // OSidelift
	WriteCall((void*)0x526AAE, CheckAnyPlayerRideOnMobileLandObjectP); // OBlift
	WriteCall((void*)0x52A5CE, CheckAnyPlayerRideOnMobileLandObjectP); // OStation
	WriteCall((void*)0x59DC87, CheckAnyPlayerRideOnMobileLandObjectP); // OUkijima
	WriteCall((void*)0x59DCA0, CheckAnyPlayerRideOnMobileLandObjectP); // OUkijima
	WriteCall((void*)0x5AC110, CheckAnyPlayerRideOnMobileLandObjectP); // Cargo
	WriteCall((void*)0x5AC153, CheckAnyPlayerRideOnMobileLandObjectP); // Cargo
	WriteCall((void*)0x5E6B26, CheckAnyPlayerRideOnMobileLandObjectP); // Aokiswitch
	WriteCall((void*)0x5FA25C, CheckAnyPlayerRideOnMobileLandObjectP); // Edge
	WriteCall((void*)0x5E6B26, CheckAnyPlayerRideOnMobileLandObjectP); // Connect
	WriteCall((void*)0x5E6B26, CheckAnyPlayerRideOnMobileLandObjectP); // Talap
	WriteCall((void*)0x601FB0, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63D865, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63D90D, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63D940, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63DFB6, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63DFED, CheckAnyPlayerRideOnMobileLandObjectP);

	// Casino knuckles checks
	WriteCall((void*)0x5C060B, Casino_FixKnuxCheck);
	WriteCall((void*)0x5C058B, Casino_FixKnuxCheck);
	WriteCall((void*)0x5C068B, Casino_FixKnuxCheck);
	WriteCall((void*)0x5C441A, Casino_FixKnuxCheck);
}

#include "pch.h"
#include "FastFunctionHook.hpp"
#include "splitscreen.h"
#include "teleport.h"
#include "result.h"
#include "levels.h"

/*

Level-related adjustements for multiplayer

*/

FastFunctionHook<void, task*> Rd_Chaos2_t(0x54A700);
FastFunctionHook<void, task*> Rd_Chaos4_t(0x550A30);
FastFunctionHook<void, task*> Rd_Chaos6_t(0x557920);

FastFunctionHook<void, task*> Rd_Beach_t(0x4F6D60);
FastFunctionHook<void, task*> Rd_Windy_t(0x4DDB30);
FastFunctionHook<void, task*> Rd_Mountain_t(0x601550);
FastFunctionHook<void, task*> Rd_Twinkle_t(0x61D150);
FastFunctionHook<void, task*> Rd_Ruin_t(0x5E18B0);

FastFunctionHook<void, task*> dispBgSnow_t(0x4E9950);
FastFunctionHook<void, task*> dispBgHighway_t(0x610570);
FastFunctionHook<void, task*> dispBgTwinkle_t(0x61D1F0);

FastFunctionHook<void, task*> Bg_MysticRuin_t(0x530670);
FastFunctionHook<void, task*> Bg_Past_t(0x542030);

FastUsercallHookPtr<Bool(*)(taskwk* twp), rEAX, rESI> CollisionSceneChangerSS_CheckCollision_t(0x640550);
FastUsercallHookPtr<void(*)(task* tp), noret, rEAX> ObjectMysticRuinTorokko_ExecATask_t(0x53D830);

VoidFunc(HighwayMaskBlock, 0x60FEE0); // real name: "checkCamera"
FunctionPointer(void, TwinkleMaskBlock, (taskwk* twp), 0x60FEE0); // real name: "checkCamera"
DataPointer(NJS_OBJECT, object_s1_nbg1_nbg1, 0x26A0EC0);
DataPointer(NJS_OBJECT, object_s2_yakei_yakei, 0x26A48E0);
DataPointer(uint8_t, byte_3C75126, 0x3C75126);
DataPointer(uint16_t, word_3C75124, 0x3C75124);
DataPointer(int, ring_kiran, 0x38D8D64);

static auto setTPFog = GenerateUsercallWrapper<void (*)(unsigned __int8 mode)>(noret, 0x61CAC0, rAL);
static auto RdRuinInit = GenerateUsercallWrapper<void (*)(task * tp)>(noret, 0x5E1670, rEDI);

void MultiArena(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 0)
	{
		PlayMusic(MusicIDs_bossall);

		switch (CurrentLevel)
		{
		case LevelIDs_Chaos0:
			setRainEffect();
			SetTableBG_Chaos0();
			CreateElementalTask(LoadObj_Data1 | LoadObj_Data2, LEV_2, BossChaos0);
			break;
		case LevelIDs_Chaos2:
			SetTableBG_Chaos2();
			CreateElementalTask(LoadObj_Data1, LEV_1, Chaos2Column);
			break;
		case LevelIDs_Chaos4:
			SetTableBG_Chaos4();
			C4SuimenYurashiSet();
			C4LeafSetOld();
			break;
		case LevelIDs_Chaos6:
			SetTableBG_Chaos6();
			CreateElementalTask(LoadObj_Data1, LEV_3, EggCarrierCloud_c6);
			break;
		case LevelIDs_EggHornet:
			InitIndirectEffect3D_Bossegm1();
			break;
		case LevelIDs_EggWalker:
			CurrentCharacter = Characters_Tails;

			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				if (playertwp[i])
				{
					SetPlayerInitialPosition(playertwp[i]);
				}
			}
			break;
		case LevelIDs_E101:
			SetTableBG_E101();
			break;
		case LevelIDs_E101R:
			CurrentCharacter = Characters_Gamma;

			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				if (playertwp[i])
				{
					SetPlayerInitialPosition(playertwp[i]);
				}
			}

			break;
		}

		twp->mode = 1;
	}
	else
	{
		switch (CurrentLevel)
		{
		case LevelIDs_Chaos0:
			chaostwp->pos.y = -1000;
			chaostwp->mode = 0x13;
			break;
		case LevelIDs_EggWalker:
			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				if (!playertwp[i])
				{
					continue;
				}

				if (playertwp[i]->pos.z >= 930.0f)
				{
					if (playertwp[i]->pos.z > 1480.0f)
					{
						playertwp[i]->pos.z = 1480.0f;
					}
				}
				else
				{
					playertwp[i]->pos.z = 930.0f;
				}

				if (playertwp[i]->pos.x >= -515.0f)
				{
					if (playertwp[i]->pos.x > -375.0)
					{
						playertwp[i]->pos.x = -375.0;
					}
				}
				else
				{
					playertwp[i]->pos.x = -515.0f;
				}

				if (playertwp[i]->pos.y >= -3.0f)
				{
					if (playertwp[i]->pos.y > 45.0f)
					{
						playertwp[i]->pos.y = 45.0f;
					}
				}
				else
				{
					playertwp[i]->pos.y = -3.0f;
				}
			}

			break;
		}
	}
}

void __cdecl Rd_Chaos2_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		Rd_Chaos2_t.Original(tp);
	}
}

void __cdecl Rd_Chaos4_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		Rd_Chaos4_t.Original(tp);
	}
}

void __cdecl Rd_Chaos6_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		Rd_Chaos6_t.Original(tp);
	}
}

void __cdecl Rd_Beach_r(task* tp)
{
	if (ssActNumber == 0 && multiplayer::IsActive())
	{
		if (IsPlayerInSphere(5746.0f, 406.0f, 655.0f, 22.0f))
		{
			tp->twp->mode = 0;
			ChangeActM(1);

			DataPointer(task*, BeachWaveTp, 0x3C5E3D0);

			if (BeachWaveTp)
			{
				FreeTask(BeachWaveTp);
				BeachWaveTp = nullptr;
			}

			return;
		}
	}

	Rd_Beach_t.Original(tp);
}

void __cdecl Rd_Windy_r(task* tp)
{
	if (ssActNumber == 1 && multiplayer::IsActive())
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (playertwp[i] && playertwp[i]->pos.y > 2250.0f)
			{
				tp->twp->mode = 0;
				ChangeActM(1);
				return;
			}
		}
	}

	Rd_Windy_t.Original(tp);
}

void __cdecl Rd_Mountain_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		if (ssActNumber == 0)
		{
			if (IsPlayerInSphere(-3667.0f, -400.0f, -2319.0f, 400.0f))
			{
				ChangeActM(1);
				dsEditLightInit();
				FreeTaskC(tp);
				RdMountainInit(tp);
				return;
			}
		}
		else if (ssActNumber == 1 && CurrentCharacter != Characters_Gamma)
		{
			// Reset lava at the beginning
			if (rd_mountain_twp && rd_mountain_twp->scl.x != 130.0f && IsPlayerInSphere(-380.0f, 440.0f, 1446.0f, 200.0f))
			{
				rd_mountain_twp->scl.x = 130.0f;
				rd_mountain_twp->scl.y = -0.5f;
			}
		}

		//patch an issue where the original function was taking priority for act swap
		if (tp->twp->mode != 1)
			Rd_Mountain_t.Original(tp);
		else
			LoopTaskC(tp);
	}
	else
	{
		Rd_Mountain_t.Original(tp);
	}
}

void __cdecl Rd_Twinkle_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		switch (twp->mode)
		{
		case 0i8:
			SetFreeCameraMode(1);
			rdTwinkleInit(tp);
			tp->dest = (TaskFuncPtr)0x61CA80;
			break;
		case 1i8:
			SetFreeCameraMode(0);

			if (IsPlayerInSphere(-6550.0f, -6720.0f, 23320.0f, 50.0f))
			{
				ChangeActM(1);
				rdTwinkleInit(tp);
				return;
			}
			break;
		case 2i8:
			SetFreeCameraMode(0);

			if (IsPlayerInSphere(-55.0f, 153.0f, -1000.0f, 50.0f))
			{
				tp->twp->mode = 3;
				EV_NpcMilesStandByOff();
				rdTwinkleInit(tp);
				setTPFog(tp->twp->mode);
				return;
			}
			break;
		case 3i8:
		{
			SetFreeCameraMode(1);

			if (IsPlayerInSphere(80.0f, 0.0f, -300.0f, 50.0f))
			{
				ChangeActM(1);
				rdTwinkleInit(tp);
				TeleportPlayers(82.0f, 0.0f, -305.0f);
			}

			break;
		}
		case 4i8:
		{
			SetFreeCameraMode(0);

			if (IsPlayerInSphere(350.0f, 100.0f, 550.0f, 36.0f))
			{
				ChangeActM(-1);
				rdTwinkleInit(tp);
				TeleportPlayers(328.0f, 100.0f, 566.0f);
			}

			break;
		}
		case 5i8:
			ResetMleriRangeRad();
			break;
		}
	}
	else
	{
		Rd_Twinkle_t.Original(tp);
	}
}

void __cdecl Rd_Ruin_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		switch (twp->mode)
		{
		case 0i8:
			RdRuinInit(tp);
			break;
		case 1i8:
			if (IsPlayerInSphere(6111.0f, -2445.0f, 1333.0f, 40.0f))
			{
				ChangeActM(1);
				twp->mode = 0i8;
			}
			break;
		case 2i8:

		{
			auto pnum = IsPlayerInSphere(6441.0f, -2421.0f, 1162.0f, 50.0f) - 1;

			if (pnum >= 0)
			{
				SetWinnerMulti(pnum); // Set winner there because act 3 consists of nothing
				ChangeActM(1);
				twp->mode = 0i8;
			}
		}

		if (byte_3C75126)
		{
			if (byte_3C75126 == 1)
			{
				if (playertwp[GetClosestPlayerNum(7740.0f, -2431.0f, 948.0f)]->pos.x >= 7600.0f)
				{
					if (++word_3C75124 > 600)
					{
						word_3C75124 = 0;
						ring_kiran = 1;
						SetSwitchOnOff(3u, 0);
						byte_3C75126 = 0;
					}
				}
				else
				{
					SetSwitchOnOff(3u, 1);
					ring_kiran = 0;
					byte_3C75126 = 2;
				}
			}
			else if (byte_3C75126 == 2)
			{
				if (++word_3C75124 > 600)
				{
					word_3C75124 = 0;
					ring_kiran = 1;
					byte_3C75126 = 0;
				}
				SetSwitchOnOff(3u, 0);
			}
		}
		else if (GetSwitchOnOff(2u))
		{
			byte_3C75126 = 1;
			word_3C75124 = 0;
		}

		break;
		}
	}
	else
	{
		Rd_Ruin_t.Original(tp);
	}
}

static void __cdecl dispBgSnow_r(task* tp)
{
	dispBgSnow_t.Original(tp);

	if (camera_twp && tp->twp->mode == 10 && SplitScreen::IsActive())
	{
		NJS_POINT3* pos = &camera_twp->pos;

		if (pos->y <= 200.0f)
		{
			if (pos->y < 0.0f)
			{
				if (pos->x >= 1612.0f)
				{
					MaskBlock = 0x1C;
				}
				else
				{
					MaskBlock = 0x6;
				}
			}
			else
			{
				if (pos->x >= 1666.0f)
				{
					MaskBlock = 0x18;
				}
				else
				{
					MaskBlock = 0x6;
				}
			}
		}
		else if (pos->x >= 1666.0f)
		{
			MaskBlock = 0x78;
		}
		else
		{
			MaskBlock = 0x62;
		}
	}
}

static void __cdecl dispBgHighway_r(task* tp)
{
	if (tp->twp->mode == 4 && SplitScreen::IsActive())
	{
		auto cam_twp = camera_twp;

		if (camera_twp)
		{
			LoopTaskC(tp);

			njControl3D_Backup();
			njControl3D_Add(NJD_CONTROL_3D_NO_CLIP_CHECK);
			njControl3D_Remove(NJD_CONTROL_3D_DEPTH_QUEUE);
			___njFogDisable();
			___njClipZ(gClipSky.f32Near, gClipSky.f32Far);
			if (cam_twp->pos.y > -10400.0f)
			{
				njPushMatrixEx();
				njTranslateEx(&cam_twp->pos);
				njScaleEx(&gSkyScale);
				njSetTexture(&bg_highway_TEXLIST);
				dsDrawModel(object_s1_nbg1_nbg1.basicdxmodel);
				njTranslate(0, 0.0f, -10000.0f - cam_twp->pos.y * 0.2f, 0.0f);
				njSetTexture(&bg_highway02_TEXLIST);
				dsDrawModel(object_s2_yakei_yakei.basicdxmodel);
				njPopMatrixEx();
			}
			___njClipZ(gClipMap.f32Near, gClipMap.f32Far);
			___njFogEnable();
			njControl3D_Restore();

			HighwayMaskBlock();
		}
	}
	else
	{
		dispBgHighway_t.Original(tp);
	}
}

static void __cdecl dispBgTwinkle_r(task* tp)
{
	dispBgTwinkle_t.Original(tp);

	if (camera_twp && pRd_Master && SplitScreen::IsActive())
	{
		TwinkleMaskBlock(pRd_Master->twp);
	}
}

static void __cdecl Create_Mountain_Cloud()
{
	CreateElementalTask(2u, LEV_1, (TaskFuncPtr)0x601230); // load task into slot 1 instead of 0 (to not run before the camera)
}

void __cdecl Bg_MysticRuin_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		Bg_MysticRuin_t.Original(tp);
		return;
	}

	auto twp = tp->twp;

	switch (twp->mode)
	{
	case 0:
		SkyBox_MysticRuins_TimeOfDayLightDirection((ObjectMaster*)tp);
		twp->mode++;
		break;
	case 1:
		tp->disp = (TaskFuncPtr)SkyBox_MysticRuins_Display;
		twp->mode++;
		break;
	case 2:
		if (ssActNumber == twp->btimer)
		{
			if (twp->btimer == 2)
			{
				GetTimeOfDay();
			}

			tp->disp(tp);
		}
		else
		{
			twp->mode = 3;
		}
		break;
	case 3:
		SkyBox_MysticRuins_TimeOfDayLightDirection((ObjectMaster*)tp);
		twp->mode = 2;
		break;
	default:
		return;
	}
}

void __cdecl Bg_Past_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		Bg_Past_t.Original(tp);
		return;
	}

	auto twp = tp->twp;

	switch (twp->mode)
	{
	case 0:
		InitBgAct(0, tp);
		twp->mode++;
		break;
	case 1:
		tp->disp = Past_Disp;
		twp->mode++;
		break;
	case 2:
		if (ssActNumber == twp->wtimer)
		{
			if (twp->wtimer == 2)
			{
				njSin(twp->value.l);
				twp->value.l += 1024;
			}

			tp->disp(tp);
		}
		else
		{
			twp->mode = 3;
		}
		break;
	case 3:
		InitBgAct(0, tp);
		twp->mode = 2;
		break;
	default:
		return;
	}
}

Bool __cdecl CollisionSceneChangerSS_CheckCollision_r(taskwk* twp)
{
	if (!multiplayer::IsActive())
	{
		return CollisionSceneChangerSS_CheckCollision_t.Original(twp);
	}

	NJS_POINT3 ppos;

	int pnum = GetTheNearestPlayerNumber(&twp->pos);
	GetPlayerPosition(pnum, 0, &ppos, 0);

	Float x = ppos.x - twp->pos.x;
	Float z = ppos.z - twp->pos.z;

	if (fabs(ppos.y - (twp->pos.y + twp->timer.f) + 10.0f) <= twp->timer.f)
	{
		Float sin = njSin(-twp->ang.y);
		Float cos = njCos(-twp->ang.y);
		if (fabs(sin * z + cos * x) < twp->counter.f &&
			fabs(cos * z - sin * x) < twp->value.f)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void __cdecl ObjectMysticRuinTorokko_ExecATask_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		ObjectMysticRuinTorokko_ExecATask_t.Original(tp);
		return;
	}

	taskwk* twp = tp->twp;
	colliwk* cwp = twp->cwp;

	if (!twp)
	{
		return;
	}

	auto pnum = GetTheNearestPlayerNumber(&twp->pos);
	taskwk* pData = playertwp[pnum];

	if (!pData)
	{
		return;
	}

	NJS_POINT3 pos;

	switch (twp->smode)
	{
	case 0:
		njPushMatrix(nj_unit_matrix_);
		njTranslateV(0, &twp->pos);
		ROTATEY(0, twp->ang.y);
		njInvertMatrix(0);
		njCalcPoint(0, &pData->pos, &pos);
		njPopMatrix(1u);

		if (cwp)
		{
			if ((pos.z - -0.14892) * (pos.z - -0.14892) + (pos.x - -1.25203) * (pos.x - -1.25203) < 36.0)
			{
				cwp->info[1].attr |= 0x10;
				cwp->info[2].attr |= 0x10;
				cwp->info[3].attr |= 0x10;
				cwp->info[4].attr |= 0x10;
				twp->btimer = 0;
			}
			else
			{
				cwp->info[1].attr &= ~0x10;
				cwp->info[2].attr &= ~0x10;
				cwp->info[3].attr &= ~0x10;
				cwp->info[4].attr &= ~0x10;
				twp->btimer = 1;
			}
		}
		else
		{
			twp->btimer = 0;
		}

		if (cwp && (cwp->flag & 2) && cwp->hit_cwp->mytask == playertp[pnum] && cwp->my_num == 0 && cwp->hit_num == 0)
		{
			if (twp->flag & 0x1 && twp->btimer == 0)
			{
				if (++twp->wtimer >= 2)
				{
					for (int i = 0; i < PLAYER_MAX; i++)
					{
						if (playertwp[i])
						{
							SetInputP(i, PL_OP_PLACEON);
							RotatePlayer(i, twp->ang.y + 0x4000);
						}
					}
					twp->btimer = 1;
					twp->smode = 1;
				}
			}
		}
		else
		{
			twp->btimer = 0;
			twp->wtimer = 0;
		}
		break;
	case 1:
		playertwp[pnum]->pos.x = twp->pos.x;
		playertwp[pnum]->pos.y = twp->pos.y + 5.4f;
		playertwp[pnum]->pos.z = twp->pos.z;

		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (playertwp[i])
			{
				SetInputP(i, PL_OP_PLACEON);
				RotatePlayer(i, twp->ang.y + 0x4000);
				if (i != pnum)
					PositionPlayer(i, pData->pos.x, pData->pos.y, pData->pos.z + 3.0f * (float)i);
			}
		}

		twp->value.f -= 0.8f;
		if (twp->value.f <= 0.0f)
		{
			njPushMatrix(nj_unit_matrix_);
			njTranslateV(0, &twp->pos);
			ROTATEY(0, twp->ang.y);
			njInvertMatrix(0);
			njCalcPoint(0, &pData->pos, &RidePos);
			njPopMatrix(1u);
			MRTorokkoRideFlg = 1;
			twp->value.l = 0;
			dsPlay_oneshot(589, 0, 0, 0);
			twp->smode = 2;
		}
		break;
	case 2:
		pData = playertwp[0];
		njPushMatrix(nj_unit_matrix_);
		CalcMMMatrix(0, MoveAction[twp->timer.w[1]], twp->value.f, 0, 0);
		SetInstancedMatrix(0, 0);
		njCalcPoint(0, &RidePos, &pData->pos);
		njCalcVector(0, &XVec1, &pos);
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (playertwp[i])
			{
				if (i != 0)
					PositionPlayer(i, pData->pos.x, pData->pos.y, pData->pos.z + 5.0f * (float)i);
				RotatePlayer(i, 0x4000 - -njArcTan2(-pos.z, pos.x));
			}
		}
		njPopMatrix(1u);

		twp->value.f += 0.1f;
		twp->timer.w[0] += 0x1000;
		if (twp->value.f >= (float)MoveAction[twp->timer.w[1]]->motion->nbFrame - 1.0f)
		{
			twp->value.f = (float)MoveAction[twp->timer.w[1]]->motion->nbFrame - 1.0f;
		}

		dsPlay_timer(590, 0, 1, 0, 2);
		if (++twp->wtimer == 120)
		{
			switch (twp->timer.w[1])
			{
			case 0:
				if (GetPlayerCharacterName(0) == Characters_Gamma)
					SeqClrFlag(48);
				SetEntranceNumber(0);
				SeqChangeStage(LevelIDs_MysticRuins, 2);
				InitFreeCamera();
				break;
			case 1:
				SetEntranceNumber(0);
				SeqChangeStage(LevelIDs_MRGarden, 0);
				InitFreeCamera();
				break;
			case 2:
				SetEntranceNumber(2);
				SeqChangeStage(LevelIDs_MysticRuins, 0);
				InitFreeCamera();
				break;
			}
		}
		break;
	}

	if (CheckCollisionP(&twp->pos, 50.0f))
	{
		twp->flag |= 0x1;
	}
	else
	{
		twp->flag &= ~0x1;
	}
}

void __cdecl Casino_StartPos_r(Uint8 pno, Float x, Float y, Float z)
{
	if (multiplayer::IsActive())
	{
		NJS_POINT3 pos = { x, y, z };
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (playertwp[i])
			{
				TeleportPlayerArea(i, &pos, 5.0f);
			}
		}
	}
	else
	{
		SetPositionP(pno, x, y, z);
	}
}

void SetAllPlayersInitialPosition(taskwk* data)
{
	if (multiplayer::IsActive())
	{
		TeleportPlayersToStart();
	}
	else
	{
		SetPlayerInitialPosition(data);
	}
}

Sint32 __cdecl PatchGetPlayerCharacterName(Uint8 pno)
{
	if (multiplayer::IsActive())
	{
		return CurrentCharacter;
	}

	return GetPlayerCharacterName(pno);
}

void InitLevels()
{
	// Patch start positions
	WriteCall((void*)0x4150FA, SetAllPlayersInitialPosition); // General
	WriteCall((void*)0x4151B1, SetAllPlayersInitialPosition); // General
	WriteJump((void*)0x7B0B00, SetAllPlayersInitialPosition); // General
	WriteCall((void*)0x4DD52D, SetAllPlayersInitialPosition); // Windy Valley
	WriteCall((void*)0x5E15CA, SetAllPlayersInitialPosition); // Lost World
	WriteCall((void*)0x5EDC66, SetAllPlayersInitialPosition); // Sky Deck
	WriteCall((void*)0x5EFA31, SetAllPlayersInitialPosition); // Sky Deck
	WriteCall((void*)0x5EDD27, SetAllPlayersInitialPosition); // Sky Deck
	WriteCall((void*)0x5602F1, SetAllPlayersInitialPosition); // Perfect Chaos

	// Patch Skyboxes (display function managing mode)
	WriteData((void**)0x4F723E, (void*)0x4F71A0); // Emerald Coast
	WriteData((void**)0x4DDBFE, (void*)0x4DDB60); // Windy Valley
	WriteData((void**)0x61D57E, (void*)0x61D4E0); // Twinkle Park
	WriteData((void**)0x610A7E, (void*)0x6109E0); // Speed Highway
	WriteData((void**)0x5E1FCE, (void*)0x5E1F30); // Lost World
	WriteData((void**)0x4EA26E, (void*)0x4EA1D0); // Ice Cap
	Bg_MysticRuin_t.Hook(Bg_MysticRuin_r);
	Bg_Past_t.Hook(Bg_Past_r);
	
	// Emerald Coast Bridge
	WriteData<2>((void*)0x501B66, 0x90ui8);
	WriteData<2>((void*)0x501B12, 0x90ui8);

	// Windy Valley leaves
	WriteData((uint8_t*)0x4E4344, 0xEBui8);

	// Windy Valley PushUpWind
	WriteData((uint8_t*)0x4E5253, (uint8_t)PLAYER_MAX);

	// Ice Cap breath generator
	WriteData((uint8_t*)0x4E91AE, (uint8_t)PLAYER_MAX);

	// Twinkle Park - fix Amy character check
	WriteCall((void*)0x61CB77, PatchGetPlayerCharacterName); 

	// Speed Highway Act 2 skybox
	WriteData((taskwk***)0x610765, &camera_twp);

	// Red Mountain cloud layer
	WriteCall((void*)0x60147B, Create_Mountain_Cloud);
	WriteCall((void*)0x601404, Create_Mountain_Cloud);

	// Red Mountain Lava
	WriteData<2>((void*)0x6087D6, 0x90ui8);

	// Water animation
	WriteData((intptr_t*)0x4F734B, 0); // BeachTexAnimAct1
	WriteData((intptr_t*)0x4F73BB, 0); // BeachTexAnimWaveAct1
	WriteData((intptr_t*)0x4F743B, 0); // BeachTexAnimWaveAct2
	WriteData((intptr_t*)0x4F734B, 0); // BeachTexAnimSandAct2
	WriteData((intptr_t*)0x4F752B, 0); // BeachTexAnimWtAct2
	WriteData((intptr_t*)0x4F759B, 0); // BeachTexAnimWaveAct3
	WriteData((intptr_t*)0x4F761B, 0); // BeachTexAnimSandAct3
	WriteData((intptr_t*)0x4F768B, 0); // BeachTexAnimWtAct3*/
	WriteData((intptr_t*)0x61E3DB, 0); // TwinkleTexAnim
	WriteData((intptr_t*)0x5E20DB, 0); // RuinTexAnimAct1
	WriteData((intptr_t*)0x5E214B, 0); // RuinTexAnimAct2
	WriteData((intptr_t*)0x5E229B, 0); // RuinTexAnimAct3
	WriteData((intptr_t*)0x63179A, 0); // SSTexAnim

	// Remove landtable collision chunk optimisation
	WriteData((uint8_t*)0x4E91C0, 0xC3ui8); // Ice Cap

#ifdef MULTI_TEST
	// In battle mode, boss become fighting arenas
	Rd_Chaos2_t.Hook(Rd_Chaos2_r);
	Rd_Chaos4_t.Hook(Rd_Chaos4_r);
	Rd_Chaos6_t.Hook(Rd_Chaos6_r);
#endif

	// Act swap fixes
	Rd_Beach_t.Hook(Rd_Beach_r);
	Rd_Windy_t.Hook(Rd_Windy_r);
	Rd_Mountain_t.Hook(Rd_Mountain_r);
	Rd_Twinkle_t.Hook(Rd_Twinkle_r);
	Rd_Ruin_t.Hook(Rd_Ruin_r);
	
	// Casino area positions
	WriteCall((void*)0x5C0D67, Casino_StartPos_r);
	WriteCall((void*)0x5C0D9B, Casino_StartPos_r);
	WriteCall((void*)0x5C0DCB, Casino_StartPos_r);
	WriteCall((void*)0x5C0E19, Casino_StartPos_r);
	//WriteCall((void*)0x5C0E77, Casino_StartPos_r); // pinball
	//WriteCall((void*)0x5C0EF1, Casino_StartPos_r); // pinball

	// Move landtable mask flag to display for multiplayer compatibility
	dispBgSnow_t.Hook(dispBgSnow_r);
	dispBgHighway_t.Hook(dispBgHighway_r);
	dispBgTwinkle_t.Hook(dispBgTwinkle_r);

	// Hub world swap fixes
	CollisionSceneChangerSS_CheckCollision_t.Hook(CollisionSceneChangerSS_CheckCollision_r); // Station Square
	ObjectMysticRuinTorokko_ExecATask_t.Hook(ObjectMysticRuinTorokko_ExecATask_r); // Mystic Ruin minecarts
	WriteData((uint8_t*)0x54575C, 0x7Cui8); // Past
}
#include "pch.h"
#include "UsercallFunctionHandler.h"
#include "splitscreen.h"
#include "result.h"
#include "levels.h"

/*

Level-related adjustements for multiplayer

*/

Trampoline* Rd_Chaos0_t   = nullptr;
Trampoline* Rd_Chaos2_t   = nullptr;
Trampoline* Rd_Chaos4_t   = nullptr;
Trampoline* Rd_Chaos6_t   = nullptr;
Trampoline* Rd_Bossegm1_t = nullptr;
Trampoline* Rd_Bossegm2_t = nullptr;
Trampoline* Rd_E101_t     = nullptr;
Trampoline* Rd_E101_R_t   = nullptr;

Trampoline* Rd_Beach_t = nullptr;
Trampoline* Rd_Windy_t = nullptr;
Trampoline* Rd_Mountain_t = nullptr;
Trampoline* Rd_Twinkle_t = nullptr;
Trampoline* Rd_Ruin_t = nullptr;

Trampoline* dispBgSnow_t = nullptr;
Trampoline* dispBgHighway_t = nullptr;
Trampoline* dispBgTwinkle_t = nullptr;

VoidFunc(HighwayMaskBlock, 0x60FEE0); // real name: "checkCamera"
FunctionPointer(void, TwinkleMaskBlock, (taskwk* twp), 0x60FEE0); // real name: "checkCamera"
DataPointer(NJS_OBJECT, object_s1_nbg1_nbg1, 0x26A0EC0);
DataPointer(NJS_OBJECT, object_s2_yakei_yakei, 0x26A48E0);
DataPointer(uint8_t, byte_3C75126, 0x3C75126);
DataPointer(uint16_t, word_3C75124, 0x3C75124);
DataPointer(int, ring_kiran, 0x38D8D64);

static auto setTPFog = GenerateUsercallWrapper<void (*)(unsigned __int8 mode)>(noret, 0x61CAC0, rAL);
static auto RdRuinInit = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5E1670, rEDI);

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

void __cdecl Rd_Chaos0_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		TARGET_DYNAMIC(Rd_Chaos0)(tp);
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
		TARGET_DYNAMIC(Rd_Chaos2)(tp);
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
		TARGET_DYNAMIC(Rd_Chaos4)(tp);
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
		TARGET_DYNAMIC(Rd_Chaos6)(tp);
	}
}

void __cdecl Rd_Bossegm1_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		TARGET_DYNAMIC(Rd_Bossegm1)(tp);
	}
}

void __cdecl Rd_Bossegm2_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		TARGET_DYNAMIC(Rd_Bossegm2)(tp);
	}
}

void __cdecl Rd_E101_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		TARGET_DYNAMIC(Rd_E101)(tp);
	}
}

void __cdecl Rd_E101_R_r(task* tp)
{
	if (multiplayer::IsFightMode())
	{
		MultiArena(tp);
	}
	else
	{
		TARGET_DYNAMIC(Rd_E101_R)(tp);
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

	TARGET_DYNAMIC(Rd_Beach)(tp);
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

	TARGET_DYNAMIC(Rd_Windy)(tp);
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
		else if (ssActNumber == 1)
		{
			// Reset lava at the beginning
			if (IsPlayerInSphere(-380.0f, 440.0f, 1446.0f, 200.0f))
			{
				rd_mountain_twp->scl.x = 130.0f;
				rd_mountain_twp->scl.y = -0.5f;
			}
		}
	}

	TARGET_DYNAMIC(Rd_Mountain)(tp);
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

			auto pnum = IsPlayerInSphere(80.0f, 0.0f, -300.0f, 50.0f) - 1;

			if (pnum >= 0)
			{
				NJS_VECTOR pos = playertwp[pnum]->pos;
				Angle ang = playertwp[pnum]->ang.y;
				ChangeActM(1);
				rdTwinkleInit(tp);
				SetAllPlayersPosition(pos.x, pos.y, pos.z, ang);
			}

			break;
		}
		case 4i8:
		{
			SetFreeCameraMode(0);

			auto pnum = IsPlayerInSphere(350.0f, 100.0f, 550.0f, 36.0f) - 1;

			if (pnum >= 0)
			{
				NJS_VECTOR pos = playertwp[pnum]->pos;
				Angle ang = playertwp[pnum]->ang.y;
				ChangeActM(-1);
				rdTwinkleInit(tp);
				SetAllPlayersPosition(pos.x, pos.y, pos.z, ang);
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
		TARGET_DYNAMIC(Rd_Twinkle)(tp);
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
		TARGET_DYNAMIC(Rd_Ruin)(tp);
	}
}

static void __cdecl dispBgSnow_r(task* tp)
{
	TARGET_DYNAMIC(dispBgSnow)(tp);

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
		TARGET_DYNAMIC(dispBgHighway)(tp);
	}
}

static void __cdecl dispBgTwinkle_r(task* tp)
{
	TARGET_DYNAMIC(dispBgTwinkle)(tp);

	if (camera_twp && pRd_Master && SplitScreen::IsActive())
	{
		TwinkleMaskBlock(pRd_Master->twp);
	}
}

static void __cdecl Create_Mountain_Cloud()
{
	CreateElementalTask(2u, LEV_1, (TaskFuncPtr)0x601230); // load task into slot 1 instead of 0 (to not run before the camera)
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

	// Emerald Coast Bridge
	WriteData<2>((void*)0x501B66, 0x90ui8);
	WriteData<2>((void*)0x501B12, 0x90ui8);
	WriteData((uint8_t*)0x501C2F, 0xEBui8);

	// Windy Valley leaves
	WriteData((uint8_t*)0x4E4344, 0xEBui8);

	// Ice Cap breath generator
	WriteData((uint8_t*)0x4E91AE, (uint8_t)PLAYER_MAX);
	
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
	WriteData((intptr_t*)0x4F768B, 0); // BeachTexAnimWtAct3
	WriteData((intptr_t*)0x61E3DB, 0); // TwinkleTexAnim
	WriteData((intptr_t*)0x5E20DB, 0); // RuinTexAnimAct1
	WriteData((intptr_t*)0x5E214B, 0); // RuinTexAnimAct2
	WriteData((intptr_t*)0x5E229B, 0); // RuinTexAnimAct3
	WriteData((intptr_t*)0x63179A, 0); // SSTexAnim

	// Remove landtable collision chunk optimisation
	WriteData((uint8_t*)0x4E91C0, 0xC3ui8); // Ice Cap
	
	// In battle mode, boss become fighting arenas
	Rd_Chaos0_t   = new Trampoline(0x545E60, 0x545E66, Rd_Chaos0_r);
	Rd_Chaos2_t   = new Trampoline(0x54A700, 0x54A706, Rd_Chaos2_r);
	Rd_Chaos4_t   = new Trampoline(0x550A30, 0x550A36, Rd_Chaos4_r);
	Rd_Chaos6_t   = new Trampoline(0x557920, 0x557926, Rd_Chaos6_r);
	Rd_Bossegm1_t = new Trampoline(0x571850, 0x571856, Rd_Bossegm1_r);
	Rd_Bossegm2_t = new Trampoline(0x5758D0, 0x5758D6, Rd_Bossegm2_r);
	Rd_E101_t     = new Trampoline(0x566C00, 0x566C05, Rd_E101_r);
	Rd_E101_R_t   = new Trampoline(0x569040, 0x569047, Rd_E101_R_r);

	// Act swap fixes
	Rd_Beach_t = new Trampoline(0x4F6D60, 0x4F6D67, Rd_Beach_r);
	Rd_Windy_t = new Trampoline(0x4DDB30, 0x4DDB37, Rd_Windy_r);
	Rd_Mountain_t = new Trampoline(0x601550, 0x601558, Rd_Mountain_r);
	Rd_Twinkle_t = new Trampoline(0x61D150, 0x61D155, Rd_Twinkle_r);
	Rd_Ruin_t = new Trampoline(0x5E18B0, 0x5E18B5, Rd_Ruin_r);

	// Move landtable mask flag to display for multiplayer compatibility
	dispBgSnow_t = new Trampoline(0x4E9950, 0x4E9955, dispBgSnow_r);
	dispBgHighway_t = new Trampoline(0x610570, 0x610575, dispBgHighway_r);
	dispBgTwinkle_t = new Trampoline(0x61D1F0, 0x61D1F5, dispBgTwinkle_r);
}
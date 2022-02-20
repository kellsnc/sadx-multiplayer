#include "pch.h"
#include "levels.h"

/*

Level-related adjustements for multiplayer

*/

Trampoline* SetPlayerInitialPosition_t = nullptr;

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

// Put players side by side
void __cdecl SetPlayerInitialPosition_r(taskwk* twp)
{
	TARGET_DYNAMIC(SetPlayerInitialPosition)(twp);

	if (multiplayer::IsActive())
	{
		static const int dists[]
		{
			-5.0f,
			5.0f,
			-10.0f,
			10.0f
		};

		twp->pos.x += njCos(twp->ang.y + 0x4000) * dists[TASKWK_PLAYERID(twp)];
		twp->pos.z += njSin(twp->ang.y + 0x4000) * dists[TASKWK_PLAYERID(twp)];
	}
}

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
	if (ssActNumber == 0 && multiplayer::IsEnabled())
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
	if (ssActNumber == 1 && multiplayer::IsEnabled())
	{
		for (int i = 1; i < PLAYER_MAX; ++i)
		{
			if (playertwp && playertwp[i]->pos.y > 2250.0f)
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
	if (ssActNumber == 0 && multiplayer::IsEnabled())
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

	TARGET_DYNAMIC(Rd_Mountain)(tp);
}

void InitLevels()
{
	// Patch start positions
	SetPlayerInitialPosition_t = new Trampoline(0x414810, 0x414815, SetPlayerInitialPosition_r);

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

	// Windy Valley exec for hane, bigfloot, saku (static "exec" name in symbols)
	WriteData<2>((void*)0x4E1399, 0x90ui8);
	
	// Speed Highway Act 2 skybox
	WriteData((taskwk***)0x610765, &camera_twp);
	
	// In battle mode, boss become fighting arenas
	Rd_Chaos0_t   = new Trampoline(0x545E60, 0x545E66, Rd_Chaos0_r);
	Rd_Chaos2_t   = new Trampoline(0x54A700, 0x54A706, Rd_Chaos2_r);
	Rd_Chaos4_t   = new Trampoline(0x550A30, 0x550A36, Rd_Chaos4_r);
	Rd_Chaos6_t   = new Trampoline(0x557920, 0x557926, Rd_Chaos6_r);
	Rd_Bossegm1_t = new Trampoline(0x571850, 0x571856, Rd_Bossegm1_r);
	Rd_Bossegm2_t = new Trampoline(0x5758D0, 0x5758D6, Rd_Bossegm2_r);
	Rd_E101_t     = new Trampoline(0x566C00, 0x566C05, Rd_E101_r);
	Rd_E101_R_t   = new Trampoline(0x569040, 0x569047, Rd_E101_R_r);

	Rd_Beach_t = new Trampoline(0x4F6D60, 0x4F6D67, Rd_Beach_r);
	Rd_Windy_t = new Trampoline(0x4DDB30, 0x4DDB37, Rd_Windy_r);
	Rd_Mountain_t = new Trampoline(0x601550, 0x601558, Rd_Mountain_r);
}
#include "pch.h"
#include "config.h"
#include "camera.h"
#include "splitscreen.h"
#include "menu.h"
#include "hud_multi.h"
#include "patches.h"
#include "death.h"
#include "set.h"
#include "levels.h"
#include "race.h"
#include "emeraldhunt.h"
#include "hud_emerald.h"
#include "hud_itembox.h"
#include "sound.h"
#include "result.h"
#include "milesrace.h"
#include "fishing.h"
#include "testspawn.h"
#include "forcewk.h"
#include "gravity.h"
#include "fog.h"
#include "network.h"
#include "input.h"
#include "logic.h"
#include "collision.h"
#include "event.h"

const HelperFunctions* gHelperFunctions;

bool DreamcastConversionEnabled = false;

extern "C"
{
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		gHelperFunctions = &helperFunctions;
		config::read(path);

		DreamcastConversionEnabled = GetModuleHandle(L"DCMods_Main") != nullptr;

		InitLogic();
		InitSplitScreen();
		InitCamera();
		InitInputPatches();
		InitPatches();
		InitDeathPatches();
		InitPlayerPatches();
		InitMenu();
		MultiHudInit();
		InitSET();
		InitLevels();
		InitRace();
		InitEmeraldHunt();
		InitItemBoxHUD();
		InitSoundPatches();
		InitResult();
		TestSpawn();
		InitEmeraldRadar();
		InitMilesRace();
		InitFishing();
		InitForceWorkPatches();
		InitGravityPatches();
		InitFogPatches();
		initEvents();
		InitCollisionPatches();
	}

	__declspec(dllexport) void __cdecl OnExit()
	{
		network.Exit();
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}
#include "pch.h"
#include "config.h"
#include "camera.h"
#include "splitscreen.h"
#include "menu.h"
#include "set.h"
#include "race.h"
#include "emeraldhunt.h"
#include "sound.h"
#include "result.h"
#include "milesrace.h"
#include "fishing.h"
#include "testspawn.h"
#include "forcewk.h"
#include "gravity.h"
#include "fog.h"
#include "netplay.h"
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
		config.read(path);

		if (helperFunctions.Version >= 16)
		{
			DreamcastConversionEnabled = helperFunctions.Mods->find("sadx-dreamcast-conversion");
		}

		InitLogic();
		InitSplitScreen();
		InitCamera();
		InitInputPatches();
		InitPlayerPatches();
		InitMenu();
		InitSET();
		InitRace();
		InitEmeraldHunt();
		InitSoundPatches();
		InitResult();
		TestSpawn();
		InitMilesRace();
		InitFishing();
		InitForceWorkPatches();
		InitGravityPatches();
		InitFogPatches();
		initEvents();
		InitCollisionPatches();

		InitPatches();
	}

	__declspec(dllexport) void __cdecl OnExit()
	{
		netplay.Exit();

		FreePatches();
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}
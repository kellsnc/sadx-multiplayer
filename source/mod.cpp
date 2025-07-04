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
#include "game.h"
#include "collision.h"
#include "event.h"

const HelperFunctions* gHelperFunctions;

bool DreamcastConversionEnabled = false;
bool CharacterSelectEnabled = false;

static bool mod_initialized = false;

extern "C"
{
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		if (helperFunctions.Version < 16)
		{
			PrintDebug("[MULTI] Error: the Mod Loader API should be above version 16, please update the Mod Loader. Current API version: %i.\n", helperFunctions.Version);
			return;
		}

		gHelperFunctions = &helperFunctions;
		config.read(path);

		DreamcastConversionEnabled = helperFunctions.Mods->find("sadx-dreamcast-conversion") != nullptr;
		CharacterSelectEnabled = helperFunctions.Mods->find_by_name("Character Select Mod") != nullptr;

		InitMultiplayer();
		InitSplitScreen();
		InitGameManager();

		InitCamera();
		InitInputPatches();
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

		mod_initialized = true;
	}

	__declspec(dllexport) void __cdecl OnFrame()
	{
		if (mod_initialized)
		{
			ExecMultiplayer();
			ExecGameManager();

			ExecPatches();
		}
	}

	__declspec(dllexport) void __cdecl OnExit()
	{
		if (mod_initialized)
		{
			netplay.Exit();

			FreePatches();
		}
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}
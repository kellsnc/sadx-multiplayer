#include "pch.h"
#include <IniFile.hpp>
#include "camera.h"
#include "splitscreen.h"
#include "menu.h"
#include "hud_multi.h"
#include "patches.h"
#include "deathzones.h"
#include "set.h"
#include "levels.h"

const HelperFunctions* gHelperFunctions;

extern "C"
{
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		const IniFile* config = new IniFile(std::string(path) + "\\config.ini");
		gHelperFunctions = &helperFunctions;

		InitSplitScreen(config);
		InitCamera();
		InitPatches();
		InitDeathPatches();
		InitPlayerPatches();
		InitMenu(helperFunctions);
		MultiHudInit();
		InitSET();
		InitLevels();

		delete config;
	}

	//__declspec(dllexport) void __cdecl OnInitEnd()
	//{
	//	
	//}

	__declspec(dllexport) void __cdecl OnFrame()
	{
		UpdatePlayersInfo();
	}

	//__declspec(dllexport) void __cdecl OnInput()
	//{
	//	
	//}

	//__declspec(dllexport) void __cdecl OnControl()
	//{
	//	
	//}

	//__declspec(dllexport) void __cdecl OnRenderDeviceReset()
	//{
	//	
	//}

	//__declspec(dllexport) void __cdecl OnRenderDeviceLost()
	//{
	//	
	//}

	//__declspec(dllexport) void __cdecl OnRenderSceneStart()
	//{
	//	
	//}

	//__declspec(dllexport) void __cdecl OnRenderSceneEnd()
	//{
	//	
	//}

	//__declspec(dllexport) void __cdecl OnExit()
	//{
	//	
	//}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}
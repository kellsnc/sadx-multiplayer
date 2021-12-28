#include "pch.h"
#include "camera.h"
#include "splitscreen.h"
#include "menu.h"
#include "multihud.h"
#include "patches.h"
#include "deathzones.h"
#include "set.h"
#include "levels.h"

const HelperFunctions* gHelperFunctions;

extern "C"
{
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		gHelperFunctions = &helperFunctions;
		InitSplitScreen();
		InitCamera();
		InitPatches();
		InitDeathPatches();
		InitMultiplayer();
		InitMenu(helperFunctions);
		MultiHudInit();
		InitSET();
		InitLevels();
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
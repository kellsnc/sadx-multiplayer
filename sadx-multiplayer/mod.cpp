#include "pch.h"
#include "camera.h"
#include "splitscreen.h"
#include "menu.h"
#include "multihud.h"
#include "patches/patches.h"

const HelperFunctions* gHelperFunctions;
unsigned int player_count = 2;

bool IsMultiplayerEnabled()
{
	return (IsIngame() || IsGamePaused()) && player_count != 0;
}

extern "C"
{
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		gHelperFunctions = &helperFunctions;
		InitSplitScreen();
		InitCamera();
		InitPatches();
		initPlayerHack();
		Menu_Init(helperFunctions);
		MultiHudInit();
	}

	//__declspec(dllexport) void __cdecl OnInitEnd()
	//{
	//	
	//}

	__declspec(dllexport) void __cdecl OnFrame()
	{
		RingsLives_OnFrames();
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
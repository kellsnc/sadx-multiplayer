#include "pch.h"
#include <SADXModLoader.h>
#include "splitscreen.h"
#include "camera.h"

unsigned int player_count = 2;

extern "C"
{
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		InitSplitScreen();
		InitCamera();
	}

	//__declspec(dllexport) void __cdecl OnInitEnd()
	//{
	//	
	//}

	//__declspec(dllexport) void __cdecl OnFrame()
	//{
	//	
	//}

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
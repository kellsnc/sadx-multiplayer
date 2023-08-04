#include "pch.h"
#include <d3d8types.h>
#include <d3d8.h>
#include <SADXModLoader.h>
#include <Trampoline.h>
#include "config.h"
#include "hud_multi.h"
#include "d3d8vars.h"
#include "drawqueue.h"
#include "camera.h"
#include "splitscreen.h"

/*

Splitscreen
Series of hacks to make splitscreen possible
- Prevents display subs from running in the exec subs
- Draw all display subs for each viewport; matrix and projection are calculated in camera.cpp

*/

FunctionHook<void> SpLoopOnlyDisplay_hook(0x456CD0);
FunctionHook<void> DisplayTask_hook(0x40B540);
FunctionHook<void> LoopTask_hook(0x40B170);
Trampoline* njDrawQuadTextureEx_t = nullptr;

void __cdecl DisplayTask_r();

static bool configSplitScreenEnabled = true;

namespace SplitScreen
{
	unsigned int numScreen = 0;
	signed int numViewPort, backupNumViewPort = -1;
	bool enabled = false;

	const ScreenRatio ScreenRatio2[]
	{
		{ 0.0f, 0.0f, 0.5f, 1.0f },
		{ 0.5f, 0.0f, 0.5f, 1.0f }
	};

	const ScreenRatio ScreenRatio3[]
	{
		{ 0.0f, 0.0f, 1.0f, 0.5f },
		{ 0.0f, 0.5f, 0.5f, 0.5f },
		{ 0.5f, 0.5f, 0.5f, 0.5f }
	};

	const ScreenRatio ScreenRatio4[]
	{
		{ 0.0f, 0.0f, 0.5f, 0.5f },
		{ 0.5f, 0.0f, 0.5f, 0.5f },
		{ 0.0f, 0.5f, 0.5f, 0.5f },
		{ 0.5f, 0.5f, 0.5f, 0.5f }
	};

	const ScreenRatio* ScreenRatios[]
	{
		ScreenRatio2,
		ScreenRatio2,
		ScreenRatio3,
		ScreenRatio4
	};

	LevelAndActIDs bannedLevels[] =
	{
		LevelAndActIDs_Casinopolis3,
		LevelAndActIDs_Casinopolis4,
	};

	const ScreenRatio* GetScreenRatio(int num)
	{
		if (num < 0 || num >= PLAYER_MAX)
		{
			return nullptr;
		}

		int screenid = 0;
		int screencount = 0;

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (IsScreenEnabled(i))
			{
				if (i < num)
					++screenid;
				++screencount;
			}
		}

		return &ScreenRatios[screencount - 1][screenid];
	}

	void SaveViewPort()
	{
		backupNumViewPort = numViewPort;
	}

	void RestoreViewPort()
	{
		ChangeViewPort(backupNumViewPort);
	}

	signed int GetCurrentViewPortNum()
	{
		return numViewPort;
	}

	bool isBannedLevel()
	{
#ifdef MULTI_TEST
		for (uint16_t i = 0; i < LengthOfArray(bannedLevels); i++)
		{
			if (GetStageNumber() == bannedLevels[i])
			{
				return true;
			}
		}
#endif
		return false;
	}

	static inline bool IsGameModeValid()
	{
		return ssGameMode < MD_GAME_END || ssGameMode > MD_GAME_END2;
	}

	bool IsActive()
	{
		return enabled && IsGameModeValid() && !canselEvent && cameraSystemWork.G_scCameraMode != CAMMD_CHAOS_STINIT && !isBannedLevel();
	}

	bool IsEnabled()
	{
		return enabled;
	}

	void Enable()
	{
		if (configSplitScreenEnabled == true)
		{
			enabled = true;
		}
	}

	void Disable()
	{
		enabled = false;
	}

	unsigned int GetCurrentScreenNum()
	{
		return IsActive() ? numScreen : 0;
	}

	bool IsScreenEnabled(int num)
	{
		if (num == 0)
		{
			return true;
		}

		if (!IsActive() || num < 0)
		{
			return false;
		}

		if (multiplayer::IsEnabled())
		{
			return num < multiplayer::GetPlayerCount();
		}
		else
		{
			return num < GetPlayerCount() && num < 4;
		}
	}

	// Change the viewport (-1 is whole screen)
	bool ChangeViewPort(int num)
	{
		if (!IsActive())
		{
			num = -1;
		}

		if (num == numViewPort)
		{
			return false;
		}

		if (num == -1)
		{
			// Reset
			Direct3D_ViewPort = { 0, 0, (unsigned long)HorizontalResolution, (unsigned long)VerticalResolution, 0.0f, 1.0f };
			Direct3D_Device->SetViewport(&Direct3D_ViewPort);
			numViewPort = -1;
			___njFogEnable();
			return true;
		}

		if (num < 0 || num > PLAYER_MAX)
		{
			return false;
		}

		auto ratio = GetScreenRatio(num);

		if (!ratio)
		{
			return false;
		}

		Direct3D_ViewPort.X = static_cast<DWORD>(ratio->x * static_cast<float>(HorizontalResolution));
		Direct3D_ViewPort.Y = static_cast<DWORD>(ratio->y * static_cast<float>(VerticalResolution));
		Direct3D_ViewPort.Width = static_cast<DWORD>(ratio->w * static_cast<float>(HorizontalResolution));
		Direct3D_ViewPort.Height = static_cast<DWORD>(ratio->h * static_cast<float>(VerticalResolution));
		Direct3D_Device->SetViewport(&Direct3D_ViewPort);

		_nj_screen_.w = ratio->w * static_cast<float>(HorizontalResolution);
		_nj_screen_.h = ratio->h * static_cast<float>(VerticalResolution);
		_nj_screen_.cx = _nj_screen_.w / 2.0f;
		_nj_screen_.cy = _nj_screen_.h / 2.0f;

		View.ang = 0; // Invalidate FOV to force recalculation of _nj_screen_.dest
		njSetPerspective(ds_GetPerspective_m(num));

		numViewPort = num;
		___njFogEnable();

		return true;
	}
}

void __cdecl SpLoopOnlyDisplay_r()
{
	if (SplitScreen::IsActive())
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (SplitScreen::IsScreenEnabled(i))
			{
				SplitScreen::ChangeViewPort(i);
				ApplyMultiCamera(i);
				SpLoopOnlyDisplay_hook.Original();
			}
		}

		SplitScreen::ChangeViewPort(-1);
	}
	else
	{
		SpLoopOnlyDisplay_hook.Original();
	}
}

// Draw every task in subscreen
static void DrawScreen(int num)
{
	if (SplitScreen::IsScreenEnabled(num) && SplitScreen::ChangeViewPort(num))
	{
		SplitScreen::numScreen = num;
		DisplayTask_hook.Original();
		DisplayMultiHud(num);
	}
}

// DisplayTask run every task displays
void __cdecl DisplayTask_r()
{
	if (SplitScreen::IsActive())
	{
		// If multiplayer is enabled, split screen:

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			DrawScreen(i);
		}

		SplitScreen::ChangeViewPort(-1);
	}
	else
	{
		// Otherwise, normal behaviour:

		DisplayTask_hook.Original();
	}
}

// LoopTask run every task execs
void __cdecl LoopTask_r()
{
	if (SplitScreen::IsActive())
	{
		// When unpaused run logic (which also runs display) for first screen, then only run display for the other screens.

		SplitScreen::ChangeViewPort(0);
		SplitScreen::numScreen = 0;
		LoopTask_hook.Original();
		DisplayMultiHud(0);

		for (int i = 1; i < PLAYER_MAX; ++i)
		{
			DrawScreen(i);
		}

		SplitScreen::numScreen = 0;
		SplitScreen::ChangeViewPort(-1);
	}
	else
	{
		// Otherwise, normal behaviour:

		LoopTask_hook.Original();
	}
}

// Draw into viewport with scaling
void __cdecl njDrawQuadTextureEx_r(NJS_QUAD_TEXTURE_EX* quad)
{
	if (SplitScreen::IsActive() && SplitScreen::GetCurrentViewPortNum() >= 0)
	{
		auto ratio = SplitScreen::GetScreenRatio(SplitScreen::numScreen);

		quad->x = quad->x * ratio->w + HorizontalResolution * ratio->x;
		quad->y = quad->y * ratio->h + VerticalResolution * ratio->y;
		quad->vx1 *= ratio->w;
		quad->vy1 *= ratio->h;
		quad->vx2 *= ratio->w;
		quad->vy2 *= ratio->h;
	}

	TARGET_DYNAMIC(njDrawQuadTextureEx)(quad);
}

void InitSplitScreen()
{
	if (config.mSplitScreen == true)
	{
		SpLoopOnlyDisplay_hook.Hook(SpLoopOnlyDisplay_r);
		DisplayTask_hook.Hook(DisplayTask_r);
		LoopTask_hook.Hook(LoopTask_r);
		njDrawQuadTextureEx_t = new Trampoline(0x77DE10, 0x77DE18, njDrawQuadTextureEx_r);

		DrawQueue_Init();
	}
}
#include "pch.h"
#include <d3d8types.h>
#include <d3d8.h>
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "config.h"
#include "hud_multi.h"
#include "d3d8vars.h"
#include "camera.h"
#include "splitscreen.h"

// Splitscreen system
// This works by using D3D viewports and running the task display subroutines for each screen
// Many tasks that don't use the display subroutine properly still need patching

FastFunctionHook<void> SpLoopOnlyDisplay_hook(0x456CD0);
FastFunctionHook<void> DisplayTask_hook(0x40B540);
FastFunctionHook<void> LoopTask_hook(0x40B170);
FastFunctionHook<void, NJS_QUAD_TEXTURE_EX*> njDrawQuadTextureEx_hook(0x77DE10);

void __cdecl DisplayTask_r();

namespace SplitScreen
{
	unsigned int numScreen = 0;
	signed int numViewPort = -1;
	signed int backupNumViewPort = -1;
	bool enabled = false;

	const ScreenRatio ScreenRatio2H[]
	{
		{ 0.0f, 0.0f, 1.0f, 0.5f },
		{ 0.0f, 0.5f, 1.0f, 0.5f }
	};

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

	// Get screen ratio of specific player screen
	// Multiply each ratio with the horizontal/vertical resolutions to get the actual screen coordinates
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

	// Get the active viewport; -1 is whole screen, the rest maps to player ID
	signed int GetCurrentViewPortNum()
	{
		return numViewPort;
	}

	// Get the active player screen, maps to player ID.
	unsigned int GetCurrentScreenNum()
	{
		return numScreen;
	}

	// Backup current viewport to restore later
	void SaveViewPort()
	{
		backupNumViewPort = numViewPort;
	}

	// Restore backed up viewport
	void RestoreViewPort()
	{
		ChangeViewPort(backupNumViewPort);
	}

	// If splitscreen is currently running.
	// Todo: this should be get/set "active" member variable
	bool IsActive()
	{
		return enabled && (ssGameMode < MD_GAME_END || ssGameMode > MD_GAME_END2) && !canselEvent && cameraSystemWork.G_scCameraMode != CAMMD_CHAOS_STINIT;
	}

	// If splitscreen is enabled (but not necessarily running)
	bool IsEnabled()
	{
		return enabled;
	}

	// Enable splitscreen
	void Enable()
	{
		if (config.mSplitScreen == true)
		{
			enabled = true;
		}
	}

	// Disable splitscreen
	void Disable()
	{
		enabled = false;
	}

	// Check if player screen is active
	bool IsScreenEnabled(int num)
	{
		// P1 is always running
		if (num == 0)
		{
			return true;
		}

		// Rest should return false if splitscreen is disabled
		if (!IsActive())
		{
			return false;
		}

		// Check if the player exists
		if (num < 0 || num >= PLAYER_MAX || !playertwp[num])
		{
			return false;
		}

		if (multiplayer::IsEnabled())
		{
			return num < multiplayer::GetPlayerCount(); // In a multiplayer context, make sure it doesn't exceed the number of players
		}
		else
		{
			return true;
		}
	}

	// Change the viewport (-1 is whole screen, rest maps to player ID)
	bool ChangeViewPort(int num)
	{
		// If splitscreen is not active, force reset
		if (!IsActive())
		{
			num = -1;
		}

		// Optimization: if the viewport hasn't changed, no need to continue
		if (num == numViewPort)
		{
			return false;
		}

		// Reset if -1 was passed (todo: enum)
		if (num == -1)
		{
			Direct3D_ViewPort = { 0, 0, (unsigned long)HorizontalResolution, (unsigned long)VerticalResolution, 0.0f, 1.0f };
			Direct3D_Device->SetViewport(&Direct3D_ViewPort);
			numViewPort = -1;
			___njFogEnable();
			numScreen = 0;
			return true;
		}

		// Check if passed number is valid
		if (num < 0 || num > PLAYER_MAX)
		{
			return false;
		}

		// Apply new viewport
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

		numScreen = num;
		return true;
	}
}

// Run sprites for each screen
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

static void DispTask_m(int level)
{
	if (level == 8)
	{
		level = 7;
	}

	task* current = btp[level];

	if (current)
	{
		do
		{
			if (current->exec != FreeTask && current->exec != DestroyTask)
			{
				if (current->disp)
					current->disp(current);
				if (current->ctp)
					DispChildrenTask(current);
			}
			current = current->next;
		} while (current);
	}
}

static void DisplayTask_m(int num)
{
	if (SplitScreen::IsScreenEnabled(num))
	{
		SplitScreen::ChangeViewPort(num);
		ResetMaterial();
		DispTask_m(8);
		for (int i = 0; i < 7; ++i)
		{
			DispTask_m(i);
		}

		DisplayMultiHud(num);
	}
}

// Run task display subroutines for each screen during pause menu
void __cdecl DisplayTask_r()
{
	if (SplitScreen::IsActive())
	{
		// If split screen is active, draw each screen:

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			DisplayTask_m(i);
		}

		SplitScreen::ChangeViewPort(-1);
	}
	else
	{
		// Otherwise, normal behaviour:

		DisplayTask_hook.Original();
	}
}

// Run task display subroutines for each screen
void __cdecl LoopTask_r()
{
	if (SplitScreen::IsActive())
	{
		// When unpaused run logic (which also runs display) for first screen, then only run display for the other screens.

		SplitScreen::ChangeViewPort(0);
		LoopTask_hook.Original();
		DisplayMultiHud(0);

		for (int i = 1; i < PLAYER_MAX; ++i)
		{
			DisplayTask_m(i);
		}

		SplitScreen::ChangeViewPort(-1);
	}
	else
	{
		// Otherwise, normal behaviour:

		LoopTask_hook.Original();
	}
}

// Since uiscale is not compatible with viewports, we scale UI manually for now
// Todo: improve scaling
void __cdecl njDrawQuadTextureEx_r(NJS_QUAD_TEXTURE_EX* quad)
{
	if (SplitScreen::IsActive() && SplitScreen::GetCurrentViewPortNum() != -1)
	{
		auto ratio = SplitScreen::GetScreenRatio(SplitScreen::GetCurrentScreenNum());

		quad->x = quad->x * ratio->w + HorizontalResolution * ratio->x;
		quad->y = quad->y * ratio->h + VerticalResolution * ratio->y;
		quad->vx1 *= ratio->w;
		quad->vy1 *= ratio->h;
		quad->vx2 *= ratio->w;
		quad->vy2 *= ratio->h;
	}

	njDrawQuadTextureEx_hook.Original(quad);
}

void InitSplitScreen()
{
	if (config.mSplitScreen == true)
	{
		SpLoopOnlyDisplay_hook.Hook(SpLoopOnlyDisplay_r);
		DisplayTask_hook.Hook(DisplayTask_r);
		LoopTask_hook.Hook(LoopTask_r);
		njDrawQuadTextureEx_hook.Hook(njDrawQuadTextureEx_r);

		if (config.mHorizontalLayout == true)
		{
			SplitScreen::ScreenRatios[0] = SplitScreen::ScreenRatio2H;
			SplitScreen::ScreenRatios[1] = SplitScreen::ScreenRatio2H;
		}
	}
}
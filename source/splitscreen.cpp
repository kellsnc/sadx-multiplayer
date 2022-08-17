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

Trampoline* SpLoopOnlyDisplay_t   = nullptr;
Trampoline* DisplayTask_t         = nullptr;
Trampoline* LoopTask_t            = nullptr;
Trampoline* njDrawQuadTextureEx_t = nullptr;

void __cdecl DisplayTask_r();

static bool configSplitScreenEnabled = true;

namespace SplitScreen
{
    unsigned int numScreen = 0;
    signed int numViewPort, backupNumViewPort = -1;

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
        ScreenRatio3,
        ScreenRatio4
    };

    const ScreenRatio* GetScreenRatio(int num)
    {
        return &ScreenRatios[multiplayer::GetPlayerCount() - 2][num];
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

    bool IsActive()
    {
        return configSplitScreenEnabled == true && multiplayer::IsActive() && !EV_CheckCansel();
    }

    unsigned int GetCurrentScreenNum()
    {
        return IsActive() ? numScreen : 0;
    }

    bool IsScreenEnabled(int num)
    {
        return num == 0 ? true : IsActive() && playertp[num] != nullptr;
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

        if (num < 0 || num > (int)multiplayer::GetPlayerCount() || num > PLAYER_MAX)
        {
            return false;
        }

        auto ratio = GetScreenRatio(num);

        Direct3D_ViewPort.X = static_cast<DWORD>(ratio->x * static_cast<float>(HorizontalResolution));
        Direct3D_ViewPort.Y = static_cast<DWORD>(ratio->y * static_cast<float>(VerticalResolution));
        Direct3D_ViewPort.Width = static_cast<DWORD>(ratio->w * static_cast<float>(HorizontalResolution));
        Direct3D_ViewPort.Height = static_cast<DWORD>(ratio->h * static_cast<float>(VerticalResolution));
        Direct3D_Device->SetViewport(&Direct3D_ViewPort);

        numViewPort = num;
        ___njFogEnable();

        return true;
    }

    bool IsVisible(int num, NJS_POINT3* pos, Float radius)
    {
        return true;
    }

    bool IsVisible(NJS_POINT3* pos, Float radius)
    {
        if (!IsActive())
        {
            return dsCheckViewV(pos, radius) != FALSE;
        }

        for (int i = 0; i < multiplayer::GetPlayerCount(); ++i)
        {
            if (IsVisible(i, pos, radius))
            {
                return true;
            }
        }

        return false;
    }
}

void __cdecl SpLoopOnlyDisplay_r()
{
    if (SplitScreen::IsActive())
    {
        for (unsigned int i = 0ui32; i < multiplayer::GetPlayerCount(); ++i)
        {
            if (SplitScreen::IsScreenEnabled(i))
            {
                SplitScreen::ChangeViewPort(i);
                ApplyMultiCamera(i);
                TARGET_DYNAMIC(SpLoopOnlyDisplay)();
            }
        }

        SplitScreen::ChangeViewPort(-1);
    }
    else
    {
        TARGET_DYNAMIC(SpLoopOnlyDisplay)();
    }
}

// Call all task displays
static void DisplayTask_o()
{
    ResetMaterial();
    DispTask(8);
    for (int i = 0; i < 7; ++i)
    {
        DispTask(i);
    }
}

// Draw every task in subscreen
static void DrawScreen(int num)
{
    if (SplitScreen::ChangeViewPort(num))
    {
        if (SplitScreen::IsScreenEnabled(num))
        {
            // If player exists, draw all objects into viewport:

            SplitScreen::numScreen = num;
            DisplayTask_o();
            DisplayMultiHud(num);
        }
        else
        {
            // If player is not there, draw a black screen and skip:

            DrawWaitScreen(num);
        }
    }
}

// DisplayTask run every task displays
void __cdecl DisplayTask_r()
{
    if (SplitScreen::IsActive())
    {
        // If multiplayer is enabled, split screen:

        for (unsigned int i = 0ui32; i < multiplayer::GetPlayerCount(); ++i)
        {
            DrawScreen(i);
        }

        SplitScreen::ChangeViewPort(-1);
    }
    else
    {
        // Otherwise, normal behaviour:

        DisplayTask_o();
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
        TARGET_DYNAMIC(LoopTask)();
        DisplayMultiHud(0);

        for (unsigned int i = 1ui32; i < multiplayer::GetPlayerCount(); ++i)
        {
            DrawScreen(i);
        }

        SplitScreen::ChangeViewPort(-1);
    }
    else
    {
        // Otherwise, normal behaviour:

        TARGET_DYNAMIC(LoopTask)();
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

        TARGET_DYNAMIC(njDrawQuadTextureEx)(quad);
    }
    else
    {
        TARGET_DYNAMIC(njDrawQuadTextureEx)(quad);
    }
}

void InitSplitScreen()
{
    if (config::splitScreenEnabled == true)
    {
        LoopTask_t = new Trampoline(0x40B170, 0x40B178, LoopTask_r);
        WriteCall((void*)((int)(LoopTask_t->Target()) + 3), RunObjectIndex); // Repair LoopTask_t
        
        WriteJump((void*)0x40B540, DisplayTask_r);
        
        SpLoopOnlyDisplay_t = new Trampoline(0x456CD0, 0x456CD9, SpLoopOnlyDisplay_r);
        WriteCall((void*)((int)(SpLoopOnlyDisplay_t->Target()) + 4), ___njFogDisable); // Repair SpLoopOnlyDisplay_t

        njDrawQuadTextureEx_t = new Trampoline(0x77DE10, 0x77DE18, njDrawQuadTextureEx_r);

        DrawQueue_Init();
    }
}
#include "pch.h"
#include <SADXModLoader.h>
#include <Trampoline.h>
#include <d3d8types.h>
#include <d3d8.h>
#include "mod.h"
#include "splitscreen.h"

VoidFunc(DisplayTask, 0x40B540);
DataPointer(IDirect3DDevice8*, Direct3D_Device, 0x03D128B0);
DataPointer(D3DVIEWPORT8, Direct3D_ViewPort, 0x03D12780);

unsigned int current_screen;

Trampoline* DisplayTask_t = nullptr;
Trampoline* LoopTask_t = nullptr;

void ChangeViewPort(int x, int y, int w, int h)
{
    Direct3D_ViewPort = { (unsigned long)x, (unsigned long)y, (unsigned long)w, (unsigned long)h, 0.0f, 1.0f };
    Direct3D_Device->SetViewport(&Direct3D_ViewPort);
}

void DrawScreen(int num)
{
    if (playertp[num])
    {
        // If player exists, draw all objects into viewport:

        current_screen = num;
        TARGET_DYNAMIC(DisplayTask)();
    }
    else
    {
        // If player is not there, draw a black screen and skip:

        DrawRect_DrawNowMaybe(0, 0, HorizontalResolution, VerticalResolution, 1000000.0f, 0xFF000000);
    }
}

void DrawTwoScreens()
{
    ChangeViewPort(0, 0, HorizontalResolution / 2, VerticalResolution);
    DrawScreen(0);

    ChangeViewPort(HorizontalResolution / 2, 0, HorizontalResolution / 2, VerticalResolution);
    DrawScreen(1);
}

void DrawFourScreens()
{
    int half_w = HorizontalResolution / 2;
    int half_h = VerticalResolution / 2;

    ChangeViewPort(0, 0, half_w, half_h);
    DrawScreen(0);

    ChangeViewPort(half_w, 0, half_w, half_h);
    DrawScreen(1);

    ChangeViewPort(0, half_h, half_w, half_h);
    DrawScreen(2);

    ChangeViewPort(half_w, half_h, half_w, half_h);
    DrawScreen(3);
}

void __cdecl DisplayTask_r()
{
    if (IsMultiplayerEnabled())
    {
        // If multiplayer is enabled, split screen:

        D3DVIEWPORT8 orig;
        Direct3D_Device->GetViewport(&orig);

        if (player_count <= 2)
        {
            DrawTwoScreens();
        }
        else
        {
            DrawFourScreens();
        }

        Direct3D_ViewPort = orig;
        Direct3D_Device->SetViewport(&orig);
    }
    else
    {
        // Otherwise, normal behaviour:

        TARGET_DYNAMIC(DisplayTask)();
    }
}

void __cdecl LoopTask_r()
{
    if (IsMultiplayerEnabled())
    {
        // If multiplayer is enabled, run only logic and display everything afterwards:

        int backup = MissedFrames;

        MissedFrames = 1; // Prevent display subs from running
        TARGET_DYNAMIC(LoopTask)();
        MissedFrames = backup;

        DisplayTask();
    }
    else
    {
        // Otherwise, normal behaviour:

        TARGET_DYNAMIC(LoopTask)();
    }
}

void InitSplitScreen()
{
    LoopTask_t = new Trampoline(0x40B170, 0x40B178, LoopTask_r);
    WriteCall((void*)((int)(LoopTask_t->Target()) + 3), RunObjectIndex); // Repair LoopTask_t
    DisplayTask_t = new Trampoline(0x40B540, 0x40B546, DisplayTask_r);
    WriteCall((void*)((int)(LoopTask_t->Target()) + 3), (void*)0x40B0C0); // Repair DisplayTask_t
}
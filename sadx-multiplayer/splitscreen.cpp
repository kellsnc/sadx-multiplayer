#include "pch.h"
#include <SADXModLoader.h>
#include <Trampoline.h>
#include <d3d8types.h>
#include <d3d8.h>

#define TARGET_DYNAMIC(name) ((decltype(name##_r)*)name##_t->Target())

VoidFunc(DisplayTask, 0x40B540);
DataPointer(IDirect3DDevice8*, Direct3D_Device, 0x03D128B0);
DataPointer(D3DVIEWPORT8, Direct3D_ViewPort, 0x03D12780);

static int player_count = 2;

Trampoline* DisplayTask_t = nullptr;
Trampoline* LoopTask_t = nullptr;

void __cdecl DisplayTask_r();

void ChangeViewPort(int x, int y, int w, int h)
{
    Direct3D_ViewPort = { (unsigned long)x, (unsigned long)y, (unsigned long)w, (unsigned long)h, 0.0f, 1.0f };
    Direct3D_Device->SetViewport(&Direct3D_ViewPort);
}

void DrawTwoScreens()
{
    ChangeViewPort(0, 0, HorizontalResolution / 2, VerticalResolution);
    
    TARGET_DYNAMIC(DisplayTask)();

    ChangeViewPort(HorizontalResolution / 2, 0, HorizontalResolution / 2, VerticalResolution);

    // Temporary second camera pos;
    njPushMatrix(_nj_unit_matrix_);
    njTranslate(0, 20, 50, 0); 
    njRotateX(0, 0x8000);
    njRotateY(0, 0x4000);
    TARGET_DYNAMIC(DisplayTask)();
    njPopMatrixEx();
}

void __cdecl DisplayTask_r()
{
    if (player_count > 0)
    {
        // If multiplayer is enabled, split screen:

        D3DVIEWPORT8 orig;
        Direct3D_Device->GetViewport(&orig);

        DrawTwoScreens();

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
    if (player_count > 0)
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

void SplitScreen_Init()
{
    LoopTask_t = new Trampoline(0x40B170, 0x40B178, LoopTask_r);
    WriteCall((void*)((int)(LoopTask_t->Target()) + 3), RunObjectIndex); // Repair LoopTask_t
    DisplayTask_t = new Trampoline(0x40B540, 0x40B546, DisplayTask_r);
    WriteCall((void*)((int)(LoopTask_t->Target()) + 3), (void*)0x40B0C0); // Repair DisplayTask_t
}
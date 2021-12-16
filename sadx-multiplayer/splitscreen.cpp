#include "pch.h"
#include <SADXModLoader.h>
#include <Trampoline.h>
#include <d3d8types.h>
#include <d3d8.h>

#define TARGET_DYNAMIC(name) ((decltype(name##_r)*)name##_t->Target())
DataPointer(IDirect3DDevice8*, Direct3D_Device, 0x03D128B0);
static int player_count = 2;

Trampoline* DisplayAllObjects_t = nullptr;
Trampoline* RunAllObjects_t = nullptr;

void __cdecl DisplayAllObjects_r();

void DrawTwoScreens()
{
    D3DVIEWPORT8 leftscreen = { 0, 0, HorizontalResolution / 2, VerticalResolution, 0.0f, 1.0f };
    D3DVIEWPORT8 rightscreen = { HorizontalResolution / 2, 0, HorizontalResolution / 2, VerticalResolution, 0.0f, 1.0f };

    Direct3D_Device->SetViewport(&leftscreen);

    TARGET_DYNAMIC(DisplayAllObjects)();

    Direct3D_Device->SetViewport(&rightscreen);

    // Temporary second camera pos;
    njPushMatrix(_nj_unit_matrix_);
    njTranslate(0, 20, 50, 0); 
    njRotateX(0, 0x8000);
    njRotateY(0, 0x4000);
    TARGET_DYNAMIC(DisplayAllObjects)();
    njPopMatrixEx();
}

void __cdecl DisplayAllObjects_r()
{
    if (player_count > 0)
    {
        D3DVIEWPORT8 orig;
        Direct3D_Device->GetViewport(&orig);

        DrawTwoScreens();

        Direct3D_Device->SetViewport(&orig);
    }
    else
    {
        // Otherwise, normal behaviour:

        TARGET_DYNAMIC(DisplayAllObjects)();
    }
}

void __cdecl RunAllObjects_r()
{
    if (player_count > 0)
    {
        // If multiplayer is enabled, run only logic and display everything afterwards:

        int backup = MissedFrames;

        MissedFrames = 1; // Prevent display subs from running
        TARGET_DYNAMIC(RunAllObjects)();
        MissedFrames = backup;

        DisplayAllObjects();
    }
    else
    {
        // Otherwise, normal behaviour:

        TARGET_DYNAMIC(RunAllObjects)();
    }
}

void SplitScreen_Init() {
    DisplayAllObjects_t = new Trampoline((int)DisplayAllObjects, (int)DisplayAllObjects + 0x6, DisplayAllObjects_r);
    RunAllObjects_t = new Trampoline((int)RunAllObjects, (int)RunAllObjects + 0x8, RunAllObjects_r);
    WriteCall((void*)((int)(RunAllObjects_t->Target()) + 3), RunObjectIndex); // Repair RunAllObjects_t
}
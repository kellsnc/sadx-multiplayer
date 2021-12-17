#include "pch.h"


DataPointer(IDirect3DDevice8*, Direct3D_Device, 0x03D128B0);
DataPointer(D3DVIEWPORT8, Direct3D_ViewPort, 0x03D12780);

unsigned int numScreen;
signed int numViewPort;

Trampoline* DisplayTask_t = nullptr;
Trampoline* LoopTask_t = nullptr;
Trampoline* late_exec_t = nullptr;
Trampoline* late_setOdr_t = nullptr;

struct ScreenRatio
{
    float x, y, w, h;
};

ScreenRatio ScreenRatio2[]
{
    { 0.0f, 0.0f, 0.5f, 1.0f },
    { 0.5f, 0.0f, 0.5f, 1.0f }
};

ScreenRatio ScreenRatio4[]
{
    { 0.0f, 0.0f, 0.5f, 0.5f },
    { 0.5f, 0.0f, 0.5f, 0.5f },
    { 0.0f, 0.5f, 0.5f, 0.5f },
    { 0.5f, 0.5f, 0.5f, 0.5f }
};

// Change the viewport
bool ChangeViewPort(int num)
{
    if (num > player_count || num > PLAYER_MAX)
    {
        return false;
    }

    if (num == numViewPort)
    {
        return true;
    }

    auto& ratio = player_count <= 2 ? ScreenRatio2[num] : ScreenRatio4[num];

    Direct3D_ViewPort.X = ratio.x * HorizontalResolution;
    Direct3D_ViewPort.Y = ratio.y * VerticalResolution;
    Direct3D_ViewPort.Width = ratio.w * HorizontalResolution;
    Direct3D_ViewPort.Height = ratio.h * VerticalResolution;
    Direct3D_Device->SetViewport(&Direct3D_ViewPort);
    
    numViewPort = num;
    return true;
}

// Set full screen viewport
void ResetViewPort()
{
    Direct3D_ViewPort = { 0, 0, (unsigned long)HorizontalResolution, (unsigned long)VerticalResolution, 0.0f, 1.0f };
    Direct3D_Device->SetViewport(&Direct3D_ViewPort);
    numViewPort = -1;
}

// Adapt viewport to queued draw calls (late_exec draw all queued entries)
void __cdecl late_exec_r()
{
    // Draw for each screen
    for (int i = 0; i < (player_count <= 2 ? 2 : 4); ++i)
    {
        if (playertp[i] && ChangeViewPort(i))
        {
            TARGET_DYNAMIC(late_exec)();
        }
    }

    ResetViewPort();
}

// Only queue draw calls once (late_setOdr allocates a queue entry)
void* __cdecl late_setOdr_original(__int16 sz, int odr, int no, int flgs)
{
    const auto late_setOdr_ptr = late_setOdr_t->Target();

    void* result;

    __asm
    {
        push dword ptr[flgs]
        push dword ptr[no]
        push[odr]
        movzx ax, [sz]
        call late_setOdr_ptr
        add esp, 12
        mov result, eax
    }

    return result;
}

void* __cdecl late_setOdr_r(__int16 sz, int odr, int no, int flgs)
{
    return numScreen == 0 ? late_setOdr_original(sz, odr, no, flgs) : nullptr;
}

static void __declspec(naked) late_setOdr_asm()
{
    __asm
    {
        push[esp + 0Ch]
        push[esp + 0Ch]
        push[esp + 0Ch]
        movzx eax, ax
        push eax
        call late_setOdr_r
        add esp, 16
        retn
    }
}

// Draw every task in subscreen
void DrawScreen(int num)
{
    if (ChangeViewPort(num))
    {
        if (playertp[num])
        {
            // If player exists, draw all objects into viewport:

            numScreen = num;
            TARGET_DYNAMIC(DisplayTask)(); // call all object display subs
        }
        else
        {
            // If player is not there, draw a black screen and skip:

            ds_DrawBoxFill2D(0, 0, HorizontalResolution, VerticalResolution, 1000000.0f, 0xFF000000);
        }
    }
}

// DisplayTask run every task displays
void __cdecl DisplayTask_r()
{
    if (IsMultiplayerEnabled())
    {
        // If multiplayer is enabled, split screen:

        for (int i = (player_count <= 2 ? 2 : 4); i > 0; --i)
        {
            DrawScreen(i - 1);
        }

        ResetViewPort();
    }
    else
    {
        // Otherwise, normal behaviour:

        TARGET_DYNAMIC(DisplayTask)();
    }
}

// LoopTask run every task execs
void __cdecl LoopTask_r()
{
    if (IsMultiplayerEnabled())
    {
        // If multiplayer is enabled, run only logic and display everything afterwards for each screen:

        int backup = loop_count;
        loop_count = 1; // Prevent display subs from running
        TARGET_DYNAMIC(LoopTask)();
        loop_count = backup;

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

    late_exec_t = new Trampoline(0x4086F0, 0x4086F6, late_exec_r);
    late_setOdr_t = new Trampoline(0x403F60, 0x403F65, late_setOdr_asm);
}
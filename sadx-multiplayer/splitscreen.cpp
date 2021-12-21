#include "pch.h"
#include <d3d8types.h>
#include <d3d8.h>
#include <SADXModLoader.h>
#include <Trampoline.h>
#include "splitscreen.h"
#include "multihud.h"
#include "d3d8vars.h"
#include "drawqueue.h"

/*

Splitscreen
Series of hacks to make splitscreen possible
- Prevents display subs from running in the exec subs
- Draw all display subs for each viewport; matrix and projection are calculated in camera.cpp

*/



unsigned int numScreen = 0;
signed int numViewPort = -1;

Trampoline* DisplayTask_t         = nullptr;
Trampoline* LoopTask_t            = nullptr;

Trampoline* njDrawTexture_t = nullptr;
Trampoline* njDrawTriangle2D_t = nullptr;
Trampoline* njDrawCircle2D_t = nullptr;
Trampoline* njDrawLine2D_t = nullptr;
Trampoline* njDrawQuadTextureEx_t = nullptr;

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
    return &ScreenRatios[player_count - 2][num];
}

// Set full screen viewport
void ResetViewPort()
{
    Direct3D_ViewPort = { 0, 0, (unsigned long)HorizontalResolution, (unsigned long)VerticalResolution, 0.0f, 1.0f };
    Direct3D_Device->SetViewport(&Direct3D_ViewPort);
    numViewPort = -1;
}

// Change the viewport
bool ChangeViewPort(int num)
{
    if (num == numViewPort)
    {
        return false;
    }

    if (num == -1)
    {
        ResetViewPort();
        return true;
    }

    if (num < 0 || num > player_count || num > PLAYER_MAX)
    {
        return false;
    }

    auto ratio = GetScreenRatio(num);

    Direct3D_ViewPort.X = ratio->x * HorizontalResolution;
    Direct3D_ViewPort.Y = ratio->y * VerticalResolution;
    Direct3D_ViewPort.Width = ratio->w * HorizontalResolution;
    Direct3D_ViewPort.Height = ratio->h * VerticalResolution;
    Direct3D_Device->SetViewport(&Direct3D_ViewPort);
    
    numViewPort = num;
    return true;
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
    if (IsMultiplayerEnabled())
    {
        // If multiplayer is enabled, split screen:

        for (int i = player_count; i > 0; --i)
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

void __cdecl njDrawTexture_r(NJS_TEXTURE_VTX* tex, Int count, Uint32 gbix, Int flgs)
{
    if (IsMultiplayerEnabled())
    {
        int backup = numViewPort;
        ChangeViewPort(-1);
        TARGET_DYNAMIC(njDrawTexture)(tex, count, gbix, flgs);
        ChangeViewPort(backup);
    }
    else
    {
        TARGET_DYNAMIC(njDrawTexture)(tex, count, gbix, flgs);
    }
}

void __cdecl njDrawTriangle2D_r(NJS_POINT2COL* p, Int n, Float pri, Uint32 attr)
{
    if (IsMultiplayerEnabled())
    {
        int backup = numViewPort;
        ChangeViewPort(-1);
        TARGET_DYNAMIC(njDrawTriangle2D)(p, n, pri, attr);
        ChangeViewPort(backup);
    }
    else
    {
        TARGET_DYNAMIC(njDrawTriangle2D)(p, n, pri, attr);
    }
}

void __cdecl njDrawLine2D_r(NJS_POINT2COL* p, Int n, Float pri, Uint32 attr)
{
    if (IsMultiplayerEnabled())
    {
        int backup = numViewPort;
        ChangeViewPort(-1);
        TARGET_DYNAMIC(njDrawLine2D)(p, n, pri, attr);
        ChangeViewPort(backup);
    }
    else
    {
        TARGET_DYNAMIC(njDrawLine2D)(p, n, pri, attr);
    }
}

void __cdecl njDrawCircle2D_r(NJS_POINT2COL* p, Int n, Float pri, Uint32 attr)
{
    if (IsMultiplayerEnabled())
    {
        int backup = numViewPort;
        ChangeViewPort(-1);
        TARGET_DYNAMIC(njDrawCircle2D)(p, n, pri, attr);
        ChangeViewPort(backup);
    }
    else
    {
        TARGET_DYNAMIC(njDrawCircle2D)(p, n, pri, attr);
    }
}

void __cdecl njDrawQuadTextureEx_r(NJS_QUAD_TEXTURE_EX* quad)
{
    if (IsMultiplayerEnabled())
    {
        int backup = numViewPort;
        ChangeViewPort(-1);
        TARGET_DYNAMIC(njDrawQuadTextureEx)(quad);
        ChangeViewPort(backup);
    }
    else
    {
        TARGET_DYNAMIC(njDrawQuadTextureEx)(quad);
    }
}

// Draw into viewport with scaling
void __cdecl njDrawQuadTextureEx_scale(NJS_QUAD_TEXTURE_EX* quad)
{
    if (IsMultiplayerEnabled() && numViewPort != -1)
    {
        auto ratio = GetScreenRatio(numScreen);

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
    LoopTask_t = new Trampoline(0x40B170, 0x40B178, LoopTask_r);
    WriteCall((void*)((int)(LoopTask_t->Target()) + 3), RunObjectIndex); // Repair LoopTask_t
    DisplayTask_t = new Trampoline(0x40B540, 0x40B546, DisplayTask_r);
    WriteCall((void*)((int)(LoopTask_t->Target()) + 3), (void*)0x40B0C0); // Repair DisplayTask_t

    // Draw GUI things into whole frame
    //njDrawTexture_t = new Trampoline(0x77DC70, 0x77DC79, njDrawTexture_r);
    //njDrawQuadTextureEx_t = new Trampoline(0x77DE10, 0x77DE18, njDrawQuadTextureEx_scale);
    //njDrawTriangle2D_t = new Trampoline(0x77E9F0, 0x77E9F8, njDrawTriangle2D_r);
    //njDrawCircle2D_t = new Trampoline(0x77DFC0, 0x77DFC7, njDrawCircle2D_r);
    //njDrawLine2D_t = new Trampoline(0x77DF40, 0x77DF49, njDrawLine2D_r);

    DrawQueue_Init();
}
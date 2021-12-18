#include "pch.h"
#include "splitscreen.h"

DataPointer(IDirect3DDevice8*, Direct3D_Device, 0x03D128B0);
DataPointer(D3DVIEWPORT8, Direct3D_ViewPort, 0x03D12780);

NJS_TEXNAME MULTIHUD_TEXNAME[3]{};
NJS_TEXLIST MULTIHUD_TEXLIST = { arrayptrandlength(MULTIHUD_TEXNAME) };

NJS_TEXANIM MULTIHUD_TEXANIMS[]
{
    { 32, 32, 0, 16, 0, 0, 64, 85, 0, 0x20 },
    { 32, 32, 0, 16, 64, 0, 128, 85, 0, 0x20 },
    { 32, 32, 0, 16, 128, 0, 192, 85, 0, 0x20 },
    { 32, 32, 0, 16, 192, 0, 255, 85, 0, 0x20 },
    { 32, 32, 0, 16, 0, 86, 64, 170, 0, 0x20 },
    { 32, 32, 0, 16, 64, 86, 128, 170, 0, 0x20 },
    { 32, 32, 0, 16, 128, 86, 192, 170, 0, 0x20 },
    { 32, 32, 0, 16, 192, 86, 255, 170, 0, 0x20 },
    { 32, 32, 0, 16, 0, 171, 64, 255, 0, 0x20 },
    { 32, 32, 0, 16, 64, 171, 128, 255, 0, 0x20 },
    { 32, 32, 0, 16, 128, 171, 192, 255, 0, 0x20 },
    { 32, 32, 0, 16, 192, 171, 255, 255, 0, 0x20 },
    { 16, 16, 0, 16, 0, 0, 124, 255, 1, 0x20 },
    { 16, 16, 0, 16, 127, 0, 255, 255, 1, 0x20 },
    { 16, 16, 0, 8, 0, 0, 64, 64, 2, 0x20 },
    { 16, 16, 0, 8, 64, 0, 128, 64, 2, 0x20 },
    { 16, 16, 0, 8, 128, 0, 192, 64, 2, 0x20 },
    { 16, 16, 0, 8, 192, 0, 255, 64, 2, 0x20 },
    { 16, 16, 0, 8, 0, 64, 64, 128, 2, 0x20 },
    { 16, 16, 0, 8, 64, 64, 128, 128, 2, 0x20 },
    { 16, 16, 0, 8, 128, 64, 192, 128, 2, 0x20 },
    { 16, 16, 0, 8, 192, 64, 255, 128, 2, 0x20 },
    { 16, 16, 0, 8, 0, 128, 64, 192, 2, 0x20 },
    { 16, 16, 0, 8, 64, 128, 128, 192, 2, 0x20 },
    { 16, 16, 0, 8, 128, 128, 192, 192, 2, 0x20 },
    { 16, 16, 0, 8, 192, 128, 255, 192, 2, 0x20 },
    { 16, 16, 0, 8, 0, 192, 64, 255, 2, 0x20 },
    { 1, 1, 0, 0, 0, 0, 255, 255, 3, 0x20 }
};

NJS_SPRITE MULTIHUD_SPRITE = { {0.0f, 0.0f, 0.0f}, 1.0f, 1.0f, 0, &MULTIHUD_TEXLIST, MULTIHUD_TEXANIMS };

static const int textseq[]{ 10, 0, 4, 12, 4, 6, 3, -1, 2, 7, 9, -1, 8, 5, 0, 11, 1, 9 };

unsigned int numScreen = 0;
signed int numViewPort = -1;

Trampoline* DisplayTask_t         = nullptr;
Trampoline* LoopTask_t            = nullptr;
Trampoline* late_exec_t           = nullptr;
Trampoline* late_setOdr_t         = nullptr;
Trampoline* njDrawQuadTextureEx_t = nullptr;

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

ScreenRatio* GetScreenRatio(int num)
{
    return player_count <= 2 ? &ScreenRatio2[num] : &ScreenRatio4[num];
}

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

    auto ratio = GetScreenRatio(num);

    Direct3D_ViewPort.X = ratio->x * HorizontalResolution;
    Direct3D_ViewPort.Y = ratio->y * VerticalResolution;
    Direct3D_ViewPort.Width = ratio->w * HorizontalResolution;
    Direct3D_ViewPort.Height = ratio->h * VerticalResolution;
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

void DrawWaitScreen(int num)
{
    // TODO: load texlist at correct place
    if (VerifyTexList(&MULTIHUD_TEXLIST))
    {
        LoadPVM("multihud", &MULTIHUD_TEXLIST);
        return;
    }

    // Draw black screen
    MULTIHUD_SPRITE.p.x = MULTIHUD_SPRITE.p.y = 0.0f;
    MULTIHUD_SPRITE.sx = HorizontalResolution;
    MULTIHUD_SPRITE.sy = VerticalResolution;
    njDrawSprite2D_DrawNow(&MULTIHUD_SPRITE, 27, 1000000.0f, 0);
    
    // Get subscreen information
    auto ratio = GetScreenRatio(num);

    // Start position and scale
    float scale = HorizontalStretch * ratio->w;
    MULTIHUD_SPRITE.sx = scale;
    MULTIHUD_SPRITE.sy = scale;
    MULTIHUD_SPRITE.p.x = 140.0f * scale + HorizontalResolution * ratio->x;
    MULTIHUD_SPRITE.p.y = 240.0f * (VerticalStretch * ratio->h) + VerticalResolution * ratio->y;

    // Draw Cream
    njDrawSprite2D_DrawNow(&MULTIHUD_SPRITE, (GameTimer / 5) % 12, 1000000.0f, NJD_SPRITE_ALPHA);

    // Move right
    MULTIHUD_SPRITE.p.x += 30 * scale;
    float x = MULTIHUD_SPRITE.p.x; // backup position
    MULTIHUD_SPRITE.p.x += njSin(GameTimer * 300) * 2.5f; // slide left and right
    
    // Draw Chao
    njDrawSprite2D_DrawNow(&MULTIHUD_SPRITE, 12 + (GameTimer / 5) % 2, 1000000.0f, NJD_SPRITE_ALPHA);

    // Restore position and move right
    MULTIHUD_SPRITE.p.x = x + 20 * scale;

    float y = MULTIHUD_SPRITE.p.y; // backup position

    for (int i = 0; i < LengthOfArray(textseq); ++i)
    {
        MULTIHUD_SPRITE.p.x += 16 * scale; // move right
        MULTIHUD_SPRITE.p.y = y + njSin(GameTimer * 1000 + i * 1000) * 5; // slide up and down individually
        SetMaterialAndSpriteColor_Float(1.0f - (fabs(njSin(GameTimer * 500 + i * 500)) * 0.5f), 1.0f, 1.0f, 1.0f); // color ramp

        // Draw letter
        if (textseq[i] != -1) njDrawSprite2D_DrawNow(&MULTIHUD_SPRITE, 14 + textseq[i], 1000000.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
    }

    ClampGlobalColorThing_Thing();
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

void __cdecl njDrawQuadTextureEx_r(NJS_QUAD_TEXTURE_EX* quad)
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

    late_exec_t = new Trampoline(0x4086F0, 0x4086F6, late_exec_r);
    late_setOdr_t = new Trampoline(0x403F60, 0x403F65, late_setOdr_asm);

    //njDrawQuadTextureEx_t = new Trampoline(0x77DE10, 0x77DE18, njDrawQuadTextureEx_r);
}
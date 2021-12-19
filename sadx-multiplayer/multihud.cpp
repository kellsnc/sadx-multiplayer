#include "pch.h"
#include "multihud.h"
#include "splitscreen.h"

NJS_TEXNAME MULTIHUD_TEXNAME[3]{};
NJS_TEXLIST MULTIHUD_TEXLIST = { arrayptrandlength(MULTIHUD_TEXNAME) };

NJS_TEXANIM MULTIHUD_TEXANIMS[]{
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

static const int waittextseq[] = { 10, 0, 4, 12, 4, 6, 3, -1, 2, 7, 9, -1, 8, 5, 0, 11, 1, 9 };

void LoadMultiHudPVM()
{
    if (VerifyTexList(&MULTIHUD_TEXLIST))
    {
        LoadPVM("multihud", &MULTIHUD_TEXLIST);
    }
}

void DrawWaitScreen(int num)
{
    LoadMultiHudPVM();

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

    for (int i = 0; i < LengthOfArray(waittextseq); ++i)
    {
        MULTIHUD_SPRITE.p.x += 16 * scale; // move right
        MULTIHUD_SPRITE.p.y = y + njSin(GameTimer * 1000 + i * 1000) * 5; // slide up and down individually
        SetMaterialAndSpriteColor_Float(1.0f - (fabs(njSin(GameTimer * 500 + i * 500)) * 0.5f), 1.0f, 1.0f, 1.0f); // color ramp

        // Draw letter
        if (waittextseq[i] != -1) njDrawSprite2D_DrawNow(&MULTIHUD_SPRITE, 14 + waittextseq[i], 1000000.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
    }

    ClampGlobalColorThing_Thing();
}
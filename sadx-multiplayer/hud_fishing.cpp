#include "pch.h"
#include "fishing.h"
#include "splitscreen.h"
#include "hud_multi.h"
#include "hud_fishing.h"

DataArray(NJS_TEXANIM, reel_anim, 0x91BAB0, 9);
static NJS_SPRITE reel_sprite = { { 1.0f, 1.0f, 1.0f }, 1.0f, 1.0f, 0, &FISHING_TEXLIST, &reel_anim };

#pragma region HUD
static void DrawBigHUDMulti(int pnum)
{
    auto ratio = SplitScreen::GetScreenRatio(pnum);

    float scaleY = VerticalStretch * ratio->h;
    float scaleX = HorizontalStretch * ratio->w;
    float scale = min(scaleX, scaleY);

    MULTIHUD_SPRITE.sx = MULTIHUD_SPRITE.sy = scale;
    MULTIHUDDIGIT_SPRITE.sx = MULTIHUDDIGIT_SPRITE.sy = scale;

    float x = 16.0f * scaleX + HorizontalResolution * ratio->x;
    float y = 16.0f * scaleY + VerticalResolution * ratio->y;

    if (HideTimerAndRings >= 0)
    {
        reel_sprite.p.x = x + 48 * scale;
        reel_sprite.p.y = y + 48 * scale;
        reel_sprite.sx = reel_sprite.sy = scale * 1.5f;

        njDrawSprite2D_ForcePriority(&reel_sprite, 4, -1.82f, NJD_SPRITE_ALPHA);

        reel_sprite.p.x = x + 226 * scale;
        reel_sprite.p.y = y + 24 * scale;
        njDrawSprite2D_ForcePriority(&reel_sprite, 5, -1.82f, NJD_SPRITE_ALPHA);

        auto etc = GetBigEtc(pnum);
        _SC_NUMBERS pscn;
        pscn.scl = scale * 1.5f;
        pscn.type = 0x8;
        pscn.attr = 0x97;
        pscn.rot = 0;
        pscn.max = 99999;
        pscn.color = 0xFFFFFFFF;
        pscn.value = etc ? etc->Big_Sakana_Weight : 0;
        pscn.pos.x = x + 96.0f * scale;
        pscn.pos.y = y + 2.0f * scale;
        pscn.pos.z = 0.0f;
        DrawSNumbers(&pscn);

        MULTIHUD_SPRITE.p.x = x + 8 * scale;
        MULTIHUD_SPRITE.p.y = y + 31 * scale;
        MultiHudRings(pnum);
    }

    if (HideLives >= 0)
    {
        MULTIHUDDIGIT_SPRITE.p.x = x;
        MULTIHUDDIGIT_SPRITE.p.y = VerticalResolution * ratio->h - 16.0f * scaleY + VerticalResolution * ratio->y;
        MultiHudLives(pnum);
    }
}

static void __cdecl dispZankiTexturePause_r(task* tp);
Trampoline dispZankiTexturePause_t(0x46FB00, 0x46FB05, dispZankiTexturePause_r);
static void __cdecl dispZankiTexturePause_r(task* tp)
{
    if (SplitScreen::IsActive())
    {
        if (!loop_count && ssStageNumber != STAGE_TWINKLEPARK && HideHud >= 0 && (ulGlobalMode != MD_GAME_FADEOUT_CHANGE2 || !GetMiClearStatus()))
        {
            reel_sprite.ang = 0;

            ghDefaultBlendingMode();
            SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

            SplitScreen::SaveViewPort();
            SplitScreen::ChangeViewPort(-1);
            for (int i = 0; i < PLAYER_MAX; ++i)
            {
                if (SplitScreen::IsScreenEnabled(i))
                {
                    DrawBigHUDMulti(i);
                }
            }
            SplitScreen::RestoreViewPort();
            ResetMaterial();
        }
    }
    else
    {
        TARGET_STATIC(dispZankiTexturePause)(tp);
    }
}
#pragma endregion

#pragma region dispFishWeightTexture
void dispFishWeightTexture_m(taskwk* twp, int pnum)
{
    if (SplitScreen::IsScreenEnabled(pnum))
    {
        SplitScreen::SaveViewPort();
        SplitScreen::ChangeViewPort(-1);

        ghDefaultBlendingMode();
        SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

        auto ratio = SplitScreen::GetScreenRatio(pnum);
        float scaleX = HorizontalStretch * ratio->w;
        float scaleY = VerticalStretch * ratio->h;
        auto scale = min(scaleX, scaleY);

        float pos = min(twp->pos.x, (scaleX * 540.0f) - 240 * scaleX);

        float x = HorizontalResolution * ratio->x + pos * scaleX + 16.0f * scaleX;
        float y = VerticalResolution * ratio->y + 16.0f * scaleY;

        reel_sprite.p.x = x + 130.0f * scale;
        reel_sprite.p.y = y + 24.0f * scale;
        reel_sprite.sx = reel_sprite.sy = scale * 1.5f;
        reel_sprite.ang = 0;
        late_DrawSprite2D(&reel_sprite, 5, 22046.182f, NJD_SPRITE_ALPHA, 4);

        _SC_NUMBERS pscn;
        
        pscn.scl = scale * 1.5f;
        pscn.type = 0x9;
        pscn.attr = 0x94;
        pscn.rot = 0;
        pscn.max = 99999;
        pscn.color = 0xFFFFFFFF;
        pscn.value = twp->value.l;
        pscn.pos.x = x;
        pscn.pos.y = y + 2 * scale;
        pscn.pos.z = 0.0f;
        DrawSNumbers(&pscn);

        ResetMaterial();
        SplitScreen::RestoreViewPort();
    }
}
#pragma endregion

#pragma region BigDisplayHit
static void dispHitTexturePause_m(task* tp)
{
    auto twp = tp->twp;

    if (twp->mode == 0 || twp->mode == 4)
        return;

    auto pnum = TASKWK_PLAYERID(twp);

    SplitScreen::SaveViewPort();
    SplitScreen::ChangeViewPort(-1);

    auto ratio = SplitScreen::GetScreenRatio(pnum);

    float scaleX = HorizontalStretch * ratio->w;
    float scaleY = VerticalStretch * ratio->h;
    float x = HorizontalResolution * ratio->x + scaleX * 320.0f + scaleX * twp->pos.x;
    float y = VerticalResolution * ratio->y + scaleY * 200.0f;

    float alpha = 1.0f;
    if (twp->pos.x > 0.0f)
    {
        alpha = (320.0f - twp->pos.x) / 420.0f;
    }
    else if (twp->pos.x < 0.0f)
    {
        alpha = (320.0f + twp->pos.x) / 420.0f;
    }
    SetMaterial(max(0.0f, alpha), 1.0f, 1.0f, 1.0f);

    reel_sprite.p.x = x + 8.0f * scaleX;
    reel_sprite.p.y = y + 8.0f * scaleY;
    reel_sprite.sx = reel_sprite.sy = min(scaleX, scaleY);
    reel_sprite.ang = 0;
    late_DrawSprite2D(&reel_sprite, 7, 22046.18f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, 4);

    reel_sprite.p.x = x;
    reel_sprite.p.y = y;
    late_DrawSprite2D(&reel_sprite, 6, 22046.18f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, 4);
    ResetMaterial();
    SplitScreen::RestoreViewPort();
}

static void __cdecl dispHitTexturePause_r(task* tp);
Trampoline dispHitTexturePause_t(0x46C920, 0x46C926, dispHitTexturePause_r);
static void __cdecl dispHitTexturePause_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        if (SplitScreen::IsScreenEnabled(TASKWK_PLAYERID(tp->twp)))
        {
            dispHitTexturePause_m(tp);
        }
    }
    else
    {
        TARGET_STATIC(dispHitTexturePause)(tp);
    }
}

void SetBigDispHit_m(int pnum)
{
    auto tp = CreateElementalTask(2, 6, BigDisplayHit);
    TASKWK_PLAYERID(tp->twp) = pnum;
}

#pragma endregion

#pragma region Fishing Meter
static void dispReelMeter_m(float x, float y, float scale, float _reel_tension)
{
    x -= 64.0f * scale;

    reel_sprite.p.x = x;
    reel_sprite.p.y = y;
    reel_sprite.sx = reel_sprite.sy = scale;
    reel_sprite.ang = 0;
    late_DrawSprite2D(&reel_sprite, 0, 22046.18f, NJD_SPRITE_ALPHA, 4);

    NJS_POINT2 rect[4];
    NJS_COLOR color[4];
    NJS_POINT2COL p2col = { rect, color, nullptr, 4 };

    uint32_t max_tension = (uint32_t)(_reel_tension * 255.0f);

    rect[1].x = x - 1.0f * scale;
    rect[0].x = rect[1].x;
    rect[2].y = y + 40.0f * scale;
    rect[0].y = rect[2].y;
    rect[3].x = x + 31.0f * scale;
    rect[2].x = rect[3].x;
    rect[3].y = 40.0f * scale - (_reel_tension * 135.0f) * scale + y;
    rect[1].y = rect[3].y;
    color[3].color = 0xFFFF * (max_tension + 0xFF01);
    color[1] = color[3];
    color[2].color = 0xFFFF * max_tension - 0xFF0001;
    color[0] = color[2];
    Draw2DLinesMaybe_Queue(&p2col, 4, 22046.18f, NJD_FILL, QueuedModelFlagsB_SomeTextureThing); // todo: fix late_DrawPolygon2D in includes
}

static void dispHandleTexture_m(float x, float y, float scale, Angle _reel_angle)
{
    reel_sprite.p.x = x - 64.0f * scale + 16.0f * scale;
    reel_sprite.p.y = y + 66.0f * scale;
    reel_sprite.sx = reel_sprite.sy = scale;
    reel_sprite.ang = _reel_angle;
    njDrawSprite2D_Queue(&reel_sprite, 1, 22046.182f, NJD_SPRITE_ALPHA | NJD_SPRITE_ANGLE, QueuedModelFlagsB_SomeTextureThing);
}

static void dispDistanceLure_m(float x, float y, float scale, float dist)
{
    _SC_NUMBERS pscn;

    ghDefaultBlendingMode();
    SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
    pscn.scl = scale;
    pscn.type = 0x48;
    pscn.attr = 0x97;
    pscn.rot = 0;
    pscn.max = 99;
    pscn.color = 0xFFFFFFFF;
    pscn.value = static_cast<unsigned int>(dist * 0.1f);
    pscn.pos.x = x - 64.0f * scale - 21.0f * scale;
    pscn.pos.y = y + 90.0f * scale;
    pscn.pos.z = 0.0f;
    DrawSNumbers(&pscn);
    ResetMaterial();
}

static void dispReelMeterAll(float x, float y, float scale, float _reel_length_d, float _reel_tension, Angle _reel_angle)
{
    dispReelMeter_m(x, y, scale, _reel_tension);
    dispHandleTexture_m(x, y, scale, _reel_angle);
    dispDistanceLure_m(x, y, scale, _reel_length_d);
}

static void DrawFishingMeter_Screen(int pnum, float _reel_length_d, float _reel_tension, Angle _reel_angle)
{
    if (SplitScreen::GetCurrentScreenNum() == pnum)
    {
        auto ratio = SplitScreen::GetScreenRatio(pnum);

        float scaleX = HorizontalStretch * ratio->w;
        float scaleY = VerticalStretch * ratio->h;

        dispReelMeterAll(HorizontalResolution * ratio->x + scaleX * 640.0f,
            VerticalResolution * ratio->y + scaleY * 328.0f,
            min(scaleX, scaleY), _reel_length_d, _reel_tension, _reel_angle);
    }
}

void DrawFishingMeter(int pnum, float _reel_length_d, float _reel_tension, Angle _reel_angle)
{
    njSetTexture(&FISHING_TEXLIST);

    if (SplitScreen::IsActive())
    {
        SplitScreen::SaveViewPort();
        SplitScreen::ChangeViewPort(-1);
        DrawFishingMeter_Screen(pnum, _reel_length_d, _reel_tension, _reel_angle);
        SplitScreen::RestoreViewPort();
    }
    else
    {
        DrawFishingMeter_Screen(pnum, _reel_length_d, _reel_tension, _reel_angle);
    }
}
#pragma endregion
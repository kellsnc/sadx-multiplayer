#include "pch.h"
#include "fishing.h"
#include "splitscreen.h"
#include "hud_fishing.h"

DataArray(NJS_TEXANIM, reel_anim, 0x91BAB0, 9);

static NJS_SPRITE reel_sprite = { { 1.0f, 1.0f, 1.0f }, 1.0f, 1.0f, 0, &FISHING_TEXLIST, &reel_anim };

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

    int max_tension = (int)(reel_tension * 255.0f);

    rect[1].x = x - 1.0f * scale;
    rect[0].x = rect[1].x;
    rect[2].y = y + 40.0f * scale;
    rect[0].y = rect[2].y;
    rect[3].x = x + 31.0f * scale;
    rect[2].x = rect[3].x;
    rect[3].y = 40.0f * scale - (reel_tension * 135.0f) * scale + y;
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
    reel_sprite.ang = reel_angle;
    njDrawSprite2D_Queue(&reel_sprite, 1, 22046.182f, NJD_SPRITE_ALPHA | NJD_SPRITE_ANGLE, QueuedModelFlagsB_SomeTextureThing);
}

static void dispDistanceLure_m(float x, float y, float scale, float dist)
{
    _SC_NUMBERS pscn;

    njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
    njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
    SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
    pscn.scl = scale;
    pscn.type = 0x48;
    pscn.attr = 0x97;
    pscn.rot = 0;
    pscn.max = 99;
    pscn.color = 0xFFFFFFFF;
    pscn.value = (dist * 0.1f);
    pscn.pos.x = x - 64.0f * scale - 21.0 * scale;
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
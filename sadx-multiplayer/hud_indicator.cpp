#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"
#include "hud_multi.h"
#include "hud_indicator.h"

/*

Multiplayer Indicators
Adapted from https://github.com/michael-fadely/sadx-multitap

*/

static task* indicator_tp = nullptr;

enum indicator_tex
{
    arrow, arrow_border, cpu_1, cpu_2, p, numbers
};

static const NJS_TEXANIM INDICATOR_TEXANIMS[] {
    { 24, 16, 12, -16, 0, 0, 0xFF, 0xFF, MHudTex_Arrow, 0 },
    { 24, 16, 12, -28, 0, 0, 0xFF, 0xFF, MHudTex_Arrow, 0 },
    { 24, 24, 24, 12,  0, 0, 0xFF, 0xFF, MHudTex_CPU1,  0 },
    { 24, 24, 0,  12,  0, 0, 0xFF, 0xFF, MHudTex_CPU2,  0 },
    { 24, 24, 24, 12,  0, 0, 0xFF, 0xFF, MHudTex_P,     0 },
    { 24, 24, 0,  12,  0, 0, 0xFF, 0xFF, MHudTex_P1,    0 },
    { 24, 24, 0,  12,  0, 0, 0xFF, 0xFF, MHudTex_P2,    0 },
    { 24, 24, 0,  12,  0, 0, 0xFF, 0xFF, MHudTex_P3,    0 },
    { 24, 24, 0,  12,  0, 0, 0xFF, 0xFF, MHudTex_P4,    0 }
};

static NJS_SPRITE INDICATOR_SPRITE = { {0.0f, 0.0f, 0.0f}, 1.0f, 1.0f, 0, &CON_MULTI_TEXLIST, (NJS_TEXANIM*)INDICATOR_TEXANIMS };

static const NJS_ARGB colors[] = {
    { 1.000f, 0.000f, 0.000f, 1.000f }, // Sonic
    { 1.000f, 0.500f, 0.000f, 0.000f }, // Eggman
    { 1.000f, 1.000f, 0.804f, 0.000f }, // Tails
    { 1.000f, 1.000f, 0.063f, 0.000f }, // Knuckles
    { 1.000f, 1.000f, 0.545f, 0.322f }, // Tikal
    { 1.000f, 1.000f, 0.545f, 0.741f }, // Amy
    { 1.000f, 0.545f, 0.545f, 0.545f }, // Gamma
    { 1.000f, 0.451f, 0.192f, 0.804f }, // Big
    { 1.000f, 0.000f, 1.000f, 1.000f }, // Metal Sonic
    { 0.750f, 0.500f, 0.500f, 0.500f }  // CPU
};

static void drawSprite(int tex, NJD_SPRITE attr)
{
	late_DrawSprite2D(&INDICATOR_SPRITE, tex, 22048.0f, attr, 0);
}

static void dispIndicatorNum(int pnum, float screenX, float screenY, float screenWidth, float screenHeight, float scale)
{
	auto ptwp = playertwp[pnum];
	auto ppwp = playerpwp[pnum];

	if (!ptwp || !ppwp)
	{
		return;
	}

	NJS_POINT3 pos = ptwp->pos;
	pos.y += ppwp->p.height;
	njCalcPoint(nullptr, &pos, &pos); // get camera space position

	bool behind = pos.z <= 0.0f;
	auto m = _nj_screen_.dist / pos.z; // projection

	if (behind)
		m = -m;

	const float xhalf = screenWidth / 2.0f;
	const float yhalf = screenHeight / 2.0f;

	pos.x = pos.x * m + xhalf;
	pos.y = pos.y * m + yhalf;

	// If the player is behind the camera, round either the X or Y offset to the screen edge.
	if (behind)
	{
		if (pos.y < screenHeight && pos.y > 0.0f)
		{
			pos.x = pos.x < xhalf ? 0.0f : screenWidth;
		}
		else if (pos.x < screenWidth && pos.x > 0.0f)
		{
			pos.y = pos.y < yhalf ? 0.0f : screenHeight;
		}
	}

	pos.y -= 40 * scale;

	const float margin_right = screenWidth * 0.875f;
	const float margin_left = screenWidth - margin_right;
	const float margin_bottom = screenHeight * 0.875f;
	const float margin_top = screenHeight - margin_bottom;

	// Clamp to screen borders
	const float new_x = min(max(margin_left, pos.x), margin_right);
	const float new_y = min(max(margin_top, pos.y), margin_bottom);

	// Apply position and scale
	INDICATOR_SPRITE.p.x = new_x + screenX;
	INDICATOR_SPRITE.p.y = new_y + screenY;
	INDICATOR_SPRITE.sx = INDICATOR_SPRITE.sy = scale;

	// Set rendering colour
	___njSetConstantMaterial(CheckPadReadModeP((unsigned char)pnum) ? (NJS_ARGB*)&colors[TASKWK_CHARID(ptwp)] : (NJS_ARGB*)&colors[9]);
	
	const bool isVisible = pos.x < margin_right && pos.x > margin_left && pos.y < margin_bottom && pos.y > margin_top;
	if (isVisible)
	{
		drawSprite(indicator_tex::arrow, NJD_SPRITE_COLOR | NJD_SPRITE_ALPHA);
	}
	else
	{
		INDICATOR_SPRITE.ang = NJM_RAD_ANG(atan2(pos.y - new_y, pos.x - new_x)) - 0x4000; // rotate clamped arrow toward real position
		drawSprite(indicator_tex::arrow_border, NJD_SPRITE_COLOR | NJD_SPRITE_ALPHA | NJD_SPRITE_ANGLE);
	}

	const bool IsCPU = ptwp->id == 2;
	drawSprite(IsCPU ? indicator_tex::cpu_1 : indicator_tex::p, NJD_SPRITE_COLOR | NJD_SPRITE_ALPHA);
	drawSprite(IsCPU ? indicator_tex::cpu_2 : indicator_tex::numbers + pnum, NJD_SPRITE_COLOR | NJD_SPRITE_ALPHA);
}

static void __cdecl dispIndicatorP(task* tp)
{
    if (!MissedFrames)
    {
        ghDefaultBlendingMode();

		bool splitscreen = SplitScreen::IsActive();

		if (SplitScreen::IsActive())
		{
			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				auto ratio = SplitScreen::GetScreenRatio(SplitScreen::GetCurrentScreenNum());

				SplitScreen::SaveViewPort();
				SplitScreen::ChangeViewPort(-1);
				dispIndicatorNum(i,
					(float)HorizontalResolution * ratio->x,
					(float)VerticalResolution * ratio->y,
					(float)HorizontalResolution * ratio->w,
					(float)VerticalResolution * ratio->h,
					min(HorizontalStretch * ratio->w, VerticalStretch * ratio->h));
				SplitScreen::RestoreViewPort();
			}
		}
		else
		{
			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				dispIndicatorNum(i, 0.0f, 0.0f, (float)HorizontalResolution, (float)VerticalResolution, min(HorizontalStretch, VerticalStretch));
			}
		}

        ResetMaterial();
    }
}

static void __cdecl destIndicatorP(task* tp)
{
	indicator_tp = nullptr;
}

static void __cdecl IndicatorP(task* tp)
{
    tp->disp(tp);
}

void CreateIndicatorP()
{
	if (indicator_tp == nullptr)
	{
		auto tp = indicator_tp = CreateElementalTask(0u, LEV_6, IndicatorP);
		tp->disp = dispIndicatorP;
		tp->dest = destIndicatorP;
	}
}
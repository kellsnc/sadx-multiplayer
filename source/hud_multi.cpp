#include "pch.h"
#include "hud_multi.h"
#include "splitscreen.h"

Trampoline* DisplayScore_t = nullptr;
Trampoline* DisplayTimer_t = nullptr;
Trampoline* LoadTextureForEachGameMode_t = nullptr;
Trampoline* ReleaseTextureForEachGameMode_t = nullptr;

NJS_TEXNAME CON_MULTI_TEXNAME[18]{};
NJS_TEXLIST CON_MULTI_TEXLIST = { arrayptrandlength(CON_MULTI_TEXNAME) };

enum MHudSprt
{
	MHudSprt_Cream = 0,
	MHudSprt_Cheese = 12,
	MHudSprt_Alphabet = 14,
	MHudSprt_Score = 27,
	MHudSprt_Time,
	MHudSprt_Ring
};

NJS_TEXANIM CON_MULTI_TEXANIMS[]{
	{ 32, 32, 0, 16, 0, 0, 64, 85, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 64, 0, 128, 85, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 128, 0, 192, 85, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 192, 0, 255, 85, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 0, 86, 64, 170, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 64, 86, 128, 170, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 128, 86, 192, 170, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 192, 86, 255, 170, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 0, 171, 64, 255, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 64, 171, 128, 255, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 128, 171, 192, 255, MHudTex_Cream, 0x20 },
	{ 32, 32, 0, 16, 192, 171, 255, 255, MHudTex_Cream, 0x20 },
	{ 16, 16, 0, 16, 0, 0, 124, 255, MHudTex_Cheese, 0x20 },
	{ 16, 16, 0, 16, 127, 0, 255, 255, MHudTex_Cheese, 0x20 },
	{ 16, 16, 0, 8, 0, 0, 64, 64, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 64, 0, 128, 64, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 128, 0, 192, 64, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 192, 0, 255, 64, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 0, 64, 64, 128, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 64, 64, 128, 128, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 128, 64, 192, 128, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 192, 64, 255, 128, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 0, 128, 64, 192, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 64, 128, 128, 192, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 128, 128, 192, 192, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 192, 128, 255, 192, MHudTex_Alphabet, 0x20 },
	{ 16, 16, 0, 8, 0, 192, 64, 255, MHudTex_Alphabet, 0x20 },
	{ 64, 24, 0, 0, 0, 0, 255, 255, MHudTex_Score, 0x20 },
	{ 55, 24, 0, 0, 0, 0, 255, 255, MHudTex_Time, 0x20 },
	{ 24, 24, 0, 0, 0, 0, 255, 255, MHudTex_Ring, 0x20 }
};

NJS_SPRITE MULTIHUD_SPRITE = { {0.0f, 0.0f, 0.0f}, 1.0f, 1.0f, 0, &CON_MULTI_TEXLIST, CON_MULTI_TEXANIMS };

NJS_TEXANIM MULTIHUDDIGIT_TEXANIMS[]{
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 0, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 1, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 2, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 3, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 4, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 5, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 6, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 7, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 8, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 9, 0x20 },
	{ 20, 20, 0, 0, 0, 0, 0x100, 0x100, 10, 0x20 },
	{ 0x40, 0x40, 0, 0, 0, 0, 0x100, 0x100, 11, 0x20 },
	{ 0x20, 0x20, 0, 0x20, 0, 0, 0x100, 0x100, 12, 0x20 },
	{ 0x20, 0x20, 0, 0x20, 0, 0, 0x100, 0x100, 13, 0x20 },
	{ 0x20, 0x20, 0, 0x20, 0, 0, 0x100, 0x100, 14, 0x20 },
	{ 0x20, 0x20, 0, 0x20, 0, 0, 0x100, 0x100, 15, 0x20 },
	{ 0x20, 0x20, 0, 0x20, 0, 0, 0x100, 0x100, 16, 0x20 },
	{ 0x20, 0x20, 0, 0x20, 0, 0, 0x100, 0x100, 17, 0x20 },
	{ 0x20, 0x20, 0, 0x20, 0, 0, 0x100, 0x100, 18, 0x20 },
	{ 0x20, 0x20, 0, 0x20, 0, 0, 0x100, 0x100, 19, 0x20 },
	{ 0x10, 0x10, 0, 0, 0, 0, 0x100, 0x100, 20, 0x20 },
	{ 0x10, 0x10, 0, 0, 0, 0, 0x100, 0x100, 21, 0x20 },
	{ 0x10, 0x10, 0, 0, 0, 0, 0x100, 0x100, 22, 0x20 },
	{ 0x10, 0x10, 0, 0, 0, 0, 0x100, 0x100, 23, 0x20 }
};

NJS_SPRITE MULTIHUDDIGIT_SPRITE = { {0.0f, 0.0f, 0.0f}, 1.0f, 1.0f, 0, &Hud_RingTimeLife_TEXLIST, MULTIHUDDIGIT_TEXANIMS };

static const int waittextseq[] = { 10, 0, 4, 12, 4, 6, 3, -1, 2, 7, 9, -1, 8, 5, 0, 11, 1, 9 };

static int ringtimer[PLAYER_MAX]{};

void LoadTextureForEachGameMode_r(int gamemode)
{
	LoadPVM("CON_MULTI", &CON_MULTI_TEXLIST);
	TARGET_DYNAMIC(LoadTextureForEachGameMode)(gamemode);
}

void ReleaseTextureForEachGameMode_r()
{
	njReleaseTexture(&CON_MULTI_TEXLIST);
	TARGET_DYNAMIC(ReleaseTextureForEachGameMode)();
}

void DrawWaitScreen(int num)
{
	if (MissedFrames || IsGamePaused())
	{
		return;
	}

	SplitScreen::SaveViewPort();
	SplitScreen::ChangeViewPort(-1);

	// Get subscreen information
	auto ratio = SplitScreen::GetScreenRatio(num);

	// Start position and scale
	float scaleY = VerticalStretch * ratio->h;
	float scale = HorizontalStretch * ratio->w;
	MULTIHUD_SPRITE.sx = MULTIHUD_SPRITE.sy = scale;
	MULTIHUD_SPRITE.p.x = 140.0f * scale + HorizontalResolution * ratio->x;
	MULTIHUD_SPRITE.p.y = 240.0f * scaleY + VerticalResolution * ratio->y;

	// Draw Cream
	njDrawSprite2D_DrawNow(&MULTIHUD_SPRITE, MHudSprt_Cream + (GameTimer / 5) % 12, -1000.0f, NJD_SPRITE_ALPHA);

	// Move right
	MULTIHUD_SPRITE.p.x += 30 * scale;
	float x = MULTIHUD_SPRITE.p.x; // backup position
	MULTIHUD_SPRITE.p.x += njSin(GameTimer * 300) * 2.5f; // slide chao left and right

	// Draw Chao
	njDrawSprite2D_DrawNow(&MULTIHUD_SPRITE, MHudSprt_Cheese + (GameTimer / 5) % 2, -1000.0f, NJD_SPRITE_ALPHA);

	// Restore position and move right
	MULTIHUD_SPRITE.p.x = x + 20 * scale;

	float y = MULTIHUD_SPRITE.p.y; // backup position

	for (size_t i = 0ui32; i < LengthOfArray(waittextseq); ++i)
	{
		MULTIHUD_SPRITE.p.x += 16 * scale; // move right
		MULTIHUD_SPRITE.p.y = y + njSin(GameTimer * 1000 + i * 1000) * 5; // slide up and down individually
		SetMaterial(1.0f - (fabs(njSin(GameTimer * 500 + i * 500)) * 0.5f), 1.0f, 1.0f, 1.0f); // color ramp

		// Draw letter
		if (waittextseq[i] != -1) njDrawSprite2D_DrawNow(&MULTIHUD_SPRITE, MHudSprt_Alphabet + waittextseq[i], -1000.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	}

	ResetMaterial();
	SplitScreen::RestoreViewPort();
}

void MultiHudScore(int num)
{
	MULTIHUDDIGIT_SPRITE.p.x = MULTIHUD_SPRITE.p.x + 195.0f * MULTIHUD_SPRITE.sx;
	MULTIHUDDIGIT_SPRITE.p.y = MULTIHUD_SPRITE.p.y + 2.0f * MULTIHUD_SPRITE.sy;

	int score = GetEnemyScoreM(num);

	for (int i = 0; i < 8; ++i)
	{
		if (score <= 0)
		{
			SetMaterial(0.8f, 0.8f, 0.8f, 0.8f);
			njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, 0, 0, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
		}
		else
		{
			njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, score % 10, -1.5f, NJD_SPRITE_ALPHA);
			score /= 10;
		}

		MULTIHUDDIGIT_SPRITE.p.x -= 18.0f * MULTIHUDDIGIT_SPRITE.sx;
	}
}

void MultiHudTime(__int8 minutes, __int8 seconds, __int8 frames)
{
	MULTIHUDDIGIT_SPRITE.p.x = MULTIHUD_SPRITE.p.x + 60.0f * MULTIHUD_SPRITE.sx;
	MULTIHUDDIGIT_SPRITE.p.y = MULTIHUD_SPRITE.p.y + 2.0f * MULTIHUD_SPRITE.sy;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, minutes / 10, -1.5f, NJD_SPRITE_ALPHA);
	MULTIHUDDIGIT_SPRITE.p.x += 16 * MULTIHUD_SPRITE.sx;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, minutes % 10, -1.5f, NJD_SPRITE_ALPHA);
	MULTIHUDDIGIT_SPRITE.p.x += 16 * MULTIHUD_SPRITE.sx;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, 10, -1.5f, NJD_SPRITE_ALPHA);
	MULTIHUDDIGIT_SPRITE.p.x += 16 * MULTIHUD_SPRITE.sx;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, seconds / 10, -1.5f, NJD_SPRITE_ALPHA);
	MULTIHUDDIGIT_SPRITE.p.x += 16 * MULTIHUD_SPRITE.sx;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, seconds % 10, -1.5f, NJD_SPRITE_ALPHA);
	MULTIHUDDIGIT_SPRITE.p.x += 16 * MULTIHUD_SPRITE.sx;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, 10, -1.5f, NJD_SPRITE_ALPHA);
	MULTIHUDDIGIT_SPRITE.p.x += 16 * MULTIHUD_SPRITE.sx;
	int timerms = static_cast<int>(static_cast<float>(frames) * 1.6666666f);
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, timerms % 10, -1.5f, NJD_SPRITE_ALPHA);
	MULTIHUDDIGIT_SPRITE.p.x += 16 * MULTIHUD_SPRITE.sx;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, timerms % 10, -1.5f, NJD_SPRITE_ALPHA);
}

void MultiHudRings(int num)
{
	MULTIHUDDIGIT_SPRITE.p.x = MULTIHUD_SPRITE.p.x;
	MULTIHUDDIGIT_SPRITE.p.y = MULTIHUD_SPRITE.p.y + 1.5f * MULTIHUD_SPRITE.sx;

	int count = GetNumRingM(num);

	float color = 1.0f;

	if (count == 0)
	{
		if (!IsGamePaused())
		{
			ringtimer[num] += 1024;
		}

		float sin = njSin(ringtimer[num]);
		color -= sin * sin;
	}

	SetMaterial(1.0f, 1.0f, color, color);

	MULTIHUDDIGIT_SPRITE.p.x += 25.0f * MULTIHUD_SPRITE.sx;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, count % 1000 / 100, 0.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	MULTIHUDDIGIT_SPRITE.p.x += 18.0f * MULTIHUD_SPRITE.sx;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, count % 100 / 10, 0.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
	MULTIHUDDIGIT_SPRITE.p.x += 18.0f * MULTIHUD_SPRITE.sx;
	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, count % 10, 0.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
}

void MultiHudLives(int num)
{
	if (!IsIngame())
		return;

	njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, TASKWK_CHARID(playertwp[num]) + 12, 0, NJD_SPRITE_ALPHA);

	if (HideHud >= 0)
	{
		int count = GetNumPlayerM(num);

		MULTIHUDDIGIT_SPRITE.p.y -= 25.0f * MULTIHUDDIGIT_SPRITE.sy;
		MULTIHUDDIGIT_SPRITE.p.x += 35.0f * MULTIHUDDIGIT_SPRITE.sx;
		njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, count % 100 / 10, 0.0f, NJD_SPRITE_ALPHA);
		MULTIHUDDIGIT_SPRITE.p.x += 18.0f * MULTIHUDDIGIT_SPRITE.sx;
		njDrawSprite2D_ForcePriority(&MULTIHUDDIGIT_SPRITE, count % 10, 0.0f, NJD_SPRITE_ALPHA);
	}
}

void DisplayMultiHud(int num)
{
	if (MissedFrames || IsGamePaused() || !IsIngame() || CurrentLevel == LevelIDs_TwinkleCircuit || CurrentCharacter == Characters_Big)
	{
		return;
	}

	if (HideTimerAndRings < 0 && HideHud < 0)
	{
		return;
	}

	SplitScreen::SaveViewPort();
	SplitScreen::ChangeViewPort(-1);

	SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

	auto ratio = SplitScreen::GetScreenRatio(num);

	float screenX = HorizontalResolution * ratio->x;
	float screenY = VerticalResolution * ratio->y;
	float scaleY = VerticalStretch * ratio->h;
	float scale = min(HorizontalStretch * ratio->w, scaleY);

	ghDefaultBlendingMode();

	MULTIHUD_SPRITE.sx = MULTIHUD_SPRITE.sy = scale;
	MULTIHUDDIGIT_SPRITE.sx = MULTIHUDDIGIT_SPRITE.sy = scale;

	float x = MULTIHUD_SPRITE.p.x = 16.0f * scale + screenX;

	if (HideTimerAndRings >= 0 && (!isInHubWorld() && !IsLevelChaoGarden()))
	{
		MULTIHUD_SPRITE.p.y = 16.0f * scaleY + screenY;
		njDrawSprite2D_ForcePriority(&MULTIHUD_SPRITE, MHudSprt_Score, 0, NJD_SPRITE_ALPHA);
		MultiHudScore(num);

		if (SplitScreen::numScreen == 0 )
		{
			MULTIHUD_SPRITE.p.x = x;
			MULTIHUD_SPRITE.p.y += 24 * scale;
			njDrawSprite2D_ForcePriority(&MULTIHUD_SPRITE, MHudSprt_Time, 0, NJD_SPRITE_ALPHA);
			MultiHudTime(TimeMinutes, TimeSeconds, TimeFrames);
		}

		MULTIHUD_SPRITE.p.x = x;
		MULTIHUD_SPRITE.p.y += 24 * scale;
		njDrawSprite2D_ForcePriority(&MULTIHUD_SPRITE, MHudSprt_Ring, 0, NJD_SPRITE_ALPHA);
		MultiHudRings(num);
	}

	if (HideLives >= 0)
	{
		MULTIHUDDIGIT_SPRITE.p.x = x;
		MULTIHUDDIGIT_SPRITE.p.y = VerticalResolution * ratio->h - 16.0f * scaleY + screenY;
		MultiHudLives(num);
	}

	if (GetLevelType() == 1)
	{
		HideLives = -1;
		HideTimerAndRings = -1;
	}
	else
	{
		HideLives = HideHud;
		HideTimerAndRings = HideHud;
	}

	ResetMaterial();
	SplitScreen::RestoreViewPort();
}

void __cdecl DisplayScore_r()
{
	if (!multiplayer::IsActive())
	{
		TARGET_DYNAMIC(DisplayScore)();
	}
}

void __cdecl DisplayTimer_r()
{
	if (!multiplayer::IsActive())
	{
		TARGET_DYNAMIC(DisplayTimer)();
	}
}

void MultiHudInit()
{
	DisplayScore_t = new Trampoline(0x425F90, 0x425F95, DisplayScore_r);
	DisplayTimer_t = new Trampoline(0x427F50, 0x427F55, DisplayTimer_r);
	LoadTextureForEachGameMode_t = new Trampoline(0x4212E0, 0x4212E5, LoadTextureForEachGameMode_r);
	ReleaseTextureForEachGameMode_t = new Trampoline(0x420F40, 0x420F45, ReleaseTextureForEachGameMode_r);
}
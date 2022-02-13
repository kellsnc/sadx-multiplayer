#include "pch.h"
#include "splitscreen.h"
#include "players.h"
#include "../include/multiapi.h"

bool splitscreen_is_active()
{
	return SplitScreen::IsActive();
}

bool viewport_is_enabled(int32_t num)
{
	return SplitScreen::IsScreenEnabled(num);
}

void viewport_save()
{
	SplitScreen::SaveViewPort();
}

void viewport_restore()
{
	SplitScreen::RestoreViewPort();
}

void viewport_change(int32_t num)
{
	SplitScreen::ChangeViewPort(num);
}

uint32_t viewport_get_num()
{
	return SplitScreen::GetCurrentScreenNum();
}

bool viewport_get_info(int32_t num, float* x, float* y, float* w, float* h)
{
	auto ratio = SplitScreen::GetScreenRatio(num);

	if (ratio)
	{
		*x = ratio->x;
		*y = ratio->y;
		*w = ratio->w;
		*h = ratio->h;

		return true;
	}

	return false;
}

void multi_set_charid(int32_t pnum, Characters character)
{
	SetCurrentCharacter(pnum, character);
}

void multi_score_reset()
{
	ResetEnemyScoreM();
}

int32_t multi_score_get(int32_t pnum)
{
	return GetEnemyScoreM(pnum);
}

void multi_score_add(int32_t pnum, int32_t amount)
{
	AddEnemyScoreM(pnum, amount);
}

void multi_lives_reset()
{
	ResetNumPlayerM();
}

int32_t multi_lives_get(int32_t pnum)
{
	return GetNumPlayerM(pnum);
}

void multi_lives_add(int32_t pnum, int32_t amount)
{
	AddNumPlayerM(pnum, amount);
}

void multi_rings_reset(int32_t pnum)
{
	ResetNumRingP(pnum);
}

int32_t multi_rings_get(int32_t pnum)
{
	return GetNumRingM(pnum);
}

void multi_rings_add(int32_t pnum, int32_t amount)
{
	AddNumRingM(pnum, amount);
}
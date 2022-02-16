#include "pch.h"
#include "splitscreen.h"
#include "players.h"
#include "result.h"
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

bool multi_is_enabled()
{
	return multiplayer::IsEnabled();
}

bool multi_is_active()
{
	return multiplayer::IsActive();
}

bool multi_is_battle()
{
	return multiplayer::IsBattleMode();
}

bool multi_is_coop()
{
	return multiplayer::IsCoopMode();
}

void multi_enable(int player_count, bool battle)
{
	multiplayer::Enable(player_count, battle ? multiplayer::mode::battle : multiplayer::mode::coop);
}

void multi_disable()
{
	multiplayer::Disable();
}

uint32_t multi_get_player_count()
{
	return multiplayer::GetPlayerCount();
}

int32_t multi_get_winner()
{
	return GetWinnerMulti();
}

void multi_set_winner(int32_t pnum)
{
	SetWinnerMulti(pnum);
}
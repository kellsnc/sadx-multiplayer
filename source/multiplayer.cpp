#include "pch.h"
#include "splitscreen.h"
#include "multiplayer.h"

namespace multiplayer
{
	bool enabled = false;
	unsigned int pcount = 0;
	mode gMode = mode::coop;
	bool online = false;

	unsigned int GetPlayerCount()
	{
		return pcount;
	}

	bool Enable(int player_count, mode md)
	{
		if (!enabled && player_count > 1 && player_count <= PLAYER_MAX)
		{
			gMode = md;
			enabled = true;
			pcount = player_count;
			SplitScreen::Enable();
			return true;
		}
		return false;
	}

	bool Disable()
	{
		if (enabled)
		{
			enabled = false;
			pcount = 0;
			ResetCharactersArray();
			SplitScreen::Disable();
			return true;
		}
		return false;
	}

	bool IsEnabled()
	{
		return enabled;
	}

	bool IsActive()
	{
		return IsEnabled() && (GameMode != GameModes_Menu) && pcount > 1;
	}

	bool IsBattleMode()
	{
		return IsActive() && gMode == mode::battle;
	}

	bool IsCoopMode()
	{
		return IsActive() && gMode == mode::coop;
	}

	bool IsFightMode()
	{
		return IsBattleMode() && CurrentLevel >= LevelIDs_Chaos0 && CurrentLevel <= LevelIDs_E101R;
	}

	bool IsAdventureMode()
	{
		return IsActive && ulGlobalMode == MD_ADVENTURE;
	}

	bool IsOnline()
	{
		return online;
	}
}
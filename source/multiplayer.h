#pragma once

namespace multiplayer
{
	enum class mode
	{
		coop,
		battle
	};

	unsigned int GetPlayerCount();

	void Enable(int player_count, mode md);
	void TemporaryDisable();
	void Disable();

	bool IsEnabled(); // Check if multiplayer is enabled
	bool IsActive(); // Check if multiplayer is active (ingame, not in menu)
	bool IsBattleMode(); // Check if multiplayer is active and in battle mode
	bool IsCoopMode(); // Check if multiplayer is active and in coop mode
	bool IsFightMode(); // Check if multiplayer is active and in VS mode
	bool IsOnline(); // Check if in networking mode (todo: move)
}
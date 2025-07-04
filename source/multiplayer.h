#pragma once

namespace multiplayer
{
	enum class mode
	{
		coop,
		battle,
		fight
	};

	unsigned int GetPlayerCount();

	bool Enable(int player_count, mode md);
	bool Disable();

	bool IsEnabled(); // Check if multiplayer is enabled
	bool IsActive(); // Check if multiplayer is active (ingame, not in menu)
	bool IsBattleMode(); // Check if multiplayer is active and in battle mode
	bool IsCoopMode(); // Check if multiplayer is active and in coop mode
	bool IsFightMode(); // Check if multiplayer is active and in VS mode
	bool IsOnline(); // Check if in networking mode (todo: move)

	void SetCharacter(int pnum, int character);
	int GetCharacter(int pnum);
	void ResetCharacters();
}

void GetPlayerInitialPositionM(NJS_POINT3* pos, Angle3* ang);
void SetOtherPlayers();

void ResetEnemyScoreM();
int GetEnemyScoreM(int pNum);
void AddEnemyScoreM(int pNum, int amount);
void SetEnemyScoreM(int pNum, int Number);

void __cdecl ResetNumPlayerM();
void AddNumPlayerM(int pNum, int amount);
void SetNumPlayerM(int pNum, int Number);
int GetNumPlayerM(int pNum);

void AddNumRingM(int pNum, int amount);
void SetNumRingM(int pNum, int Number);
int GetNumRingM(int pNum);
void ResetNumRingP(int pNum);

void ExecMultiplayer();
void InitMultiplayer();
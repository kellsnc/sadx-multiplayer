#pragma once

extern unsigned int player_count;
bool IsMultiplayerEnabled();

void ResetEnemyScoreM();
int GetEnemyScoreM(int pNum);
void AddEnemyScoreM(int pNum, int amount);

void __cdecl ResetNumPlayerM();
void AddNumPlayerM(int pNum, int amount);
int GetNumPlayerM(int pNum);

void AddNumRingM(int pNum, int amount);
int GetNumRingM(int pNum);

void UpdatePlayersInfo();

void __cdecl InitMultiplayer();
void SetCurrentCharacter(int pnum, int character);
int GetCurrentCharacter(int pnum);
void ResetCharactersArray();
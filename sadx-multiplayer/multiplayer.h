#pragma once

extern unsigned int player_count;
bool IsMultiplayerEnabled();

void ResetScoreM();
int GetScoreM(int pNum);
void AddScoreM(int pNum, int amount);

void __cdecl ResetLivesM();
void SetLivesM(int pNum, int amount);
int GetLivesM(int pNum);

void AddRingsM(int pNum, int amount);
int GetRingsM(int pNum);

void RingsLives_OnFrames();

void __cdecl initPlayerHack();
void SetCurrentCharacter(int pnum, int character);
int GetCurrentCharacter(int pnum);
void ResetCharactersArray();
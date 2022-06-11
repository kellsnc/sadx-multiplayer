#pragma once

void ResetEnemyScoreM();
int GetEnemyScoreM(int pNum);
void AddEnemyScoreM(int pNum, int amount);

void __cdecl ResetNumPlayerM();
void AddNumPlayerM(int pNum, int amount);
int GetNumPlayerM(int pNum);

void AddNumRingM(int pNum, int amount);
int GetNumRingM(int pNum);
void ResetNumRingP(int pNum);

void GetPlayerInitialPositionM(NJS_POINT3* pos, Angle3* ang);

void SetCurrentCharacter(int pnum, Characters character);
int GetCurrentCharacter(int pnum);
void ResetCharactersArray();

void UpdatePlayersInfo();

void InitPlayerPatches();
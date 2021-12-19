#pragma once

void __cdecl ResetLivesM();
void SetLivesM(int pNum, int amount);
int GetLivesM(int pNum);

void AddRingsM(int pNum, int amount);
int GetRingsM(int pNum);

void RingsLives_OnFrames();

void __cdecl initPlayerHack();
void __cdecl SetCurrentCharacter(char pnum, char character);
__int16 __cdecl GetCurrentCharacter(char pnum);
void ResetCharactersArray();
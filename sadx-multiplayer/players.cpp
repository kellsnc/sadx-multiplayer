#include "pch.h"

static int rings[8];
static int lives[8];

void __cdecl ResetLivesM()
{
    Lives = 4;

    for (int i = 0; i < PLAYER_MAX; i++)
    {
        lives[i] = 4;
    }
}

int GetLivesM(int pNum)
{
    return lives[pNum];
}

void SetLivesM(int pNum, int amount)
{
    if (amount > 0)
    {
        PlaySound(743, 0, 0, 0);
    }

    lives[pNum] += amount;

    if ((lives[pNum] < 0 && amount > 0) || amount == CHAR_MAX)
    {
        lives[pNum] = CHAR_MAX;
    }

    if (GetLevelType() == 1)
    {
        LoadObject(LoadObj_UnknownB, 6, sub_425B30);
    }

    if (!pNum)
        Lives = lives[pNum];
}

int GetRingsM(int pNum)
{
    return rings[pNum];
}

void AddRingsM(int pNum, int amount)
{
    int origc = rings[pNum] / 100;
    rings[pNum] += amount;

    int newc = rings[pNum] / 100;

    if (origc < newc)
    {
        SetLivesM(pNum, newc - origc);
    }

    if (GetLevelType() == 1)
    {
        LoadObject(LoadObj_UnknownB, 6, sub_425BB0);
    }
}

void RingsLives_OnFrames()
{
    rings[0] = Rings;
    lives[0] = Lives;
}

void __cdecl initPlayerHack()
{
    WriteJump(ResetLives, ResetLivesM);
}
#include "pch.h"


static Sint16 rings[8];
static char lives[8];

ObjectFunc(sub_425B30, 0x425B30);
ObjectFunc(sub_425BB0, 0x425BB0);

__int16 GetLives_r(char pNum)
{
    return lives[pNum];
}

void __cdecl GiveLives_r(char pNum, char live)
{
    if (live > 0)
    {
        PlaySound(743, 0, 0, 0);
    }
    lives[pNum] += live;
    if (lives[pNum] < 0 && live > 0)
    {
        lives[pNum] = 127;
    }
    if (GetLevelType() == 1)
    {
        LoadObject(LoadObj_UnknownB, 6, sub_425B30);
    }
}

__int16 GetRings_r(char pNum)
{
    return rings[pNum];
}

void __cdecl AddRings_r(char pNum, Sint16 amount)
{
    int calcRing;
    int result;

    calcRing = rings[pNum] / 100;
    rings[pNum] += amount;

    result = rings[pNum] / 100;
    if (calcRing < result)
    {
        GiveLives_r(pNum, result - calcRing);
    }
    if (GetLevelType() == 1)
    {
        LoadObject(LoadObj_UnknownB, 6, sub_425BB0);
    }
}

void RingsLives_OnFrames() {

    rings[0] = Rings;
    lives[0] = Lives;
}
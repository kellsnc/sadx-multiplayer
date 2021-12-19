#include "pch.h"

static int rings[8];
static int lives[8];
static __int16 characters[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }; //don't remove the -1 or it will break test spawn

ObjectFuncPtr charfuncs[] = {
    Sonic_Main,
    Eggman_Main,
    Tails_Main,
    Knuckles_Main,
    Tikal_Main,
    Amy_Main,
    Gamma_Main,
    Big_Main
};

void ResetCharactersArray() {
    for (int i = 0; i < 8; i++) {
        characters[i] = -1;
    }
}

void LoadCharObj(char pnum, char character)
{
    ObjectMaster* player = nullptr;
    player = LoadObject((LoadObj)(LoadObj_UnknownA | LoadObj_Data1 | LoadObj_Data2), 1, charfuncs[character]);
    player->Data1->CharID = character;
    player->Data1->CharIndex = pnum;
    EntityData1Ptrs[pnum] = player->Data1;
    EntityData2Ptrs[pnum] = (EntityData2*)player->Data2;
    return;
}

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

void __cdecl SetCurrentCharacter(char pnum, char character)
{
    characters[pnum] = character;
    return;
}

__int16 __cdecl GetCurrentCharacter(char pnum)
{
    return characters[pnum];
}

void Load_MultipleCharacters() {

    for (uint8_t i = 1; i < 8; i++) {

        if (characters[i] != -1)
            LoadCharObj(i, characters[i]);
    }

    return LoadCharacter();
}
void __cdecl initPlayerHack()
{
    WriteJump(ResetLives, ResetLivesM);
    WriteCall((void*)0x415A25, Load_MultipleCharacters);
    return;
}


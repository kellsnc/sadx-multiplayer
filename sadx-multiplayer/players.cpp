#include "pch.h"

static int rings[PLAYER_MAX];
static int lives[PLAYER_MAX];
static int score[PLAYER_MAX];
static __int16 characters[PLAYER_MAX] = { -1, -1, -1, -1 };

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
    for (int i = 0; i < PLAYER_MAX; i++) {
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

void ResetScoreM()
{
    for (auto& s : score)
    {
        s = 0;
    }
}

int GetScoreM(int pNum)
{
    return score[pNum];
}

void AddScoreM(int pNum, int amount)
{
    score[pNum] += amount;
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

    player_count = 0;

    LoadCharacter();
    player_count++; //count player one


    for (uint8_t i = 1; i < PLAYER_MAX; i++) {

        if (characters[i] > -1) {
            LoadCharObj(i, characters[i]);
            player_count++;
        }
    }

    if (TailsAI_ptr) //temporary so we can make test 
        player_count++;


    return;
}
void __cdecl initPlayerHack()
{
    WriteJump(ResetLives, ResetLivesM);
    WriteCall((void*)0x415A25, Load_MultipleCharacters);
    return;
}


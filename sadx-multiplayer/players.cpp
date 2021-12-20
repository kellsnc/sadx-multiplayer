#include "pch.h"

unsigned int player_count = 0;

static int rings[PLAYER_MAX];
static int lives[PLAYER_MAX];
static int score[PLAYER_MAX];
static int characters[PLAYER_MAX] = { -1, -1, -1, -1 };

TaskFuncPtr charfuncs[] = {
    (TaskFuncPtr)Sonic_Main,
    (TaskFuncPtr)Eggman_Main,
    (TaskFuncPtr)Tails_Main,
    (TaskFuncPtr)Knuckles_Main,
    (TaskFuncPtr)Tikal_Main,
    (TaskFuncPtr)Amy_Main,
    (TaskFuncPtr)Gamma_Main,
    (TaskFuncPtr)Big_Main
};

bool IsMultiplayerEnabled()
{
    return (IsIngame() || IsGamePaused()) && player_count > 1;
}

void ResetCharactersArray()
{
    for (int i = 0; i < PLAYER_MAX; i++)
    {
        characters[i] = -1;
    }
}

void LoadCharObj(char pnum, char character)
{
    task* tp = CreateElementalTask((LoadObj_UnknownA | LoadObj_Data1 | LoadObj_Data2), LEV_1, charfuncs[character]);
    TASKWK_CHARID(tp->twp) = character;
    TASKWK_PLAYERID(tp->twp) = pnum;
    playertwp[pnum] = tp->twp;
    playermwp[pnum] = (motionwk2*)tp->mwp;
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
    score[0] = Score + EnemyBonus;
}

void __cdecl SetCurrentCharacter(char pnum, char character)
{
    characters[pnum] = character;
}

__int16 __cdecl GetCurrentCharacter(char pnum)
{
    return characters[pnum];
}

void Load_MultipleCharacters()
{
    player_count = 0;

    LoadCharacter();
    player_count++; //count player one

    for (int i = 1; i < PLAYER_MAX; i++)
    {
        if (characters[i] > -1)
        {
            LoadCharObj(i, characters[i]);
            player_count++;
        }
    }

    if (TailsAI_ptr) //temporary so we can make test 
        player_count++;
}

void __cdecl initPlayerHack()
{
    WriteJump(ResetLives, ResetLivesM);
    WriteCall((void*)0x415A25, Load_MultipleCharacters);
}
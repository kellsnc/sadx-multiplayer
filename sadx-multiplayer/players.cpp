#include "pch.h"
#include "players.h"
#include "multiplayer.h"

/*

Multiplayer manager
- Extends player variables
- Load available players
- GameStates

*/

Trampoline* DamegeRingScatter_t = nullptr;

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

void LoadCharObj(int pnum, int character)
{
    task* tp = CreateElementalTask((LoadObj_UnknownA | LoadObj_Data1 | LoadObj_Data2), LEV_1, charfuncs[character]);
    TASKWK_CHARID(tp->twp) = character;
    TASKWK_PLAYERID(tp->twp) = pnum;
    playertwp[pnum] = tp->twp;
    playermwp[pnum] = (motionwk2*)tp->mwp;
    SetPlayerInitialPosition(tp->twp);
}

void ResetEnemyScoreM()
{
    InitActionScore();

    for (auto& s : score)
    {
        s = 0;
    }
}

int GetEnemyScoreM(int pNum)
{
    if (multiplayer::IsBattleMode())
    {
        return score[pNum];
    }
    else
    {
        return slEnemyScore;
    }
}

void AddEnemyScoreM(int pNum, int add)
{
    if (multiplayer::IsBattleMode())
    {
        score[pNum] += add;

        if (pNum == 0)
        {
            slEnemyScore = score[0];
        }
    }
    else
    {
        AddEnemyScore(add);
    }
}

void __cdecl ResetNumPlayerM()
{
    scNumPlayer = 4;

    for (int i = 0; i < PLAYER_MAX; i++)
    {
        lives[i] = 4;
    }
}

int GetNumPlayerM(int pNum)
{
    if (multiplayer::IsBattleMode())
    {
        return lives[pNum];
    }
    else
    {
        return GetNumPlayer();
    }
}

void AddNumPlayerM(int pNum, int Number)
{
    if (multiplayer::IsBattleMode())
    {
        if (Number > 0)
        {
            PlaySound(743, 0, 0, 0);
        }

        lives[pNum] += Number;

        if ((lives[pNum] < 0 && Number > 0) || Number >= CHAR_MAX)
        {
            lives[pNum] = CHAR_MAX;
        }

        if (GetLevelType() == 1)
        {
            LoadObject(LoadObj_UnknownB, 6, sub_425B30);
        }

        if (pNum == 0)
        {
            scNumPlayer = lives[0];
        }
    }
    else
    {
        AddNumPlayer(Number);
    }
}

int GetNumRingM(int pNum)
{
    if (multiplayer::IsBattleMode())
    {
        return rings[pNum];
    }
    else
    {
        return GetNumRing();
    }
}

void AddNumRingM(int pNum, int add)
{
    if (multiplayer::IsBattleMode())
    {
        int origc, newc = 0;

        origc = rings[pNum] / 100;
        rings[pNum] += add;
        newc = rings[pNum] / 100;

        if (pNum == 0)
        {
            ssNumRing = rings[0];
        }

        if (origc < newc)
        {
            AddNumPlayerM(pNum, newc - origc);
        }

        if (GetLevelType() == 1)
        {
            LoadObject(LoadObj_UnknownB, 6, sub_425BB0);
        }
    }
    else
    {
        AddNumRing(add);
    }
}

void ResetNumRingP(int pNum)
{
    if (multiplayer::IsBattleMode())
    {
        rings[pNum] = 0;
    }
    
    if (pNum == 0)
    {
        Rings = 0;
    }
}

void __cdecl DamegeRingScatter_r(uint8_t pno)
{
    if (multiplayer::IsBattleMode())
    {
        auto rings = GetNumRingM(pno);

        if (rings > 0)
        {
            ResetNumRingP(pno);

            float v6 = (float)((double)rand() * 0.000030517578 * 360.0);

            for (int i = 0; i < min(20, rings); ++i)
            {
                auto tp = CreateElementalTask(LoadObj_UnknownB | LoadObj_Data1, 2, (TaskFuncPtr)0x44FD10);
                tp->twp->pos = playertwp[pno]->pos;
                tp->twp->ang.y = (Angle)((((i * 350) / rings) + v6) * 65536.0 * 0.002777777777777778);
            }

            dsPlay_oneshot(0, 0, 0, 0);
        }
        else
        {
            KillHimP(pno);

            if (TASKWK_CHARID(playertwp[pno]) == Characters_Gamma)
            {
                dsPlay_oneshot(1431, 0, 0, 0);
            }
            else
            {
                dsPlay_oneshot(23, 0, 0, 0);
            }
        }
    }
    else
    {
        TARGET_DYNAMIC(DamegeRingScatter)(pno);
    }
}

// Remove ability to be hurt by players
void RemovePlayersDamage(taskwk* twp)
{
    if (twp && twp->cwp)
    {
        for (int i = 0; i < twp->cwp->nbInfo; i++)
        {
            twp->cwp->info[i].damage &= ~0x20u;
        }
    }
}

void SetPlayerTargetable(taskwk* twp)
{
    if (twp->cwp)
    {
        twp->cwp->flag |= 0x40;
        twp->cwp->id = 3;
    }
}

void UpdatePlayersInfo()
{
    rings[0] = ssNumRing;
    lives[0] = scNumPlayer;
    score[0] = slEnemyScore;

    if (PressedButtons[1] & Buttons_L)
    {
        playertwp[1]->pos = playertwp[0]->pos;
    }

    if (IsIngame())
    {
        bool coop = multiplayer::IsCoopMode();
        bool vs = multiplayer::IsFightMode();

        if (coop || vs)
        {
            for (int i = 0; i < PLAYER_MAX; ++i)
            {
                if (playertwp[i])
                {
                    if (coop) RemovePlayersDamage(playertwp[i]);
                    if (vs) SetPlayerTargetable(playertwp[i]);
                }
            }
        }
    }
}

void ResetCharactersArray()
{
    for (int i = 0; i < PLAYER_MAX; i++)
    {
        characters[i] = -1;
    }
}

void SetCurrentCharacter(int pnum, int character)
{
    characters[pnum] = character;
}

int GetCurrentCharacter(int pnum)
{
    return characters[pnum];
}

void LoadCharacter_r()
{
    if (multiplayer::IsActive())
    {
        if (characters[0] >= 0)
            CurrentCharacter = characters[0];

        TailsAI_ptr = (ObjectMaster*)1; // don't load tails AI; horrible patch for compatibility with CharSel
        LoadCharacter();
        TailsAI_ptr = nullptr;

        for (int i = 1; i < multiplayer::GetPlayerCount(); i++)
        {
            if (characters[i] >= 0)
            {
                LoadCharObj(i, characters[i]);
            }
        }
    }
    else
    {
        LoadCharacter();
    }
}

void InitPlayerPatches()
{
    WriteJump(ResetNumPlayer, ResetNumPlayerM);
    WriteCall((void*)0x415A25, LoadCharacter_r);
    DamegeRingScatter_t = new Trampoline(0x4506F0, 0x4506F7, DamegeRingScatter_r);
}
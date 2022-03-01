#include "pch.h"
#include "players.h"
#include "multiplayer.h"
#include "result.h"

/*

Multiplayer manager
- Extends player variables
- Load available players
- Patch start positions
- GameStates

*/

DataPointer(GM_START_POSANG*, paSonicIP_Ptr, 0x41491E);
DataPointer(GM_START_POSANG*, paMilesIP_Ptr, 0x414925);
DataPointer(GM_START_POSANG*, paKnucklesIP_Ptr, 0x41492C);
DataPointer(GM_START_POSANG*, paAmyIP_Ptr, 0x41493A);
DataPointer(GM_START_POSANG*, paE102IP_Ptr, 0x414941);
DataPointer(GM_START_POSANG*, paBigIP_Ptr, 0x414933);

DataPointer(ADVPOS**, vInitialPositionSS_Ptr, 0x62F6EE);
DataPointer(ADVPOS**, vInitialPositionEC_AB_Ptr, 0x52D854);
DataPointer(ADVPOS**, vInitialPositionEC_C_Ptr, 0x52D861);
DataPointer(ADVPOS**, vInitialPositionMR_Ptr, 0x5307AE);
DataPointer(ADVPOS**, vInitialPositionPast_Ptr, 0x54219E);

Trampoline* SetPlayerInitialPosition_t = nullptr;
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
        return EnemyScore;
    }
}

void AddEnemyScoreM(int pNum, int add)
{
    if (multiplayer::IsBattleMode())
    {
        score[pNum] += add;

        if (pNum == 0)
        {
            EnemyScore = score[0];
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

    for (auto& i : lives)
    {
        i = 4;
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

void ResetNumRingM()
{
    Rings = 0;

    if (multiplayer::IsBattleMode())
    {
        for (auto& i : rings)
        {
            i = 0;
        }
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

void GetPlayerInitialPositionM(NJS_POINT3* pos, Angle3* ang)
{
    if (CheckContinueData())
    {
        *pos = continue_data.pos;
        *ang = continue_data.ang;
    }
    else
    {
        if (FieldStartPos)
        {
            *pos = FieldStartPos->Position;
            *ang = { 0, FieldStartPos->YRot, 0 };
            FieldStartPos = nullptr;
        }
        else if (ssStageNumber >= LevelIDs_StationSquare && ssStageNumber <= LevelIDs_Past)
        {
            ADVPOS** adpos;

            // Adv Field:
            switch (ssStageNumber)
            {
            default:
            case LevelIDs_StationSquare:
            case 27:
            case 28:
                adpos = vInitialPositionSS_Ptr;
                break;
            case LevelIDs_EggCarrierOutside:
                adpos = vInitialPositionEC_AB_Ptr;
                break;
            case LevelIDs_EggCarrierInside:
                adpos = vInitialPositionEC_C_Ptr;
                break;
            case LevelIDs_MysticRuins:
                adpos = vInitialPositionMR_Ptr;
                break;
            case LevelIDs_Past:
                adpos = vInitialPositionPast_Ptr;
                break;
            }

            *pos = adpos[ssActNumber]->pos;
            *ang = { 0, adpos[ssActNumber]->angy, 0 };
        }
        else
        {
            GM_START_POSANG* stpos;

            switch (CurrentCharacter)
            {
            default:
            case Characters_Sonic:
                stpos = paSonicIP_Ptr;
                break;
            case Characters_Tails:
                stpos = paMilesIP_Ptr;
                break;
            case Characters_Knuckles:
                stpos = paKnucklesIP_Ptr;
                break;
            case Characters_Amy:
                stpos = paAmyIP_Ptr;
                break;
            case Characters_Gamma:
                stpos = paE102IP_Ptr;
                break;
            case Characters_Big:
                stpos = paBigIP_Ptr;
                break;
            }

            while (stpos->stage != CurrentLevel || stpos->act != CurrentAct)
            {
                if (stpos->stage == LevelIDs_Invalid)
                {
                    *pos = { 0.0f, 0.0f, 0.0f };
                    *ang = { 0, 0, 0 };

                    return;
                }
                ++stpos;
            }

            *pos = continue_data.pos = stpos->p;
            *ang = continue_data.ang = { 0, stpos->angy, 0 };
            continue_data.continue_flag = TRUE;
        }
    }
}

void __cdecl SetPlayerInitialPosition_r(taskwk* twp)
{
    if (multiplayer::IsActive())
    {
        NJS_POINT3 pos; Angle3 ang;
        GetPlayerInitialPositionM(&pos, &ang);

        static const int dists[]{ -5.0f, 5.0f, -10.0f, 10.0f };
        twp->ang = ang;
        twp->pos.x = pos.x + njCos(ang.y + 0x4000) * dists[TASKWK_PLAYERID(twp)];
        twp->pos.y = pos.y;
        twp->pos.z = pos.z + njSin(ang.y + 0x4000) * dists[TASKWK_PLAYERID(twp)];
    }
    else
    {
        TARGET_DYNAMIC(SetPlayerInitialPosition)(twp);
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
    score[0] = EnemyScore;

#ifdef _DEBUG
    if (PressedButtons[1] & Buttons_L)
    {
        playertwp[1]->pos = playertwp[0]->pos;
    }
#endif

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

void SetCurrentCharacter(int pnum, Characters character)
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

        SetWinnerMulti(-1);

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

        SetAllPlayersInitialPosition();
    }
    else
    {
        LoadCharacter();
    }
}

void InitPlayerPatches()
{
    SetPlayerInitialPosition_t = new Trampoline(0x414810, 0x414815, SetPlayerInitialPosition_r);
    
    DamegeRingScatter_t = new Trampoline(0x4506F0, 0x4506F7, DamegeRingScatter_r);
    WriteCall((void*)((int)DamegeRingScatter_t->Target() + 2), rand); // Patch trampoline

    WriteCall((void*)0x415A25, LoadCharacter_r);

    WriteJump(ResetNumPlayer, ResetNumPlayerM);
    WriteJump(ResetNumRing, ResetNumRingM);
}
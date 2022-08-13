#include "pch.h"
#include "network.h"
#include "timer.h"
#include "multiplayer.h"
#include "result.h"
#include "milesrace.h"
#include "hud_indicator.h"
#include "players.h"

/*

Multiplayer manager
- Extends player variables
- Load available players
- Patch start positions
- GameStates

*/

static Timer update_timer(std::chrono::steady_clock::duration(std::chrono::milliseconds(100)));

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
Trampoline* SetPlayer_t = nullptr;

static bool isCharSel = false;

VariableHook<int16_t, 0x3B0F0E4> ssNumRing_m;
VariableHook<int8_t, 0x3B0EF34> scNumPlayer_m;
VariableHook<int32_t, 0x3B0F104> EnemyScore_m;

static int characters[PLAYER_MAX] = { -1, -1, -1, -1 };

static constexpr uint16_t FLAG_MASK = Status_Ball | Status_Attack;

TaskFuncPtr charfuncs[] = {
    SonicTheHedgehog,
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
    ssNumRing_m.clear();
}

int GetEnemyScoreM(int pNum)
{
    return EnemyScore_m[pNum];
}

void AddEnemyScoreM(int pNum, int add)
{
    EnemyScore_m[pNum] += add;
}

void SetEnemyScoreM(int pNum, int Number)
{
    EnemyScore_m[pNum] = Number;
}

void __cdecl ResetNumPlayerM()
{
    for (auto& i : scNumPlayer_m)
    {
        i = 4;
    }
}

int GetNumPlayerM(int pNum)
{
    return scNumPlayer_m[pNum];
}

void AddNumPlayerM(int pNum, int Number)
{
    if (multiplayer::IsBattleMode())
    {
        if (Number > 0)
        {
            PlaySound(743, 0, 0, 0);
        }

        auto& counter = scNumPlayer_m[pNum];

        counter += Number;

        if ((counter < 0 && Number > 0) || Number >= CHAR_MAX)
        {
            counter = CHAR_MAX;
        }

        if (GetLevelType() == 1)
        {
            LoadObject(LoadObj_UnknownB, 6, sub_425B30);
        }
    }
    else
    {
        AddNumPlayer(Number);
    }
}

void SetNumPlayerM(int pNum, int Number)
{
    scNumPlayer_m[pNum] = Number;
}

int GetNumRingM(int pNum)
{
    return ssNumRing_m[pNum];
}

void AddNumRingM(int pNum, int add)
{
    if (multiplayer::IsBattleMode())
    {
        int origc, newc = 0;

        auto& counter = ssNumRing_m[pNum];

        origc = counter / 100;
        counter += add;
        newc = counter / 100;

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

void SetNumRingM(int pNum, int Number)
{
    ssNumRing_m[pNum] = Number;
}

void ResetNumRingM()
{
    ssNumRing_m.clear();
}

void ResetNumRingP(int pNum)
{
    ssNumRing_m[pNum] = 0;
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
        auto pnum = TASKWK_PLAYERID(twp);

        NJS_POINT3 pos; Angle3 ang;
        GetPlayerInitialPositionM(&pos, &ang);

        if (pnum == 0 && multiplayer::IsCoopMode() && continue_data.continue_flag)
        {
            SetTime2(continue_data.minutes, continue_data.second, continue_data.frame);
        }

        static const float dists[]{ -5.0f, 5.0f, -10.0f, 10.0f };
        twp->ang = ang;
        twp->pos.x = pos.x + njCos(ang.y + 0x4000) * dists[pnum];
        twp->pos.y = pos.y;
        twp->pos.z = pos.z + njSin(ang.y + 0x4000) * dists[pnum];
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

            for (int i = 0; i < min(20, rings); ++i)
            {
                auto tp = CreateElementalTask(LoadObj_UnknownB | LoadObj_Data1, 2, (TaskFuncPtr)0x44FD10);
                tp->twp->pos = playertwp[pno]->pos;
                tp->twp->ang.y = NJM_DEG_ANG(((double)(i * 350.0) / (double)rings) + (njRandom() * 360.0));
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

static bool PlayerListener(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum)
{
    auto ptwp = playertwp[pnum];
    auto pmwp = playermwp[pnum];
    auto ppwp = playerpwp[pnum];

    if (!ptwp || !ppwp || !pmwp)
    {
        return false;
    }

    switch (type)
    {
    case Network::PACKET_PLAYER_LOCATION:
        packet >> ptwp->pos >> ptwp->ang >> ppwp->spd >> pmwp->spd;
        return true;
    case Network::PACKET_PLAYER_MODE:
        packet >> ptwp->mode;
        return true;
    case Network::PACKET_PLAYER_SMODE:
        packet >> ptwp->smode;
        ptwp->flag |= Status_DoNextAction;
        return true;
    case Network::PACKET_PLAYER_FLAG:
    {
        short flag;
        packet >> flag;
        ptwp->flag = (ptwp->flag & ~FLAG_MASK) | (flag & FLAG_MASK);
        return true;
    }
    case Network::PACKET_PLAYER_ANIM:
        packet >> ppwp->mj.mtnmode >> (ppwp->mj.mtnmode == 2 ? ppwp->mj.action : ppwp->mj.reqaction) >> ppwp->mj.nframe;
        return true;
    case Network::PACKET_PLAYER_RINGS:
        packet >> ssNumRing_m[pnum];
        return true;
    case Network::PACKET_PLAYER_LIVES:
        packet >> scNumPlayer_m[pnum];
        return true;
    case Network::PACKET_PLAYER_SCORE:
        packet >> EnemyScore_m[pnum];
        return true;
    default:
        return false;
    }
}

static bool PlayerSender(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum)
{
    auto ptwp = playertwp[pnum];
    auto ppwp = playerpwp[pnum];
    auto pmwp = playermwp[pnum];
    
    switch (type)
    {
    case Network::PACKET_PLAYER_LOCATION:
        packet << ptwp->pos << ptwp->ang << ppwp->spd << pmwp->spd;
        return true;
    case Network::PACKET_PLAYER_MODE:
        packet << ptwp->mode;
        return true;
    case Network::PACKET_PLAYER_SMODE:
        packet << ptwp->smode;
        return true;
    case Network::PACKET_PLAYER_FLAG:
        packet << (short)(ptwp->flag & FLAG_MASK);
        return true;
    case Network::PACKET_PLAYER_ANIM:
        packet << ppwp->mj.mtnmode << (ppwp->mj.mtnmode == 2 ? ppwp->mj.action : ppwp->mj.reqaction) << ppwp->mj.nframe;
        return true;
    case Network::PACKET_PLAYER_RINGS:
        packet << ssNumRing_m[pnum];
        return true;
    case Network::PACKET_PLAYER_LIVES:
        packet << scNumPlayer_m[pnum];
        return true;
    case Network::PACKET_PLAYER_SCORE:
        packet << EnemyScore_m[pnum];
        return true;
    default:
        return false;
    }
}

void UpdatePlayersInfo()
{
    if (network.IsConnected())
    {
        auto pnum = network.GetPlayerNum();
        auto ptwp = playertwp[pnum];
        auto ppwp = playerpwp[pnum];

        if (ptwp && ppwp)
        {
            if (update_timer.Finished())
            {
                network.Send(Network::PACKET_PLAYER_LOCATION, PlayerSender);
                network.Send(Network::PACKET_PLAYER_ANIM, PlayerSender);
                network.Send(Network::PACKET_PLAYER_FLAG, PlayerSender);
            }

            static char last_action = 0;
            if (last_action != ptwp->mode)
            {
                network.Send(Network::PACKET_PLAYER_MODE, PlayerSender);
                last_action = ptwp->mode;
            }

            if (ptwp->flag & Status_DoNextAction)
            {
                network.Send(Network::PACKET_PLAYER_SMODE, PlayerSender);
            }

            static short last_mtnmode = 0;
            static short last_mtnaction = 0;
            if (last_mtnmode != ppwp->mj.mtnmode || last_mtnaction != (last_mtnmode == 2 ? ppwp->mj.action : ppwp->mj.reqaction))
            {
                network.Send(Network::PACKET_PLAYER_ANIM, PlayerSender);
                last_mtnaction = (last_mtnmode == 2 ? ppwp->mj.action : ppwp->mj.reqaction);
                last_mtnmode = ppwp->mj.mtnmode;
            }

            static int16_t old_rings = 0;
            if (old_rings != ssNumRing_m[pnum])
            {
                network.Send(Network::PACKET_PLAYER_RINGS, PlayerSender);
                old_rings = ssNumRing_m[pnum];
            }

            static int8_t old_lives = 0;
            if (old_lives != scNumPlayer_m[pnum])
            {
                network.Send(Network::PACKET_PLAYER_LIVES, PlayerSender);
                old_lives = scNumPlayer_m[pnum];
            }

            static int32_t old_score = 0;
            if (old_score != EnemyScore_m[pnum])
            {
                network.Send(Network::PACKET_PLAYER_SCORE, PlayerSender);
                old_score = EnemyScore_m[pnum];
            }
        }
    }
    
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

void SetPlayer_r()
{
    if (multiplayer::IsActive())
    {
        // Load all characters:
        for (unsigned int i = 0ui32; i < multiplayer::GetPlayerCount(); i++)
        {
            int playernum = i == 0 && characters[0] < 0 ? CurrentCharacter : characters[i];

            if (playernum >= 0)
            {
                LoadCharObj(i, playernum);
            }
        }

        // Load game mechanics:
        switch (CurrentCharacter)
        {
        case Characters_Tails:
            if (multiplayer::IsCoopMode())
            {
                Set_NPC_Sonic_m(7); // load opponent into slot 7
            }
            break;
        case Characters_Knuckles:
            if (isCharSel && !EV_CheckCansel() && (ulGlobalMode == 4 || ulGlobalMode == 10 || ulGlobalMode == 9))
            {
                CreateElementalTask(2u, 6, Knuckles_KakeraGame);
            }
            break;
        case Characters_Big:
            if (isCharSel)
                CreateElementalTask(2u, 6, BigDisplayStatus);
            break;
        }

        CreateIndicatorP();
        SetWinnerMulti(-1);
        SetAllPlayersInitialPosition();
    }
    else
    {
        TARGET_DYNAMIC(SetPlayer)();
    }
}

void InitPlayerPatches()
{
    isCharSel = GetModuleHandle(L"SADXCharSel") != nullptr;

    SetPlayerInitialPosition_t = new Trampoline(0x414810, 0x414815, SetPlayerInitialPosition_r);
    
    DamegeRingScatter_t = new Trampoline(0x4506F0, 0x4506F7, DamegeRingScatter_r);
    WriteCall((void*)((int)DamegeRingScatter_t->Target() + 2), rand); // Patch trampoline

    SetPlayer_t = new Trampoline(0x4157C0, 0x4157C8, SetPlayer_r);
    WriteCall((void*)((int)SetPlayer_t->Target() + 3), (void*)0x43B920); // Patch trampoline
    WriteCall((void*)0x415A25, SetPlayer_r);

    WriteJump(ResetNumPlayer, ResetNumPlayerM);
    WriteJump(ResetNumRing, ResetNumRingM);

    network.RegisterListener(Network::PACKET_PLAYER_LOCATION, PlayerListener);
    network.RegisterListener(Network::PACKET_PLAYER_MODE, PlayerListener);
    network.RegisterListener(Network::PACKET_PLAYER_SMODE, PlayerListener);
    network.RegisterListener(Network::PACKET_PLAYER_FLAG, PlayerListener);
    network.RegisterListener(Network::PACKET_PLAYER_ANIM, PlayerListener);

    network.RegisterListener(Network::PACKET_PLAYER_RINGS, PlayerListener);
    network.RegisterListener(Network::PACKET_PLAYER_LIVES, PlayerListener);
    network.RegisterListener(Network::PACKET_PLAYER_SCORE, PlayerListener);
}
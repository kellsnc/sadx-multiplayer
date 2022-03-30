#include "pch.h"
#include "multiplayer.h"
#include "fishing.h"
#include "hud_fishing.h"
#include "BigKaeru.h"

#define SAKANA_PNUM(twp) twp->smode
#define MAX_FISH 5 * GetPlayerCount()

enum : __int8
{
    MODE_INIT,
    MODE_FREE,
    MODE_RETPOS,        // return to start position -> MODE_RETANG          <- symbol name?
    MODE_RETANG,        // return to start angle -> MODE_FREE               <- symbol name?
    MODE_TRACE,         // move fish toward the lure -> MODE_FISHING_ATARI
    MODE_FISHING_ATARI, // check if it's a hit -> fishing mode
    MODE_FISHING,       // fishing mode (normal)
    MODE_TURN,          // fishing mode (turning)                           <- symbol name?
    MODE_ESCAPE,        // fishing mode (resistance)                        <- symbol name?
    MODE_CATCHING       // fish is caught -> FREE TASK
};

static auto setSakana = GenerateUsercallWrapper<task* (*)(task* tp)>(rEAX, 0x597590, rEDI);
static auto motionSakana = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x596A40, rEDX);
static auto moveLureDirSakana = GenerateUsercallWrapper<BOOL (*)(task* tp, task* otp)>(rEAX, 0x596D40, rEAX, stack4);
static auto sub_596570 = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x596570, rEDX); // inlined in symbols

DataArray(NJS_ACTION*, sakana_hontai_tbl, 0x173B630, 16);
DataArray(NJS_ACTION*, sakana_kuituki_tbl, 0x173B6B0, 16);
DataArray(NJS_ACTION*, sakana_abare_tbl, 0x0, 16);
DataArray(NJS_OBJECT*, sakana_object_tbl, 0x173B5F0, 16);
DataArray(NJS_POINT3, sakana_catch_small_tbl, 0x173B7B0, 16);
DataArray(NJS_POINT3, sakana_catch_midium_tbl, 0x173B870, 16);
DataArray(NJS_POINT3, sakana_catch_large_tbl, 0x173B930, 16);
DataPointer(int, sakana_stgAct, 0x3C72A20);
DataPointer(CCL_INFO, seabass_colli_tbl, 0x173BAF0);

static void __cdecl SakanaGenerater_r(task* tp);
static void __cdecl BigSakana_r(task* tp);

Trampoline SakanaGenerater_t(0x597660, 0x597666, SakanaGenerater_r);
Trampoline BigSakana_t(0x597010, 0x597015, BigSakana_r);

static bool FishingEnabled()
{
    return Big_Lure_Ptr != nullptr;
}

static void sub_593F40_m(taskwk* twp, int pnum)
{
    if (twp->scl.y == 0.0f)
    {
        VibShot(pnum, 2);
    }
    else if (twp->scl.y == 1.0f)
    {
        VibShot(pnum, 4);
    }
    else
    {
        VibShot(pnum, 6);
    }
}

static void sub_593F90_m(BIGETC* etc)
{
    ++etc->Big_Fishing_Timer;
    if (etc->Big_Fish_Flag & LUREFLAG_SWING)
    {
        ++etc->Big_Fishing_Timer;
    }
}

static void setActionPointer(task* tp, int motion_type) // inlined in sadxpc
{
    auto twp = tp->twp;
    auto awp = tp->awp;
    auto fish_type = twp->value.w[1];

    NJS_ACTION* action;
    switch (motion_type)
    {
    case 0:
    default:
        action = sakana_hontai_tbl[fish_type];
        break;
    case 1:
        action = sakana_kuituki_tbl[fish_type];
        break;
    case 2:
        action = sakana_kuituki_tbl[fish_type];
        break;
    }

    awp->work.ptr[0] = sakana_object_tbl[twp->value.w[1]];
    awp->work.ptr[1] = action->motion;
}

static bool moveEscapeSakana_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto etc = GetBigEtc(SAKANA_PNUM(twp));

    NJS_POINT3 v = { -0.3f, 0.0f, 0.0f };

    njPushMatrix(_nj_unit_matrix_);
    njRotateY_(twp->ang.y);
    njCalcVector(0, &v, &mwp->spd);
    njPopMatrixEx();

    NJS_POINT3 pos = twp->pos;
    auto ret = MSetPosition(&pos, &mwp->spd, 0, 5.5f);
    CalcHookPos_m(etc, &twp->pos);

    if (GetReelLength_m(etc) <= twp->scl.y * 30.0f + 130.0f)
    {
        return ret != FALSE;
    }

    return true;
}

static void setDirSakanaTurn_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto pnum = SAKANA_PNUM(twp);
    auto etc = GetBigEtc(pnum);

    if (--twp->value.w[0] <= 0)
    {
        twp->mode = MODE_FISHING;
        twp->value.w[0] = (short)((UNIT_RAND + 1.0) * 60.0);
        mwp->ang_aim.y = etc->Big_Lure_Ptr->mwp->ang_aim.y + 0x8000;
        
        if (UNIT_RAND >= 0.5)
        {
            mwp->ang_aim.y -= 0x3000;
        }
        else
        {
            mwp->ang_aim.y += 0x3000;
        }
        
        if (((mwp->weight - 600.0f) * 0.000125f + (etc->Big_Fishing_Timer >= 900 ? 0.0f : 0.2f)) < UNIT_RAND)
        {
            etc->Big_Fish_Flag &= ~LUREFLAG_ESCAPE;
            VibShot(pnum, 5);
        }
        else
        {
            twp->wtimer = 120i16;
            etc->Big_Fish_Flag |= LUREFLAG_ESCAPE;
            dsPlay_oneshot_Dolby(846, 0, 0, 0, 120, twp);
            VibShot(pnum, 4);
        }

        setActionPointer(tp, 2);
    }

    if ((twp->ang.y - mwp->ang_aim.y) >= 0)
    {
        twp->ang.y -= 0x200;
    }
    else
    {
        twp->ang.y += 0x200;
    }

    twp->ang.y &= 0x0000FFFF;

    CalcHookPos_m(etc, &twp->pos);

    mwp->spd.z = 0.0f;
    mwp->spd.y = 0.0f;
    mwp->spd.x = 0.0f;
}

static void setDirSakana2_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto pnum = SAKANA_PNUM(twp);
    auto etc = GetBigEtc(pnum);

    if (--twp->value.w[0] > 0)
    {
        if (etc->Big_Fish_Flag & LUREFLAG_ESCAPE)
        {
            chkAngLimit_m(twp, mwp, &twp->pos);
        }

        if (twp->ang.y - mwp->ang_aim.y >= 0)
        {
            twp->ang.y -= 0x100;
        }
        else
        {
            twp->ang.y += 0x100;
        }
    }
    else
    {
        if (twp->mode == MODE_TURN)
        {
            twp->mode = MODE_FISHING;
        }

        twp->value.w[0] = (short)((UNIT_RAND + 1.0) * 60.0);
        mwp->ang_aim.y = etc->Big_Lure_Ptr->mwp->ang_aim.y + 0x8000;

        if (twp->ang.y - mwp->ang_aim.y >= 0)
        {
            mwp->ang_aim.y -= 0x100;
        }
        else
        {
            mwp->ang_aim.y += 0x100;
        }

        if (UNIT_RAND >= 0.5f)
        {
            mwp->ang_aim.y -= 0x3000;
        }
        else
        {
            mwp->ang_aim.y += 0x3000;
        }

        if (((double)(mwp->weight - 600.0f) * 0.000125 + (etc->Big_Fishing_Timer >= 900 ? 0.0 : 0.2)) < UNIT_RAND)
        {
            VibShot(pnum, 5);
        }
        else if (!twp->wtimer)
        {
            twp->wtimer = 120i16;
            etc->Big_Fish_Flag |= LUREFLAG_ESCAPE;
            setActionPointer(tp, 2);
            dsPlay_oneshot_Dolby(846, 0, 0, 0, 120, twp);
            VibShot(pnum, 4);
        }
    }
}

static void moveFishingSakana_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto etc = GetBigEtc(SAKANA_PNUM(twp));

    if (etc->Big_Lure_Ptr)
    {
        NJS_POINT3 v = { -0.4f, -0.002f, 0.0f };
        
        if ((etc->Big_Fish_Flag & LUREFLAG_ESCAPE) && UNIT_RAND < 0.5f)
        {
            v.x = -0.8f;
        }

        njPushMatrix(_nj_unit_matrix_);
        njRotateY_(twp->ang.y);
        njCalcVector(0, &v, &mwp->spd);
        njPopMatrixEx();

        NJS_POINT3 pos = twp->pos;
        if (MSetPosition(&pos, &mwp->spd, 0, 5.5f) && mwp->spd.y > 0.0f)
        {
            twp->value.w[0] = 64;
            mwp->ang_aim.y = (mwp->ang_aim.y + 0x8000);
            if (twp->mode == MODE_FISHING)
            {
                twp->mode = MODE_TURN;
            }
            etc->Big_Fish_Flag &= ~LUREFLAG_ESCAPE;
            mwp->spd.x = 0.0f;
            mwp->spd.y = 0.0f;
            mwp->spd.z = 0.0f;
        }
        else
        {
            mwp->spd.x *= 0.5f;
            mwp->spd.y *= 0.5f;
            mwp->spd.z *= 0.5f;
        }

        CalcHookPos_m(etc, &twp->pos);

        if (twp->wtimer)
        {
            --twp->wtimer;
        }
        else
        {
            etc->Big_Fish_Flag &= ~LUREFLAG_ESCAPE;
            setActionPointer(tp, 2);
        }
    }
}

static bool moveLureDirSakana_m(task* tp, task* otp) // todo: try again to rewrite this one
{
    auto pnum = SAKANA_PNUM(tp->twp);
    auto etc = GetBigEtc(pnum);
    auto backup_ptr = Big_Fish_Ptr;
    auto backup_ptwp = playertwp[0];
    Big_Fish_Ptr = etc->Big_Fish_Ptr;
    playertwp[0] = playertwp[pnum];
    auto ret = moveLureDirSakana(tp, otp);
    Big_Fish_Ptr = backup_ptr;
    playertwp[0] = backup_ptwp;
    return ret;
}

static bool chkRetStart_m(task* tp)
{
    auto mwp = tp->mwp;
    auto twp = tp->twp;
    auto awp = tp->awp;

    if (!awp)
    {
        return false;
    }

    auto orig_tp = (task*)awp->work.ul[2];
    NJS_POINT3 v{};

    if (orig_tp)
    {
        v.x = orig_tp->twp->pos.x - twp->pos.x;
        v.y = orig_tp->twp->pos.y - twp->pos.y;
        v.z = orig_tp->twp->pos.z - twp->pos.z;
    }
    else
    {
        orig_tp = tp;
    }

    mwp->ang_aim.y = -0x4000 - NJM_RAD_ANG(-atan2(twp->pos.x - orig_tp->twp->pos.x, twp->pos.z - orig_tp->twp->pos.z));

    if ((twp->ang.y - mwp->ang_aim.y) <= 0x8000u)
    {
        twp->ang.y -= 0x80;
    }
    else
    {
        twp->ang.y += 0x80;
    }
    
    twp->ang.y &= 0x0000FFFF;

    mwp->spd.z = 0.0f;
    mwp->spd.y = 0.0f;
    mwp->spd.x = 0.0f;

    moveLureDirSakana_m(tp, orig_tp);

    if (njScalor(&v) >= 3.0f)
    {
        return false;
    }

    twp->pos = orig_tp->twp->pos;
    mwp->ang_aim.y = orig_tp->twp->ang.y;

    return true;
}

static void setSakanaRetStart_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto etc = GetBigEtc(SAKANA_PNUM(twp));

    twp->mode = MODE_RETPOS;

    if (etc->Big_Fish_Ptr == tp)
    {
        etc->Big_Fish_Flag &= ~(LUREFLAG_ESCAPE | LUREFLAG_RANGEOUT | LUREFLAG_HIT);
        etc->Big_Fish_Ptr = nullptr;
    }

    if (mwp->height > 1.0f)
    {
        mwp->height -= 1.0f;
    }

    twp->btimer = 0;
    setActionPointer(tp, 0);
}

static void moveCatchingSakana_m(task* tp)
{
    auto twp = tp->twp;
    auto pnum = SAKANA_PNUM(twp);
    auto ptwp = playertwp[pnum];

    if (ptwp)
    {
        njPushMatrix(_nj_unit_matrix_);
        njRotateZ_(ptwp->ang.z);
        njRotateX_(ptwp->ang.x);
        njRotateY_(0x8000 - ptwp->ang.y);

        auto ppwp = playerpwp[pnum];
        NJS_VECTOR v;
        v.x = ppwp->righthand_pos.x;
        v.y = ppwp->righthand_pos.y + ppwp->p.center_height;
        v.z = ppwp->righthand_pos.z;

        auto fish_type = twp->value.w[1];
        if (twp->scl.y == 1.0f)
        {
            njAddVector(&v, &sakana_catch_midium_tbl[fish_type]);
        }
        else if (twp->scl.y == 2.0f)
        {
            njAddVector(&v, &sakana_catch_large_tbl[fish_type]);
        }
        else
        {
            njAddVector(&v, &sakana_catch_small_tbl[fish_type]);
        }

        njCalcVector(0, &v, &v);
        njPopMatrixEx();

        twp->pos.x = v.x + ptwp->pos.x;
        twp->pos.y = v.y + ptwp->pos.y;
        twp->pos.z = v.z + ptwp->pos.z;

        if (twp->scl.y)
        {
            twp->ang.z = 0;
            twp->ang.y = 0x4000 - ptwp->ang.y;
            twp->ang.z = 0;
            twp->ang.x = 0;
        }
        else
        {
            twp->ang.z = 0x4000;
            twp->ang.x = 0x4000 - ptwp->ang.y;
        }
    }
}

static bool SakanaRangeOut(task* tp) // custom, probably inlined
{
    auto twp = tp->twp;

    // if currently being fished, don't delete
    if (twp->mode >= MODE_FISHING_ATARI)
    {
        auto etc = GetBigEtc(SAKANA_PNUM(twp));

        if (etc->Big_Lure_Ptr && etc->Big_Fish_Flag & LUREFLAG_HIT)
        {
            return false;
        }
    }

    // if player in range, don't delete
    if (IsPlayerInSphere(&twp->pos, 400.0f))
    {
        return false;
    }
    
    FreeTask(tp);
    return true;
}

static void BigSakana_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto pnum = SAKANA_PNUM(twp);
    auto etc = GetBigEtc(pnum);

    if (SakanaRangeOut(tp))
    {
        return;
    }

    switch (twp->mode)
    {
    case MODE_INIT:
        CCL_Init(tp, &seabass_colli_tbl, 1, 4u);
        twp->mode = MODE_FREE;
        break;
    case MODE_FREE:
        if (FishingEnabled() && chkFishPtr_m(tp) && chkLureKaeru_m(twp, mwp, etc))
        {
            twp->mode = MODE_TRACE;
            mwp->work.f = GetWaterLevel_m(etc);
        }

        setDirKaeru(tp);
        moveKaeru(tp);
        break;
    case MODE_RETPOS:
        if (chkRetStart_m(tp))
        {
            twp->mode = MODE_RETANG;
        }
        break;
    case MODE_RETANG:
    {
        auto angdiff = SubAngle(mwp->ang_aim.y, twp->ang.y);

        if (angdiff <= 0x8000)
        {
            twp->ang.y -= 0x80;
        }
        else
        {
            twp->ang.y += 0x80;
        }

        twp->ang.y &= 0x0000FFFF;

        mwp->spd.x = 0.0f;
        mwp->spd.y = 0.0f;
        mwp->spd.z = 0.0f;

        if (angdiff < 0x50 || angdiff > 0xFF80)
        {
            twp->ang.y = mwp->ang_aim.y;

            twp->mode = MODE_FREE;
            if (etc->Big_Fish_Ptr == tp)
            {
                etc->Big_Fish_Flag &= ~(LUREFLAG_ESCAPE | LUREFLAG_RANGEOUT | LUREFLAG_HIT);
                etc->Big_Fish_Ptr = nullptr;
            }
            
            if (mwp->height > 1.0f)
            {
                mwp->height -= 1.0f;
            }

            setActionPointer(tp, 0);
        }

        break;
    }
    case MODE_TRACE:
        if (!chkFishPtr_m(tp))
        {
            setSakanaRetStart_m(tp);
            break;
        }

        if (chkDistanceLure_m(twp, mwp, etc))
        {
            etc->Big_Fish_Flag |= LUREFLAG_HIT;
            twp->mode = MODE_FISHING_ATARI;
            mwp->ang_aim.y = etc->Big_Lure_Ptr->mwp->ang_aim.y + 0x8000;
            
            sub_593F40_m(twp, pnum);
            setActionPointer(tp, 1);

            twp->wtimer = 45;
            twp->btimer = 1;
            mwp->spd.x = 0.0f;
            mwp->spd.y = 0.0f;
            mwp->spd.z = 0.0f;
        }
        else
        {
            setDirKaeru3_m(twp, mwp, etc);
            if (!moveLureDirSakana_m(tp, etc->Big_Lure_Ptr))
            {
                setSakanaRetStart_m(tp);
                break;
            }
        }
        break;
    case MODE_FISHING_ATARI:
        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            twp->mode = MODE_CATCHING;
            twp->ang = { 0, 0, 0x4000 };
            setActionPointer(tp, 0);
            break;
        }

        if ((etc->Big_Fish_Flag & (LUREFLAG_MISS | LUREFLAG_RANGEOUT)) || !twp->wtimer)
        {
            setSakanaRetStart_m(tp);
            break;
        }

        // Only catch if the player is swinging or if it's a small fish
        if ((etc->Big_Fish_Flag & LUREFLAG_SWING) || twp->scl.y == 0.0f)
        {
            etc->Big_Fishing_Timer = 0;

            if (GetReelLength_m(etc) > 100.0f || GetStageNumber() == LevelAndActIDs_IceCap1)
            {
                twp->mode = MODE_FISHING;
            }
            else
            {
                twp->mode = MODE_ESCAPE;
                etc->Big_Fish_Flag |= LUREFLAG_ESCAPE;
            }

            dsPlay_oneshot(1414, 0, 0, 0);
            sub_593F40_m(twp, pnum);

            PlayJingle(45);
            etc->reel_tension_add = 0.0f;
            SetBigDispHit_m(pnum);

            setActionPointer(tp, 2);
        }

        moveFishingSakana_m(tp);
        break;
    case MODE_FISHING:
        sub_593F90_m(etc);

        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            twp->mode = MODE_CATCHING;
            twp->ang = { 0, 0, 0x4000 };
            setActionPointer(tp, 0);
            break;
        }

        if ((etc->Big_Fish_Flag & (LUREFLAG_MISS | LUREFLAG_RANGEOUT)))
        {
            setSakanaRetStart_m(tp);
            break;
        }

        setDirSakana2_m(tp);
        moveFishingSakana_m(tp);

        break;
    case MODE_TURN:
        sub_593F90_m(etc);

        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            twp->mode = MODE_CATCHING;
            twp->ang = { 0, 0, 0x4000 };
            setActionPointer(tp, 0);
            break;
        }

        if ((etc->Big_Fish_Flag & (LUREFLAG_MISS | LUREFLAG_RANGEOUT)))
        {
            setSakanaRetStart_m(tp);
            break;
        }

        setDirSakanaTurn_m(tp);
        break;
    case MODE_ESCAPE:
        sub_593F90_m(etc);

        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            twp->mode = MODE_CATCHING;
            twp->ang = { 0, 0, 0x4000 };
            setActionPointer(tp, 0);
            break;
        }

        if ((etc->Big_Fish_Flag & (LUREFLAG_MISS | LUREFLAG_RANGEOUT)))
        {
            setSakanaRetStart_m(tp);
            break;
        }

        sub_596570(tp);
        if (moveEscapeSakana_m(tp))
        {
            twp->mode = MODE_FISHING;
        }

        break;
    case MODE_CATCHING:
        moveCatchingSakana_m(tp);

        if (ChkFishingThrowNow_m(pnum))
        {
            FreeTask(tp);
        }
        break;
    }

    motionSakana(tp);
    tp->disp(tp);
}

static void __cdecl BigSakana_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        BigSakana_m(tp);
    }
    else
    {
        TARGET_STATIC(BigSakana)(tp);
    }
}

static void SakanaGenerater_m(task* tp)
{
    auto twp = tp->twp;
    switch (twp->mode)
    {
    case 0i8:
        twp->mode = 1i8;
        twp->value.l = 0;
        sakana_stgAct = CurrentAct | (CurrentLevel << 8);
        break;
    case 1i8:
        if (sakana_stgAct == (CurrentAct | (CurrentLevel << 8)) && Sakana_Num < MAX_FISH)
        {
            if (IsPlayerInSphere(&twp->pos, 400.0f) && UNIT_RAND <= 0.3
                && (GetStageNumber() != LevelAndActIDs_HotShelter1 || twp->scl.y == 0.0f || Big_Stg12_Flag))
            {
                twp->value.ptr = setSakana(tp);
                if (twp->value.ptr)
                {
                    twp->mode = 2i8;
                    ++Sakana_Num;
                }
            }
        }
        break;
    case 2i8:
    {
        auto fish_tp = (task*)twp->value.ptr;

        if (fish_tp == nullptr || fish_tp->twp == nullptr)
        {
            twp->mode = 1i8;
            break;
        }

        if (fish_tp->twp->mode >= MODE_TRACE)
        {
            auto pnum = SAKANA_PNUM(fish_tp->twp);
            auto etc = GetBigEtc(pnum);

            if (fish_tp == etc->Big_Fish_Ptr && (etc->Big_Fish_Flag & (LUREFLAG_HIT | LUREFLAG_RELEASE)))
            {
                twp->mode = 3i8;
                twp->timer.l = 18000;
            }
        }

        break;
    }
    case 3i8:
        if (--twp->timer.l <= 0)
        {
            twp->mode = 1i8;
        }
        break;
    }
}

static void __cdecl SakanaGenerater_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        SakanaGenerater_m(tp);
    }
    else
    {
        TARGET_STATIC(SakanaGenerater)(tp);
    }
}
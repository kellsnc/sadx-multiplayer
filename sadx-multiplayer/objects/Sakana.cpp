#include "pch.h"
#include "multiplayer.h"
#include "fishing.h"

#define SAKANA_PNUM(twp) twp->smode

enum : __int8
{
    MODE_INIT,     // symbol
    MODE_FREE,     // symbol
    MODE_2,
    MODE_3,
    MODE_FISHING,  // symbol
    MODE_CATCHING, // symbol
    MODE_6,
    MODE_7,
    MODE_8,
    MODE_CAUGHT
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
TaskFunc(setDirKaeru, 0x7A6CE0);
TaskFunc(moveKaeru, 0x7A7100);

static void __cdecl SakanaGenerater_r(task* tp);
static void __cdecl BigSakana_r(task* tp);

Trampoline SakanaGenerater_t(0x597660, 0x597666, SakanaGenerater_r);
Trampoline BigSakana_t(0x597010, 0x597015, BigSakana_r);

static bool FishingEnabled()
{
    return Big_Lure_Ptr != nullptr;
}

static bool ChkFishing(BIGETC* etc)
{
    return etc->Big_Fish_Flag & (LUREFLAG_HIT | LUREFLAG_FISH);
}

static bool ChkLureFree(BIGETC* etc)
{
    return !(etc->Big_Fish_Flag & (LUREFLAG_RANGEOUT | LUREFLAG_RELEASE | LUREFLAG_HIT));
}

static float GetReelLength(BIGETC* etc)
{
    return etc->reel_length;
}

static bool chkFishPtr_m(task* tp)
{
    auto twp = tp->twp;

    if (twp->mode == MODE_FREE)
    {
        for (int i = 0; i < PLAYER_MAX; ++i)
        {
            auto etc = GetBigEtc(i);

            if (!etc->Big_Lure_Ptr)
            {
                continue;
            }

            if (!ChkFishing(etc))
            {
                etc->Big_Fish_Ptr = nullptr;
                continue;
            }

            auto dist = GetDistance(&etc->Big_Lure_Ptr->twp->pos, &twp->pos);

            if (etc->Big_Fish_Ptr)
            {
                if (etc->Big_Fish_Ptr == tp)
                {
                    if (!ChkLureFree(etc) && dist < 100.0f)
                    {
                        SAKANA_PNUM(twp) = i;
                        return true;
                    }
                    else
                    {
                        etc->Big_Fish_Ptr = nullptr;
                        continue;
                    }
                }
                else if (ChkLureFree(etc))
                {
                    auto dist2 = GetDistance(&etc->Big_Lure_Ptr->twp->pos, &etc->Big_Fish_Ptr->twp->pos);

                    if (dist2 > dist)
                    {
                        etc->Big_Fish_Ptr = tp;
                        SAKANA_PNUM(twp) = i;
                        return true;
                    }
                }
            }
            else if (dist < 100.0f)
            {
                SAKANA_PNUM(twp) = i;
                etc->Big_Fish_Ptr = tp;
                return 1;
            }
        }
    }
    else
    {
        auto etc = GetBigEtc(SAKANA_PNUM(twp));
        auto dist = GetDistance(&etc->Big_Lure_Ptr->twp->pos, &twp->pos);

        if (!etc->Big_Lure_Ptr)
        {
            return false;
        }

        if (!ChkFishing(etc))
        {
            etc->Big_Fish_Ptr = nullptr;
            return false;
        }

        if (etc->Big_Fish_Ptr == tp)
        {
            if (dist < 100.0f)
            {
                return true;
            }
            else
            {
                etc->Big_Fish_Ptr = nullptr;
                return false;
            }
        }
    }

    return false;
}

static bool chkLureKaeru_m(task* tp)
{
    auto twp = tp->twp;
    auto etc = GetBigEtc(SAKANA_PNUM(twp));

    if (!etc->Big_Lure_Ptr)
    {
        return false;
    }

    if (!(etc->Big_Fish_Flag & LUREFLAG_FISH))
    {
        return false;
    }

    NJS_POINT3 v1;
    v1.x = etc->Big_Lure_Ptr->twp->pos.x - twp->pos.x;
    v1.y = etc->Big_Lure_Ptr->twp->pos.y - twp->pos.y;
    v1.z = etc->Big_Lure_Ptr->twp->pos.z - twp->pos.z;

    NJS_POINT3 v2 = { -1.0f, 0.0f, 0.0f };

    njPushMatrix(_nj_unit_matrix_);
    njRotateY_(twp->ang.y);
    njCalcVector(0, &v2, &v2);
    njPopMatrixEx();

    auto inp = njInnerProduct(&v1, &v2);
    auto scl = njScalor(&v1);

    if (scl != 0.0f)
    {
        inp = inp / scl;
    }

    float test = twp->scl.y == 0.0f ? 100.0f : (twp->scl.y == 1.0f ? 60.0f : 30.0f);

    if (scl >= test)
    {
        return false;
    }

    if (inp > 1.0f || inp <= 0.0f)
    {
        if (etc->Big_Fish_Flag & LUREFLAG_1000)
        {
            tp->mwp->height += 1.0f;
        }
    }
    else
    {
        if (etc->Big_Fish_Flag & LUREFLAG_1000)
        {
            tp->mwp->height += 2.0f;
        }
    }

    return tp->mwp->height >= 3.0f;
}

static bool chkDistanceLure_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto etc = GetBigEtc(SAKANA_PNUM(twp));

    if (!etc->Big_Lure_Ptr)
    {
        return false;
    }

    if (GetDistance(&etc->Big_Lure_Ptr->twp->pos, &twp->pos) >= 1.0f)
    {
        return false;
    }
    
    mwp->work.f = etc->water_level;
    return true;
}

static void setDirKaeru3_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto etc = GetBigEtc(SAKANA_PNUM(twp));

    if (etc->Big_Lure_Ptr)
    {
        auto ang = -0x4000 - NJM_RAD_ANG(atan2f(twp->pos.x - etc->Big_Lure_Ptr->twp->pos.x, twp->pos.z - etc->Big_Lure_Ptr->twp->pos.z));

        mwp->ang_aim.y = ang;

        if (SubAngle(ang, twp->ang.y) <= 0x8000u)
        {
            twp->ang.y -= 0x80;
        }
        else
        {
            twp->ang.y += 0x80;
        }

        HIWORD(twp->ang.y) = 0;
    }
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

static void CalcHookPos_m(BIGETC* etc, NJS_POINT3* ret)
{
    if (etc->Big_Lure_Ptr)
    {
        auto twp = etc->Big_Lure_Ptr->twp;
        NJS_POINT3 v = { 0.6f, -0.45f, 0.0f };
        
        njPushMatrix(_nj_unit_matrix_);
        njRotateZ_(twp->ang.z);
        njRotateX_(twp->ang.x);
        njRotateY_(-HIWORD(twp->ang.y));
        njCalcVector(0, &v, ret);
        njPopMatrixEx();

        ret->x += twp->pos.x;
        ret->y += twp->pos.y;
        ret->z += twp->pos.z;
    }
    else
    {
        ret->z = 0.0f;
        ret->y = 0.0f;
        ret->x = 0.0f;
    }
}

static bool chkAngLimit_m(task* tp, NJS_POINT3* next_pos)
{
    auto mwp = tp->mwp;
    auto pnum = SAKANA_PNUM(tp->twp);
    auto ptwp = playertwp[pnum];

    if (ptwp)
    {
        __int16 ang = 0x4000 - NJM_RAD_ANG(atan2f(next_pos->x - ptwp->pos.x, next_pos->z - ptwp->pos.x));
        auto diffang = SubAngle(ptwp->ang.y, ang);

        if (diffang <= 0x1555u || diffang >= 0x8000u)
        {
            if (diffang >= 0xEAAAu || diffang <= 0x8000u)
            {
                if (GetDistance(next_pos, &ptwp->pos))
                {
                    return true;
                }
            }
            else
            {
                mwp->ang_aim.y = ptwp->ang.y + 0x8000;
                return false;
            }
        }

        mwp->ang_aim.y = ptwp->ang.y + 0x8000;
        return false;
    }

    return false;
}

#pragma region BigSakana
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

    auto ret = MSetPosition(&twp->pos, &mwp->spd, 0, 5.5f);
    CalcHookPos_m(etc, &twp->pos);

    if (GetReelLength(etc) <= twp->scl.y * 30.0f + 130.0f)
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
        twp->mode = MODE_6;
        twp->value.w[0] = (short)(((double)rand() * 0.000030517578 + 1.0) * 60.0);
        mwp->ang_aim.y = etc->Big_Lure_Ptr->mwp->ang_aim.y + 0x8000;
        
        if ((float)((double)rand() * 0.000030517578) >= 0.5f)
        {
            mwp->ang_aim.y -= 0x3000;
        }
        else
        {
            mwp->ang_aim.y += 0x3000;
        }
        
        if ((float)((double)rand() * 0.000030517578) >= ((mwp->weight - 600.0f) * 0.000125f + (etc->Big_Fishing_Timer >= 900 ? 0.0f : 0.2f)))
        {
            etc->Big_Fish_Flag &= ~LUREFLAG_8;
            VibShot(pnum, 5);
        }
        else
        {
            twp->wtimer = 120i16;
            etc->Big_Fish_Flag |= LUREFLAG_8;
            dsPlay_oneshot_Dolby(846, 0, 0, 0, 120, twp);
            VibShot(pnum, 4);
        }

        setActionPointer(tp, 2);
    }

    if (SubAngle(mwp->ang_aim.y, twp->ang.y) >= 0)
    {
        twp->ang.y -= 0x200;
    }
    else
    {
        twp->ang.y += 0x200;
    }

    HIWORD(twp->ang.y) = 0;

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
        if (etc->Big_Fish_Flag & LUREFLAG_8)
        {
            chkAngLimit_m(tp, &twp->pos);
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
        if (twp->mode == 7)
        {
            twp->mode = 6;
        }

        twp->value.w[0] = (short)(((double)rand() * 0.000030517578 + 1.0) * 60.0);
        mwp->ang_aim.y = etc->Big_Lure_Ptr->mwp->ang_aim.y + 0x8000;

        if (twp->ang.y - mwp->ang_aim.y >= 0)
        {
            mwp->ang_aim.y -= 0x100;
        }
        else
        {
            mwp->ang_aim.y += 0x100;
        }

        if ((float)((double)rand() * 0.000030517578) >= 0.5f)
        {
            mwp->ang_aim.y -= 0x3000;
        }
        else
        {
            mwp->ang_aim.y += 0x3000;
        }

        if ((float)((double)rand() * 0.000030517578) >= ((mwp->weight - 600.0f) * 0.000125f + (etc->Big_Fishing_Timer >= 900 ? 0.0f : 0.2f)))
        {
            VibShot(pnum, 5);
        }
        else if (!twp->wtimer)
        {
            twp->wtimer = 120i16;
            etc->Big_Fish_Flag |= LUREFLAG_8;
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
        
        if ((etc->Big_Fish_Flag & LUREFLAG_8) && (float)((double)rand() * 0.000030517578) < 0.5f)
        {
            v.x = -0.8f;
        }

        njPushMatrix(_nj_unit_matrix_);
        njRotateY_(twp->ang.y);
        njCalcVector(0, &v, &mwp->spd);
        njPopMatrixEx();

        if (MSetPosition(&twp->pos, &mwp->spd, 0, 5.5f) && mwp->spd.y > 0.0f)
        {
            twp->value.w[0] = 64;
            mwp->ang_aim.y = (mwp->ang_aim.y + 0x8000);
            if (twp->mode == MODE_6)
            {
                twp->mode = MODE_7;
            }
            etc->Big_Fish_Flag &= ~LUREFLAG_8;
            mwp->spd.x = 0.0f;
            mwp->spd.y = 0.0f;
            mwp->spd.z = 0.0f;
        }

        mwp->spd.x *= 0.5f;
        mwp->spd.y *= 0.5f;
        mwp->spd.z *= 0.5f;

        CalcHookPos_m(etc, &twp->pos);

        if (twp->wtimer)
        {
            --twp->wtimer;
        }
        else
        {
            etc->Big_Fish_Flag &= ~LUREFLAG_8;
            setActionPointer(tp, 2);
        }
    }
}

static bool moveLureDirSakana_m(task* tp)
{
    auto etc = GetBigEtc(SAKANA_PNUM(tp->twp));
    auto backup_ptr = Big_Fish_Ptr;
    Big_Fish_Ptr = etc->Big_Fish_Ptr;
    auto ret = moveLureDirSakana(tp, etc->Big_Lure_Ptr); //f u
    Big_Fish_Ptr = backup_ptr;
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

    auto some_tp = (task*)awp->work.ul[2];
    NJS_POINT3 v{};

    if (some_tp)
    {
        v.x = some_tp->twp->pos.x - twp->pos.x;
        v.y = some_tp->twp->pos.y - twp->pos.y;
        v.z = some_tp->twp->pos.z - twp->pos.z;
    }
    else
    {
        some_tp = tp;
    }

    mwp->ang_aim.y = -0x4000 - NJM_RAD_ANG(atan2f(twp->pos.x - some_tp->twp->pos.x, twp->pos.z - some_tp->twp->pos.z));

    if (SubAngle(mwp->ang_aim.y, twp->ang.y) <= 0x8000)
    {
        twp->ang.y -= 0x80;
    }
    else
    {
        twp->ang.y += 0x80;
    }
    
    HIWORD(twp->ang.y) = 0;

    mwp->spd.z = 0.0f;
    mwp->spd.y = 0.0f;
    mwp->spd.x = 0.0f;

    moveLureDirSakana_m(tp);

    if (njScalor(&v) >= 3.0f)
    {
        return false;
    }

    twp->pos = some_tp->twp->pos;
    mwp->ang_aim.y = some_tp->twp->ang.y;

    return true;
}

static void setSakanaRetStart_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto etc = GetBigEtc(SAKANA_PNUM(twp));

    twp->mode = MODE_2;

    if (etc->Big_Fish_Ptr == tp)
    {
        etc->Big_Fish_Flag &= ~(LUREFLAG_8 | LUREFLAG_RANGEOUT | LUREFLAG_HIT);
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
    if (twp->mode >= MODE_CATCHING)
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
        if (FishingEnabled() && chkFishPtr_m(tp) && chkLureKaeru_m(tp))
        {
            twp->mode = MODE_FISHING;
            mwp->work.f = etc->water_level;
        }

        setDirKaeru(tp);
        moveKaeru(tp);
        break;
    case MODE_2:
        if (chkRetStart_m(tp))
        {
            twp->mode = MODE_3;
        }
        break;
    case MODE_3:
    {
        auto angdiff = SubAngle(mwp->ang_aim.y, twp->ang.y);

        if (angdiff <= 0x8000)
        {
            twp->ang.y -= 128;
        }
        else
        {
            twp->ang.y += 128;
        }

        HIWORD(twp->ang.y) = 0;

        mwp->spd.x = 0.0f;
        mwp->spd.y = 0.0f;
        mwp->spd.z = 0.0f;

        if (angdiff < 0x50 || angdiff > 0xFF80)
        {
            twp->ang.y = mwp->ang_aim.y;

            twp->mode = 1;
            if (etc->Big_Fish_Ptr == tp)
            {
                etc->Big_Fish_Flag &= ~(LUREFLAG_8 | LUREFLAG_RANGEOUT | LUREFLAG_HIT);
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
    case MODE_FISHING:
        if (!chkFishPtr_m(tp))
        {
            setSakanaRetStart_m(tp);
            break;
        }

        if (chkDistanceLure_m(tp))
        {
            etc->Big_Fish_Flag |= LUREFLAG_HIT;
            twp->mode = MODE_CATCHING;
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
            setDirKaeru3_m(tp);
            if (!moveLureDirSakana_m(tp))
            {
                setSakanaRetStart_m(tp);
                break;
            }
        }
        break;
    case MODE_CATCHING:
        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            twp->mode = MODE_CAUGHT;
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

            if (GetReelLength(etc) > 100.0f || GetStageNumber() == LevelAndActIDs_IceCap1)
            {
                twp->mode = MODE_6;
            }
            else
            {
                twp->mode = MODE_8;
                etc->Big_Fish_Flag |= LUREFLAG_8;
            }

            dsPlay_oneshot(1414, 0, 0, 0);
            sub_593F40_m(twp, pnum);
            //SetBigDispHit
            setActionPointer(tp, 2);
        }

        moveFishingSakana_m(tp);
        break;
    case MODE_6:
        sub_593F90_m(etc);

        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            twp->mode = MODE_CAUGHT;
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
    case MODE_7:
        sub_593F90_m(etc);

        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            twp->mode = MODE_CAUGHT;
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
    case MODE_8:
        sub_593F90_m(etc);

        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            twp->mode = MODE_CAUGHT;
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
            twp->mode = MODE_6;
        }

        break;
    case MODE_CAUGHT:
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
#pragma endregion

#pragma region SakanaGenerater
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
        if (sakana_stgAct == (CurrentAct | (CurrentLevel << 8)) && Sakana_Num < 5)
        {
            if (IsPlayerInSphere(&twp->pos, 400.0f) && (float)((double)rand() * 0.000030517578) <= 0.3f
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

        if (fish_tp == nullptr)
        {
            twp->mode = 1i8;
            break;
        }

        if (fish_tp->twp->mode >= MODE_FISHING)
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
#pragma endregion
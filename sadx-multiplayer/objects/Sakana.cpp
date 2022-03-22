#include "pch.h"
#include "multiplayer.h"
#include "fishing.h"

#define SAKANA_PNUM(twp) twp->smode

enum : __int8
{
    MODE_INIT,
    MODE_FREE,
    MODE_2,
    MODE_FISHING = 4,
    MODE_CATCHING
};

static auto setSakana = GenerateUsercallWrapper<task* (*)(task* tp)>(rEAX, 0x597590, rEDI);
static auto motionSakana = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x596A40, rEDX);
static auto moveLureDirSakana = GenerateUsercallWrapper<BOOL (*)(task* tp, task* otp)>(rEAX, 0x596D40, rEAX, stack4);

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
        auto ang = 0x4000 - NJM_RAD_ANG(atan2f(twp->pos.x - etc->Big_Lure_Ptr->twp->pos.x, twp->pos.z - etc->Big_Lure_Ptr->twp->pos.z));

        mwp->ang_aim.y = ang;

        if ((twp->ang.y - ang) <= 0x8000u)
        {
            twp->ang.y -= 0x80;
        }
        else
        {
            twp->ang.y += 0x80;
        }
    }
}

#pragma region BigSakana
static bool moveLureDirSakana_m(task* tp)
{
    auto etc = GetBigEtc(SAKANA_PNUM(tp->twp));
    auto backup_ptr = Big_Fish_Ptr;
    Big_Fish_Ptr = etc->Big_Fish_Ptr;
    auto ret = moveLureDirSakana(tp, etc->Big_Lure_Ptr); //f u
    Big_Fish_Ptr = backup_ptr;
    return ret;
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
}

static void BigSakana_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto pnum = SAKANA_PNUM(twp);
    auto etc = GetBigEtc(pnum);

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
    case MODE_FISHING:
        if (!chkFishPtr_m(tp))
        {
            setSakanaRetStart_m(tp);
        }

        if (chkDistanceLure_m(tp))
        {
            etc->Big_Fish_Flag |= LUREFLAG_HIT;
            twp->mode = MODE_CATCHING;
            mwp->ang_aim.y = etc->Big_Lure_Ptr->mwp->ang_aim.y + 0x8000;
            
            if (twp->scl.y == 0.0f)
            {
                RumbleA(pnum, 2);
            }
            else if (twp->scl.y == 1.0f)
            {
                RumbleA(pnum, 4);
            }
            else
            {
                RumbleA(pnum, 6);
            }

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
            }
        }
        break;
    case MODE_CATCHING:

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
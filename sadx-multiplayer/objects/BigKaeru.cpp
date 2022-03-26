#include "pch.h"
#include "multiplayer.h"
#include "fishing.h"
#include "hud_fishing.h"
#include "BigKaeru.h"

#define KAERU_PNUM(twp) twp->smode
#define CHKFISHING(etc) (etc->Big_Fish_Flag & (LUREFLAG_HIT | LUREFLAG_FISH))
#define CHKLUREFREE(etc) (!(etc->Big_Fish_Flag & (LUREFLAG_RANGEOUT | LUREFLAG_RELEASE | LUREFLAG_HIT)))

enum : __int8
{
    MODE_INIT,
    MODE_FREE,
    MODE_TRACE,
    MODE_FISHING_ATARI,
    MODE_FISHING,
    MODE_CATCHING
};

FunctionPointer(BOOL, moveLureDirKaeru, (task* tp), 0x7A7290);
TaskFunc(moveKaeru_Only, 0x7A6F60);
DataPointer(float, gf32BigDepth, 0x3C55018);
DataPointer(NJS_ACTION, action_f_f0021_frog, 0x9228A4);
DataPointer(NJS_ACTION, action_f_f0022_frog, 0x9228A4); // unused
DataPointer(NJS_ACTION, action_f_f0023_frog, 0x927594);

static auto motionBigKaeru = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x7A6C90, rEAX);

static void setActionPointer(taskwk* twp, int motion_type) // inlined in both
{
    NJS_ACTION* action;
    switch (motion_type)
    {
    case 0:
    default:
        action = &action_f_f0021_frog;
        break;
    case 1:
        action = &action_f_f0022_frog;
        break;
    case 2:
        action = &action_f_f0023_frog;
        break;
    }

    if (twp->counter.ptr != action)
    {
        twp->counter.ptr = action;
        twp->timer.f = 0.0f;
    }
}

static void setDirKaeru2_m(taskwk* twp, motionwk* mwp, BIGETC* etc)
{
    if (--twp->value.w[0] <= 0)
    {
        twp->value.w[0] = 120;
        mwp->ang_aim.y ^= 0x8000u;

        if (UNIT_RAND < 0.15)
        {
            dsPlay_oneshot_v(853, 0, 0, 0, twp->pos.x, twp->pos.y, twp->pos.z);
            twp->wtimer = 120i16;
            etc->Big_Fish_Flag |= LUREFLAG_ESCAPE;
            setActionPointer(twp, 2);
            VibShot(KAERU_PNUM(twp), 4);
        }
    }
}

bool chkAngLimit_m(taskwk* twp, motionwk* mwp, NJS_POINT3* next_pos)
{
    auto pnum = KAERU_PNUM(twp);
    auto ptwp = playertwp[pnum];

    if (ptwp)
    {
        __int16 ang = 0x4000 - NJM_RAD_ANG(atan2(next_pos->x - ptwp->pos.x, next_pos->z - ptwp->pos.x));
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

void setDirKaeru3_m(taskwk* twp, motionwk* mwp, BIGETC* etc)
{
    if (etc->Big_Lure_Ptr)
    {
        mwp->ang_aim.y = -0x4000 - NJM_RAD_ANG(-atan2(twp->pos.x - etc->Big_Lure_Ptr->twp->pos.x, twp->pos.z - etc->Big_Lure_Ptr->twp->pos.z));

        if ((twp->ang.y - mwp->ang_aim.y) <= 0x8000u)
        {
            twp->ang.y -= 0x80;
        }
        else
        {
            twp->ang.y += 0x80;
        }

        twp->ang.y &= 0x0000FFFF;
    }
}

static bool moveLureDirKaeru_m(task* tp)// todo: try again to rewrite this one
{
    auto pnum = KAERU_PNUM(tp->twp);
    auto etc = GetBigEtc(pnum);
    auto backup_ptr = Big_Lure_Ptr;
    auto backup_flag = Big_Fish_Flag;
    auto backup_ptwp = playertwp[0];
    Big_Lure_Ptr = etc->Big_Lure_Ptr;
    Big_Fish_Flag = etc->Big_Fish_Flag;
    playertwp[0] = playertwp[pnum];
    auto ret = moveLureDirKaeru(tp);
    Big_Lure_Ptr = backup_ptr;
    Big_Fish_Flag = backup_flag;
    playertwp[0] = backup_ptwp;
    return ret != FALSE;
}

static void moveFishingKaeru_m(taskwk* twp, motionwk* mwp, BIGETC* etc)
{
    if (etc->Big_Lure_Ptr)
    {
        auto spd = njScalor(&mwp->spd);
        NJS_POINT3 v = {};

        if (twp->timer.f <= 25.0f || twp->timer.f >= 45.0f)
        {
            if (-spd >= -0.05f)
            {
                v.x = -0.1f;
            }
            else
            {
                v.x = 0.004f - spd;
            }
        }
        else
        {
            v.x = -0.2f;
        }

        v.x *= 2;

        njPushMatrix(_nj_unit_matrix_);
        njRotateY_(twp->ang.y);
        njCalcVector(0, &v, &mwp->spd);
        njPopMatrixEx();

        if (MSetPosition(&twp->pos, &mwp->spd, 0, gf32BigDepth))
        {
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
            setActionPointer(twp, 0);
        }
    }
}

static void setCatchingKaeru_m(taskwk* twp, BIGETC* etc)
{
    twp->mode = MODE_CATCHING;
    twp->ang = { 0, 0, 0x4000 };
    etc->Big_Fish_Flag |= LUREFLAG_KAERU;
    setActionPointer(twp, 2);
}

static void moveCatchingKaeru_m(taskwk* twp, int pnum)
{
    auto ppwp = playerpwp[pnum];
    auto ptwp = playertwp[pnum];
    njPushMatrix(_nj_unit_matrix_);
    njRotateZ_(ptwp->ang.z);
    njRotateX_(ptwp->ang.x);
    njRotateY_(0x8000 - LOWORD(ptwp->ang.y));
    NJS_POINT3 v;
    v.x = ppwp->righthand_pos.x;
    v.y = ppwp->righthand_pos.y + ppwp->p.center_height;
    v.z = ppwp->righthand_pos.z + 2.0f;
    njCalcVector(0, &v, &v);
    njPopMatrixEx();
    twp->pos.x = v.x + ptwp->pos.x;
    twp->pos.y = v.y + ptwp->pos.y;
    twp->pos.z = v.z + ptwp->pos.z;
    twp->ang.y = 0x4000 - ptwp->ang.y;
    twp->ang.z = 0;
    twp->ang.x = 0;
}

bool chkLureKaeru_m(taskwk* twp, motionwk* mwp, BIGETC* etc)
{
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
        if (etc->Big_Fish_Flag & LUREFLAG_REEL)
        {
            mwp->height += 1.0f;
        }
    }
    else
    {
        if (etc->Big_Fish_Flag & LUREFLAG_REEL)
        {
            mwp->height += 2.0f;
        }
    }

    return mwp->height >= 3.0f;
}

bool chkDistanceLure_m(taskwk* twp, motionwk* mwp, BIGETC* etc)
{
    if (!etc->Big_Lure_Ptr)
    {
        return false;
    }

    if (GetDistance(&etc->Big_Lure_Ptr->twp->pos, &twp->pos) >= 1.0f)
    {
        return false;
    }

    mwp->work.f = GetWaterLevel_m(etc);
    return true;
}

bool chkFishPtr_m(task* tp)
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

            if (!CHKFISHING(etc))
            {
                etc->Big_Fish_Ptr = nullptr;
                continue;
            }

            auto dist = GetDistance(&etc->Big_Lure_Ptr->twp->pos, &twp->pos);

            if (etc->Big_Fish_Ptr)
            {
                if (etc->Big_Fish_Ptr == tp)
                {
                    if (!CHKLUREFREE(etc) && dist < 100.0f)
                    {
                        KAERU_PNUM(twp) = i;
                        return true;
                    }
                    else
                    {
                        etc->Big_Fish_Ptr = nullptr;
                        continue;
                    }
                }
                else if (CHKLUREFREE(etc))
                {
                    auto dist2 = GetDistance(&etc->Big_Lure_Ptr->twp->pos, &etc->Big_Fish_Ptr->twp->pos);

                    if (dist2 > dist)
                    {
                        etc->Big_Fish_Ptr = tp;
                        KAERU_PNUM(twp) = i;
                        return true;
                    }
                }
            }
            else if (dist < 100.0f)
            {
                KAERU_PNUM(twp) = i;
                etc->Big_Fish_Ptr = tp;
                return 1;
            }
        }
    }
    else
    {
        auto etc = GetBigEtc(KAERU_PNUM(twp));
        auto dist = GetDistance(&etc->Big_Lure_Ptr->twp->pos, &twp->pos);

        if (!etc->Big_Lure_Ptr)
        {
            return false;
        }

        if (!CHKFISHING(etc))
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

static void setKaeruFree_m(taskwk* twp, motionwk* mwp, BIGETC* etc)
{
    twp->mode = MODE_FREE;
    if (mwp->height > 1.0f)
        mwp->height -= 1.0f;
    etc->Big_Fish_Ptr = nullptr;
    setActionPointer(twp, 0);
}

static bool KaeruRangeOut(task* tp) // custom, probably inlined
{
    auto twp = tp->twp;

    // if currently being fished, don't delete
    if (twp->mode >= MODE_TRACE)
    {
        auto etc = GetBigEtc(KAERU_PNUM(twp));

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

static void BigKaeru_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;
    auto pnum = KAERU_PNUM(twp);
    auto etc = GetBigEtc(pnum);
    
    if (KaeruRangeOut(tp))
    {
        return;
    }

    switch (twp->mode)
    {
    case MODE_INIT:
        // We don't want Froggy in Battle mode
        if (multiplayer::IsBattleMode())
        {
            FreeTask(tp);
            return;
        }

        twp->mode = MODE_FREE;
        tp->disp = dispBigKaeru;
        setActionPointer(twp, 0);
        mwp->weight = 500.0f;
        twp->scl.y = 2.0f;
        twp->value.w[1] = 0;
        break;
    case MODE_FREE:
        if (chkFishPtr_m(tp) && chkLureKaeru_m(twp, mwp, etc))
        {
            twp->mode = MODE_TRACE;
            mwp->work.f = GetWaterLevel_m(etc);
        }
        else
        {
            setDirKaeru(tp);
            moveKaeru_Only(tp);
        }
        break;
    case MODE_TRACE:
        if (chkDistanceLure_m(twp, mwp, etc))
        {
            etc->Big_Fish_Flag |= LUREFLAG_HIT;
            twp->mode = MODE_FISHING_ATARI;
            mwp->ang_aim.y = etc->Big_Lure_Ptr->mwp->ang_aim.y + 0x8000;

            VibShot(pnum, 2);
            setActionPointer(twp, 2);

            twp->wtimer = 45;
        }
        else
        {
            setDirKaeru3_m(twp, mwp, etc);
            if (!moveLureDirKaeru_m(tp))
            {
                etc->Big_Fish_Flag &= ~LUREFLAG_ESCAPE;
                setKaeruFree_m(twp, mwp, etc);
                break;
            }
        }
        break;
    case MODE_FISHING_ATARI:
        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            setCatchingKaeru_m(twp, etc);
            break;
        }

        if ((etc->Big_Fish_Flag & (LUREFLAG_MISS | LUREFLAG_RANGEOUT)) || !twp->wtimer)
        {
            etc->Big_Fish_Flag &= ~(LUREFLAG_ESCAPE | LUREFLAG_HIT);
            setKaeruFree_m(twp, mwp, etc);
            break;
        }

        // Only catch if the player is swinging or if it's a small fish
        if ((etc->Big_Fish_Flag & LUREFLAG_SWING) || twp->scl.y == 0.0f)
        {
            etc->Big_Fishing_Timer = 0;

            twp->mode = MODE_FISHING;
            dsPlay_oneshot(1414, 0, 0, 0);
            VibShot(pnum, 2);
            PlayJingle(45);
            etc->reel_tension_add = 0.0f;
            SetBigDispHit_m(pnum);
            break;
        }

        if (!chkFishPtr_m(tp))
        {
            etc->Big_Fish_Flag &= ~(LUREFLAG_ESCAPE | LUREFLAG_HIT);
            setKaeruFree_m(twp, mwp, etc);
            break;
        }

        setDirKaeru2_m(twp, mwp, etc);
        moveFishingKaeru_m(twp, mwp, etc);

        break;
    case MODE_FISHING:
        if (etc->Big_Fish_Flag & LUREFLAG_RELEASE)
        {
            setCatchingKaeru_m(twp, etc);
            break;
        }

        if ((etc->Big_Fish_Flag & (LUREFLAG_MISS | LUREFLAG_RANGEOUT)))
        {
            etc->Big_Fish_Flag &= ~(LUREFLAG_ESCAPE | LUREFLAG_HIT);
            setKaeruFree_m(twp, mwp, etc);
            break;
        }

        setDirKaeru2_m(twp, mwp, etc);
        moveFishingKaeru_m(twp, mwp, etc);

        break;
    case MODE_CATCHING:
        moveCatchingKaeru_m(twp, pnum);

        if (ChkFishingThrowNow_m(pnum))
        {
            FreeTask(tp);
        }
        break;
    }

    motionBigKaeru(tp);
    EntryColliList(twp);
    tp->disp(tp);
}

static void __cdecl BigKaeru_r(task* tp);
Trampoline BigKaeru_t(0x7A7AD0, 0x7A7AD5, BigKaeru_r);
static void __cdecl BigKaeru_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        BigKaeru_m(tp);
    }
    else
    {
        TARGET_STATIC(BigKaeru)(tp);
    }
}
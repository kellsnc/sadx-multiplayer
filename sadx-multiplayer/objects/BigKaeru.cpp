#include "pch.h"
#include "multiplayer.h"
#include "fishing.h"
#include "hud_fishing.h"
#include "BigKaeru.h"

#define KAERU_PNUM(twp) twp->smode

enum : __int8
{
    MODE_INIT,
    MODE_FREE,
    MODE_TRACE,
    MODE_FISHING_ATARI,
    MODE_FISHING,
    MODE_CATCHING
};

static bool ChkFishing(BIGETC* etc)
{
    return etc->Big_Fish_Flag & (LUREFLAG_HIT | LUREFLAG_FISH);
}

static bool ChkLureFree(BIGETC* etc)
{
    return !(etc->Big_Fish_Flag & (LUREFLAG_RANGEOUT | LUREFLAG_RELEASE | LUREFLAG_HIT));
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
                        KAERU_PNUM(twp) = i;
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

void setDirKaeru3_m(taskwk* twp, motionwk* mwp, BIGETC* etc)
{
    if (etc->Big_Lure_Ptr)
    {
        mwp->ang_aim.y = -0x4000 - NJM_RAD_ANG(atan2f(twp->pos.x - etc->Big_Lure_Ptr->twp->pos.x, twp->pos.z - etc->Big_Lure_Ptr->twp->pos.z));

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


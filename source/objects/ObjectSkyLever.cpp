#include "pch.h"
#include "multiplayer.h"

TaskFunc(exitSkyLever, 0x5F1BC0);
TaskFunc(dispSkyLever, 0x5F1B50);
DataPointer(Angle, tAngX, 0x3C8068C);
DataPointer(Angle, tAngZ, 0x3C80690);
DataPointer(NJS_OBJECT, object_sky_s_levar_s_cnt1, 0x21AE9A0);
DataArray(CCL_INFO, cci_lever, 0x2239B00, 2);

enum : char
{
    MODE_0,
    MODE_1,
    MODE_2
};

static int GetPlayerHolding(task* htp)
{
    for (int i = 0; i < PLAYER_MAX; ++i)
    {
        auto ppwp = playerpwp[i];

        if (ppwp && ppwp->htp == htp)
        {
            return i;
        }
    }

    return -1;
}

static void mode_2(task* tp, taskwk* twp, motionwk* mwp)
{
    auto pnum = twp->btimer;

    dsPlay_timer(978, 4161, 1, 0, 30);
    twp->ang.x = AdjustAngle(twp->ang.x, tAngX, 182);
    twp->ang.z = AdjustAngle(twp->ang.z, tAngZ, 182);

    if (twp->ang.x == tAngX && twp->ang.z == tAngZ || !(AttackButtons & perG[pnum].on))
    {
        twp->mode = MODE_1;
    }

    if (EV_CheckCansel())
    {
        SetGravityMaybe(0, 0);
    }
    else
    {
        SetGravityMaybe(twp->ang.x, twp->ang.z);
    }
}

static void mode_1(task* tp, taskwk* twp, motionwk* mwp)
{
    auto pnum = GetPlayerHolding(tp);

    if (pnum >= 0)
    {
        auto ptwp = playertwp[pnum];

        if (!ptwp)
        {
            return;
        }

        twp->btimer = pnum;

        auto ang = NJM_RAD_ANG(atan2f(ptwp->pos.x - twp->pos.x, ptwp->pos.z - twp->pos.z));

        if (ang < 0x2000)
        {
            if (mwp->acc.z <= 0.0f)
            {
                if (mwp->acc.z >= 0.0f)
                {
                    tAngX = twp->ang.x;
                    tAngZ = 0;
                    twp->mode = MODE_2;
                    return;
                }
                else
                {
                    tAngX = -3640;
                    tAngZ = 0;
                    twp->mode = MODE_2;
                    return;
                }
            }
            else
            {
                tAngX = 3640;
                tAngZ = 0;
                twp->mode = MODE_2;
                return;
            }
        }
        if (ang >= 0x6000)
        {
            if (ang < 0xA000)
            {
                if (mwp->acc.z <= 0.0f)
                {
                    if (mwp->acc.z >= 0.0f)
                    {
                        tAngX = twp->ang.x;
                        tAngZ = 0;
                        twp->mode = MODE_2;
                        return;
                    }
                    else
                    {
                        tAngX = -3640;
                        tAngZ = 0;
                        twp->mode = MODE_2;
                        return;
                    }
                }
                else
                {
                    tAngX = 3640;
                    tAngZ = 0;
                    twp->mode = MODE_2;
                    return;
                }
            }
            if (ang >= 0xE000)
            {
                if (mwp->acc.z <= 0.0f)
                {
                    if (mwp->acc.z >= 0.0f)
                    {
                        tAngX = twp->ang.x;
                        tAngZ = 0;
                        twp->mode = MODE_2;
                        return;
                    }
                    else
                    {
                        tAngX = -3640;
                        tAngZ = 0;
                        twp->mode = MODE_2;
                        return;
                    }
                }
                else
                {
                    tAngX = 3640;
                    tAngZ = 0;
                    twp->mode = MODE_2;
                    return;
                }
            }
            if (mwp->acc.x <= 0.0f)
            {
                if (mwp->acc.x >= 0.0f)
                {
                    tAngZ = twp->ang.z;
                }
                else
                {
                    tAngZ = 3640;
                }
                tAngX = 0;
                twp->mode = MODE_2;
            }
            else
            {
                tAngZ = -3640;
                tAngX = 0;
                twp->mode = MODE_2;
            }
        }
        else if (mwp->acc.x <= 0.0f)
        {
            if (mwp->acc.x >= 0.0f)
            {
                tAngZ = twp->ang.z;
            }
            else
            {
                tAngZ = 3640;
            }
            tAngX = 0;
            twp->mode = MODE_2;
        }
        else
        {
            tAngZ = -3640;
            tAngX = 0;
            twp->mode = MODE_2;
        }
    }
}

static void execSkyLever_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = tp->mwp;

    switch (twp->mode)
    {
    case MODE_0:
        twp->mode = MODE_1;
        twp->pos.y = tp->ptp->twp->pos.y + 6.0f;
        twp->counter.ptr = &object_sky_s_levar_s_cnt1;
        CCL_Init(tp, cci_lever, 2, 4u);
        ObjectMovableInitialize(twp, mwp, 5u); //5: push lever
        tp->dest = exitSkyLever;
        tp->disp = dispSkyLever;
        twp->ang.x = tAngX;
        twp->ang.z = tAngZ;
        tAngX = 0;
        tAngZ = 0;
        break;
    case MODE_1:
        mode_1(tp, twp, mwp);
        break;
    case MODE_2:
        mode_2(tp, twp, mwp);
        break;
    }

    EntryColliList(twp);
    ObjectSetupInput(twp, mwp);
    tp->disp(tp);
}

static void __cdecl execSkyLever_r(task* tp);
Trampoline execSkyLever_t(0x5F1D20, 0x5F1D28, execSkyLever_r);
static void __cdecl execSkyLever_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        execSkyLever_m(tp);
    }
    else
    {
        TARGET_STATIC(execSkyLever)(tp);
    }
}
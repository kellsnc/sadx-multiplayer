#include "pch.h"
#include <UsercallFunctionHandler.h>
#include "multiplayer.h"

static auto chkDamage = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x4E8090, rEAX);
static auto smoke = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x4E8260, rEAX);
static auto calcSpd = GenerateUsercallWrapper<void (*)(Angle3* ang, MISSILE_WK* wk, float force)>(noret, 0x568280, rEDI, rESI, stack4); // custom name

static void updownHeight_m(taskwk* twp, MISSILE_WK* wk, taskwk* ptwp)
{
    float y = playerpwp[TASKWK_PLAYERID(ptwp)]->p.center_height + ptwp->pos.y - twp->pos.y;
    
    if (fabs(y) >= 0.1f)
    {
        if (y <= 0.0f)
        {
            wk->spd.y -= 0.02f;
        }
        else
        {
            wk->spd.y += 0.03f;
        }
    }
    else
    {
        wk->spd.y *= 0.7f;
    }

    wk->spd.y = min(2.0f, max(-1.5f, wk->spd.y));
}

static void exec_m(task* tp)
{
    auto twp = tp->twp;
    auto wk = (MISSILE_WK*)tp->awp;

    --wk->nLifeTime;

    chkDamage(tp);

    auto ptwp = playertwp[GetTheNearestPlayerNumber(&twp->pos)];
    twp->ang.y += min(910, max(-910, SubAngle(twp->ang.y, NJM_RAD_ANG(-atan2f(twp->pos.z - ptwp->pos.z, twp->pos.x - ptwp->pos.x)))));

    updownHeight_m(twp, wk, ptwp);
    calcSpd(&twp->ang, wk, 0.4f);

    auto len = njScalor(&wk->spd);
    if (len > 1.0f)
    {
        len = 1.0f / len;
        wk->spd.x = len * wk->spd.x;
        wk->spd.y = len * wk->spd.y;
        wk->spd.z = len * wk->spd.z;
    }

    twp->ang.x = NJM_RAD_ANG(atan2f(sqrtf(wk->spd.x * wk->spd.x + wk->spd.z * wk->spd.z), wk->spd.y));
    twp->pos.x += wk->spd.x;
    twp->pos.y += wk->spd.y;
    twp->pos.z += wk->spd.z;

    if (wk->nLifeTime >= 5)
    {
        if (wk->nLifeTime < 24)
        {
            wk->flagBrink = (wk->nLifeTime >> 1) & 1;
        }
    }
    else
    {
        wk->flagBrink = 1;
    }

    smoke(tp);
    EntryColliList(twp);
    dsPlay_Dolby_time(154, (int)tp, 1, 0, 4, twp);
    tp->disp(tp);
}

static void __cdecl EnemyWindyTgtMissile_exec_r(task* tp);
Trampoline EnemyWindyTgtMissile_exec_t(0x4E84B0, 0x4E84B6, EnemyWindyTgtMissile_exec_r);
static void __cdecl EnemyWindyTgtMissile_exec_r(task* tp)
{
    if (multiplayer::IsEnabled())
    {
        exec_m(tp);
    }
    else
    {
        TARGET_STATIC(EnemyWindyTgtMissile_exec)(tp);
    }
}
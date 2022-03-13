#include "pch.h"
#include <UsercallFunctionHandler.h>

enum : char
{
    INIT,   // symbol
    WAIT,   // symbol
    ACTIVE, // custom
    STOP,   // custom
};

DataPointer(CCL_INFO, palley_colli, 0x24C8C70);
TaskFunc(PathKassha, 0x603640);
TaskFunc(KasshaDisplayer, 0x603590);

auto SetPath2Taskwk_0 = GenerateUsercallWrapper<void (*)(pathtag* a1, taskwk* a2, float a3)>(noret, 0x602A50, rEDX, rESI, noret);
auto DrawHuck = GenerateUsercallWrapper<void (*)(task* a1)>(noret, 0x602B10, rEAX);
auto DrawWireTarumi = GenerateUsercallWrapper<void (*)(task* a1, int a2, char a3)>(noret, 0x603330, rEAX, rECX, noret);
auto DrawWire = GenerateUsercallWrapper<void (*)(task* a1)>(noret, 0x602DF0, rEAX);
 
void PathKassha_r(task* tp);
Trampoline PathKassha_t(0x603640, 0x603647, PathKassha_r);
void PathKassha_r(task* tp)
{
    if (!multiplayer::IsActive())
    {
        return TARGET_STATIC(PathKassha)(tp);
    }

    auto twp = tp->twp;
    auto path = (pathtag*)twp->value.l;
    auto pnum = twp->smode;

    NJS_VECTOR pos;
    int pnt;

    if (twp->timer.w[0] != GetStageNumber())
    {
        FreeTask(tp);
        return;
    }

    switch (twp->mode)
    {
    case INIT:
        tp->disp = KasshaDisplayer;

        CCL_Init(tp, &palley_colli, 1, 4u);
        SetPath2Taskwk_0(path, twp, twp->counter.f);

        twp->wtimer = 0i16;
        twp->mode = WAIT;
        twp->counter.f = 0.0f;
        twp->scl.z = path->totallen;
        break;
    case WAIT:
        if ((twp->cwp->flag & 1) != 0)
        {
            auto ptwp = CCL_IsHitPlayer(twp);

            if (!ptwp)
                ptwp = playertwp[GetTheNearestPlayerNumber(&twp->pos)];

            pnum = twp->smode = TASKWK_PLAYERID(ptwp);

            SetInputP(pnum, 16);
            twp->mode = ACTIVE;
        }

        EntryColliList(twp);

        pos = { twp->pos.x - playertwp[pnum]->pos.x, twp->pos.y - playertwp[pnum]->pos.y, twp->pos.z - playertwp[pnum]->pos.z };
        if (njScalor(&pos) < 100.0f)
        {
            path = (pathtag*)twp->value.l;
            SetPath2Taskwk_0(path, twp, 0.0f);
            DrawHuck(tp);
            SCPathOnposToPntnmb(path, twp->counter.f, &pnt);
            DrawWireTarumi(tp, pnt, 0);
            DrawWireTarumi(tp, pnt + 1, 0);
            return;
        }

        break;
    case ACTIVE:
        SCPathOnposToPntnmb((pathtag*)twp->value.l, twp->counter.f, &pnt);
        DrawWireTarumi(tp, pnt + 1, 0);
        DrawWireTarumi(tp, pnt + 2, 0);
        DrawWire(tp);

        twp->counter.f += 8.0f;
        if (twp->counter.f > twp->scl.z)
        {
            twp->mode = STOP;
            SetInputP(pnum, 24);
            SetVelocityP(pnum, njCos(-playertwp[pnum]->ang.y) * 2.2f, 1.0f, njSin(-playertwp[pnum]->ang.y) * -2.2f);
            SCPathOnposToPntnmb((pathtag*)twp->value.l, twp->scl.z, &pnt);
            twp->btimer = pnt;
        }

        dsPlay_timer(132, (int)tp, 1, 0, 10);
        return;
    case 3:
        pos = { twp->pos.x - playertwp[pnum]->pos.x, twp->pos.y - playertwp[pnum]->pos.y, twp->pos.z - playertwp[pnum]->pos.z };
        
        if (njScalor(&pos) < 100.0f)
        {
            DrawWireTarumi(tp, twp->btimer, 1);
        }
        break;
    default:
        return;
    }
}
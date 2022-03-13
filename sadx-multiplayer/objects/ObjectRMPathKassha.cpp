#include "pch.h"
#include <UsercallFunctionHandler.h>
#include "multiplayer.h"
#include "splitscreen.h"

enum : char
{
    INIT,   // symbol
    WAIT,   // symbol
    ACTIVE, // custom
    STOP,   // custom
};

DataPointer(CCL_INFO, palley_colli, 0x24C8C70);

auto SetPath2Taskwk_0 = GenerateUsercallWrapper<void (*)(pathtag* a1, taskwk* a2, float a3)>(noret, 0x602A50, rEDX, rESI, noret);
auto DrawHuck = GenerateUsercallWrapper<void (*)(task* a1)>(noret, 0x602B10, rEAX);
auto DrawWireTarumi = GenerateUsercallWrapper<void (*)(task* a1, int a2, char a3)>(noret, 0x603330, rEAX, rECX, stack1);
auto DrawWire = GenerateUsercallWrapper<void (*)(task* a1)>(noret, 0x602DF0, rEAX);

// The wire displayer also moves the player for some reason
// Luckily they put a "mode == 2" check before even though the function is only called during mode 2
static void DrawWire_m(task* tp)
{
    tp->twp->mode = WAIT;
    DrawWire(tp);
    tp->twp->mode = ACTIVE;
}

static void DrawWireTarumi_m(task* tp)
{
    auto twp = tp->twp;
    auto path = (pathtag*)twp->value.l;
    
    for (int i = 0; i < path->points; ++i)
    {
        auto pt = &path->tblhead[i];

        if (twp->mode == ACTIVE && twp->btimer == i)
        {
            continue;
        }

        if (dsCheckViewV((NJS_POINT3*)&pt->xpos, pt->length))
        {
            DrawWireTarumi(tp, i, i == path->points - 1 ? 1 : 0);
        }
    }
}

static void KasshaDisplayer_r(task* tp);
Trampoline KasshaDisplayer_t(0x603590, 0x603595, KasshaDisplayer_r);
static void KasshaDisplayer_r(task* tp)
{
    if (!SplitScreen::IsActive())
    {
        return TARGET_STATIC(KasshaDisplayer)(tp);
    }

    if (!MissedFrames)
    {
        auto twp = tp->twp;

        if (twp->mode != ACTIVE)
        {
            DrawHuck(tp);
        }

        DrawWireTarumi_m(tp);

        if (twp->mode == ACTIVE)
        {
            DrawWire_m(tp);
        }
    }
}

static void MovePlayer(taskwk* twp, pathtag* tag, float onpathpos, int pnum)
{
    pathinfo info; info.onpathpos = max(0.0f, onpathpos - 8.0f);
    if (GetStatusOnPath(tag, &info))
    {
        SetPositionP(pnum, info.xpos, info.ypos - twp->scl.y - 15.5f, info.zpos);
        SetRotationP(pnum, 0, twp->ang.y, 0);
    }
}

static void PathKassha_r(task* tp);
Trampoline PathKassha_t(0x603640, 0x603647, PathKassha_r);
static void PathKassha_r(task* tp)
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
        if (twp->cwp->flag & 1)
        {
            twp->cwp->flag &= ~1;

            auto ptwp = CCL_IsHitPlayer(twp);

            if (ptwp)
            {
                pnum = twp->smode = TASKWK_PLAYERID(ptwp);
                SetInputP(pnum, 16);
                twp->mode = ACTIVE;
            }
        }

        pos = { twp->pos.x - playertwp[pnum]->pos.x, twp->pos.y - playertwp[pnum]->pos.y, twp->pos.z - playertwp[pnum]->pos.z };
        if (njScalor(&pos) < 100.0f)
        {
            SetPath2Taskwk_0(path, twp, 0.0f);
        }

        break;
    case ACTIVE:
        MovePlayer(twp, path, twp->counter.f, twp->smode);
        
        twp->counter.f += 8.0f;
        if (twp->counter.f > twp->scl.z)
        {
            twp->wtimer = 0;
            twp->mode = STOP;
            SetInputP(pnum, 24);
            SetVelocityP(pnum, njCos(-playertwp[pnum]->ang.y) * 2.2f, 1.0f, njSin(-playertwp[pnum]->ang.y) * -2.2f);
            break;
        }

        SCPathOnposToPntnmb(path, twp->counter.f, &pnt);
        twp->btimer = pnt;

        dsPlay_timer(132, (int)tp, 1, 0, 10);
        break;
    case STOP:
        if (++twp->wtimer > 30)
        {
            twp->mode = WAIT;
            twp->btimer = 0;
            twp->counter.f = 0.0f;
            SetPath2Taskwk_0(path, twp, twp->counter.f);
        }
        break;
    default:
        break;
    }

    EntryColliList(twp);
    tp->disp(tp);
}
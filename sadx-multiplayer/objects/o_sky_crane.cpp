#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

static void ObjectSkydeck_crane_hang_m(task* tp)
{
    auto twp = tp->twp;
    auto parent_twp = tp->ptp->twp;
    
    float dist = twp->smode ? 210.0f : -210.0f;

    twp->scl.z = GetShadowPos(twp->pos.x, twp->pos.y, twp->pos.z, &twp->ang);

    Angle parent_ang = -(parent_twp->ang.y + 136);
    twp->pos.x = njCos(parent_ang) * dist + parent_twp->pos.x;
    twp->pos.y = parent_twp->pos.y - 72.0f - 140.0f;
    twp->pos.z = njSin(parent_ang) * dist + parent_twp->pos.z;

    auto ptwp = twp->mode <= 1 ? CCL_IsHitPlayer(twp) : playertwp[twp->btimer];
    auto& pnum = twp->btimer;

    switch (twp->mode)
    {
    case 0i8:
        twp->mode = 1i8;
        tp->disp = ObjectSkydeck_crane_hang_Draw;
        CCL_Init(tp, (CCL_INFO*)0x2239C68, 1, 4u);
        break;
    case 1i8:
        if (ptwp == nullptr)
        {
            break;
        }

        pnum = TASKWK_PLAYERID(ptwp);
        SetInputP(pnum, PL_OP_PLACEWITHHUNG);
        twp->mode = 2i8;
        dsPlay_oneshot(180, 0, 0, 0);

        switch (TASKWK_CHARID(twp))
        {
        case Characters_Sonic:
            if (!MetalSonicFlag) dsPlay_oneshot(1229, 0, 0, 0);
            break;
        case Characters_Tails:
            dsPlay_oneshot(1246, 0, 0, 0);
            break;
        case Characters_Knuckles:
            dsPlay_oneshot(1270, 0, 0, 0);
            break;
        }
       
        break;
    case 2i8:
        ptwp->pos.x = twp->pos.x;
        ptwp->pos.y = twp->pos.y - 9.5f;
        ptwp->pos.z = twp->pos.z;

        SetRotationP(pnum, 0, parent_twp->ang.y + (twp->smode << 15), 0);

        if (ptwp->smode != 16)
        {
            twp->mode = 3;
            twp->wtimer = 120;
        }

        if (Controllers[pnum].PressedButtons & JumpButtons)
        {
            SetInputP(pnum, PL_OP_LETITGO);
            twp->mode = 3i8;
            twp->wtimer = 120;
        }

        break;
    case 3i8:
        if (!--twp->wtimer)
        {
            twp->mode = 1i8;
        }
        break;
    }

    EntryColliList(twp);
    tp->disp(tp);
}

static void __cdecl ObjectSkydeck_crane_hang_r(task* tp);
Trampoline ObjectSkydeck_crane_hang_t(0x5F2F90, 0x5F2F99, ObjectSkydeck_crane_hang_r);
static void __cdecl ObjectSkydeck_crane_hang_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        ObjectSkydeck_crane_hang_m(tp);
    }
    else
    {
        TARGET_STATIC(ObjectSkydeck_crane_hang)(tp);
    }
}
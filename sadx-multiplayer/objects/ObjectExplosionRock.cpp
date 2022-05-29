#include "pch.h"
#include "multiplayer.h"
#include <UsercallFunctionHandler.h>

static void __cdecl ObjectMountainExplosionRock1_r(task* tp);
static void __cdecl ObjectMountainExplosionRock2_r(task* tp);
static void __cdecl MiddleRock_r(task* tp);

Trampoline ObjectMountainExplosionRock1_t(0x602130, 0x602135, ObjectMountainExplosionRock1_r);
Trampoline ObjectMountainExplosionRock2_t(0x602320, 0x602325, ObjectMountainExplosionRock2_r);
Trampoline MiddleRock_t(0x601EC0, 0x601EC7, MiddleRock_r);

struct steamtbl
{
    NJS_POINT3 pos;
    unsigned __int16 init_time;
    unsigned __int16 wtimer;
};

DataArray(steamtbl, exrock1_steamtbl, 0x24985B0, 1);
DataArray(steamtbl, exrock2_steamtbl, 0x24985F0, 5);

auto GenerateSteam = GenerateUsercallWrapper<void (*)(steamtbl* tbl, int num)>(noret, 0x601A10, rECX, rEAX);
auto CreateExplosion = GenerateUsercallWrapper<void (*)(steamtbl* tbl, int num)>(noret, 0x601AD0, rECX, rEAX);

#pragma region MiddleRock
static void MiddleRock_m(task* tp)
{
    auto twp = tp->twp;
    auto mwp = (motionwk*)tp->mwp;

    if (IsPlayerInSphere(&mwp->acc, mwp->rad + 10.0f))
    {
        twp->flag |= 0x100u;
    }
    else
    {
        twp->flag &= ~0x100u;
    }

    for (int i = 0; i < PLAYER_MAX; ++i)
    {
        if (CheckPlayerRideOnMobileLandObjectP(i, tp))
        {
            tp->ptp->twp->btimer = i;
            tp->ptp->twp->flag |= 0x100;
        }
    }
    
    tp->disp(tp);
}

static void __cdecl MiddleRock_r(task* tp)
{
    if (!multiplayer::IsActive() || tp->twp->mode == 0)
    {
        TARGET_STATIC(MiddleRock)(tp);
    }
    else
    {
        MiddleRock_m(tp);
    }
}
#pragma endregion

#pragma region ObjectMountainExplosionRock2
static void BlowPlayer3_m(int pnum, float x, float y, float z)
{
    auto ptwp = playertwp[pnum];
    auto ppwp = playerpwp[pnum];
    auto weight = ppwp->p.weight;

    auto dist = (ptwp->pos.z - z) * (ptwp->pos.z - z) + (ptwp->pos.x - x) * (ptwp->pos.x - x);
    dist = sqrtf(dist) / 5.0f;

    NJS_POINT3 pos;
    pos.x = (x - ptwp->pos.x) * (1.0f / dist);
    pos.y = (dist * dist * weight * 0.5f + y - ptwp->pos.y) * (1.0f / dist);
    pos.z = (z - ptwp->pos.z) * (1.0f / dist);

    dist = njScalor(&pos);
    njUnitVector(&pos);
    SetParabolicMotionP(pnum, dist, &pos);
}

static void ObjectMountainExplosionRock2_m(task* tp)
{
    auto twp = tp->twp;

    switch (twp->mode)
    {
    case 1i8:
        GenerateSteam(&exrock2_steamtbl, 4);

        if (twp->flag & 0x100)
        {
            twp->flag &= ~0x100;

            if (++twp->wtimer >= 20)
            {
                dsPlay_oneshot_v(128, 0, 0, 0, twp->pos.x, twp->pos.y, twp->pos.z);
                CreateExplosion(&exrock2_steamtbl, 4);
                twp->mode = 2i8;
                twp->flag |= 0x200;
            }
        }
        break;
    case 2i8:
        BlowPlayer3_m(twp->btimer, -800.0f, 1120.0f, -125.0f);
        twp->mode = 3i8;
        break;
    case 3i8:
        twp->mode = 1i8;
        break;
    }

    LoopTaskC(tp);
}

static void __cdecl ObjectMountainExplosionRock2_r(task* tp)
{
    if (!multiplayer::IsActive() || tp->twp->mode == 0)
    {
        TARGET_STATIC(ObjectMountainExplosionRock2)(tp);
    }
    else
    {
        ObjectMountainExplosionRock2_m(tp);
    }
}
#pragma endregion

#pragma region ObjectMountainExplosionRock1
static void BlowPlayer2_m(int pnum, float x, float y, float z)
{
    auto ptwp = playertwp[pnum];
    auto ppwp = playerpwp[pnum];

    auto weight = ppwp->p.weight;
    auto maxspd = ppwp->p.lim_v_spd;

    auto dist = (ptwp->pos.z - z) * (ptwp->pos.z - z) + (ptwp->pos.x - x) * (ptwp->pos.x - x);
    dist = sqrtf(dist) / 5.0f;

    NJS_POINT3 pos;
    pos.x = (x - ptwp->pos.x) * (1.0f / dist);
    pos.z = (z - ptwp->pos.z) * (1.0f / dist);

    if (ptwp->pos.y <= y)
    {
        pos.y = (dist * dist * weight * 0.5f + y - ptwp->pos.y) * (1.0f / dist);
    }
    else
    {
        pos.y = ((dist - maxspd / weight) * maxspd + ptwp->pos.y - ptwp->pos.y + maxspd / weight * (maxspd / weight) * weight * 0.5f) / (maxspd / weight);
    }

    dist = njScalor(&pos);
    njUnitVector(&pos);
    SetParabolicMotionP(pnum, dist, &pos);
}

static void ObjectMountainExplosionRock1_m(task* tp)
{
    auto twp = tp->twp;

    switch (twp->mode)
    {
    case 1i8:
        GenerateSteam(&exrock1_steamtbl, 4);

        if (twp->flag & 0x100)
        {
            twp->flag &= ~0x100;

            if (++twp->wtimer >= 20)
            {
                dsPlay_oneshot_v(128, 0, 0, 0, twp->pos.x, twp->pos.y, twp->pos.z);
                CreateExplosion(&exrock1_steamtbl, 4);
                twp->mode = 2i8;
                twp->flag |= 0x200;
            }
        }
        break;
    case 2i8:
        BlowPlayer2_m(twp->btimer, -931.0f, 856.0f, -933.0f);
        twp->mode = 3i8;
        break;
    case 3i8:
        twp->mode = 1i8;
        break;
    }

    LoopTaskC(tp);
}

static void __cdecl ObjectMountainExplosionRock1_r(task* tp)
{
    if (!multiplayer::IsActive() || tp->twp->mode == 0)
    {
        TARGET_STATIC(ObjectMountainExplosionRock1)(tp);
    }
    else
    {
        ObjectMountainExplosionRock1_m(tp);
    }
}
#pragma endregion
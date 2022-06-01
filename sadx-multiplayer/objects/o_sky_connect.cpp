#include "pch.h"
#include "multiplayer.h"

bool CheckAnyPlayerRide(task* tp)
{
    for (int i = 0; i < PLAYER_MAX; ++i)
    {
        if (CheckPlayerRideOnMobileLandObjectP(i, tp))
        {
            return true;
        }
    }
    return false;
}

static void __cdecl ObjectSkydeck_c_connect_Exec_r(task* tp);
Trampoline ObjectSkydeck_c_connect_Exec_t(0x5FAE90, 0x5FAE98, ObjectSkydeck_c_connect_Exec_r);
static void __cdecl ObjectSkydeck_c_connect_Exec_r(task* tp)
{
    TARGET_STATIC(ObjectSkydeck_c_connect_Exec)(tp);

    if (multiplayer::IsActive())
    {
        auto twp = tp->twp;
        auto ocm = (OCMDATA*)twp->timer.ptr;

        if (CheckAnyPlayerRide(tp))
            ocm->flag |= 1;
        else
            ocm->flag &= ~1;
    }
}
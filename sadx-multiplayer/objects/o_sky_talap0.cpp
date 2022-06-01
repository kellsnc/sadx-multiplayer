#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl ObjectSkydeck_c_talap0_Exec_r(task* tp);
Trampoline ObjectSkydeck_c_talap0_Exec_t(0x5FB5F0, 0x5FB5F8, ObjectSkydeck_c_talap0_Exec_r);
static void __cdecl ObjectSkydeck_c_talap0_Exec_r(task* tp)
{
    TARGET_STATIC(ObjectSkydeck_c_talap0_Exec)(tp);

    if (multiplayer::IsActive())
    {
        auto twp = tp->twp;
        auto ocm = (OCMDATA*)twp->timer.ptr;

        if (IsPlayerOnDyncol(tp))
            ocm->flag |= 1;
        else
            ocm->flag &= ~1;
    }
}
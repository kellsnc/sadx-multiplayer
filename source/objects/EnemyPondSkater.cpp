#include "pch.h"
#include "multiplayer.h"

static void __cdecl PondDisplayer_r(task* tp);
Trampoline PondDisplayer_t(0x7AA3D0, 0x7AA3D5, PondDisplayer_r);
static void __cdecl PondDisplayer_r(task* tp)
{
    if (tp->twp->mode < 4)
    {
        TARGET_STATIC(PondDisplayer)(tp);
    }
}
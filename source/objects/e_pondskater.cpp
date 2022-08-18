#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"

static void __cdecl PondDisplayer_r(task* tp);
Trampoline PondDisplayer_t(0x7AA3D0, 0x7AA3D5, PondDisplayer_r);
static void __cdecl PondDisplayer_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        if (tp->twp->mode < 4)
        {
            PondDraw(tp->twp, (enemywk*)tp->mwp);
        }
    }
    else
    {
        TARGET_STATIC(PondDisplayer)(tp);
    }
}

void PatchPondSkater()
{
    WriteData<5>((void*)0x7AA627, 0x90ui8); // remove redundant SetVelocityP
    WriteData<5>((void*)0x7AA7A9, 0x90ui8); // remove redundant SetVelocityP
    WriteData<5>((void*)0x7AA707, 0x90ui8); // remove redundant SetVelocityP
}

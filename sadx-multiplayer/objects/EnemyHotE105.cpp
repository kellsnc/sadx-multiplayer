#include "pch.h"

Trampoline* e105_calcSomeValue_t = nullptr;
Trampoline* e105_chkPlayerRangeIn_t = nullptr;
Trampoline* e105_moveBattery_t = nullptr;

// Unknown main struct so no full rewrite for now

#pragma region calcSomeValue
static void e105_calcSomeValue_o(task* tp)
{
    auto target = e105_calcSomeValue_t->Target();
    __asm
    {
        mov eax, [tp]
        call target
    }
}

static void __cdecl e105_calcSomeValue_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        auto pltp = playertp[0];
        auto plmwp = playermwp[0];
        auto pnum = GetTheNearestPlayerNumber(&tp->twp->pos);
        playertp[0] = playertp[pnum];
        playermwp[0] = playermwp[pnum];
        e105_calcSomeValue_o(tp);
        playertp[0] = pltp;
        playermwp[0] = plmwp;
    }
    else
    {
        e105_calcSomeValue_o(tp);
    }
}

static void __declspec(naked) e105_calcSomeValue_w()
{
    __asm
    {
        push eax
        call e105_calcSomeValue_r
        pop eax
        retn
    }
}
#pragma endregion

#pragma region moveBattery
static void __cdecl e105_moveBattery_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        auto pltp = playertp[0];
        playertp[0] = playertp[GetTheNearestPlayerNumber(&tp->twp->pos)];
        TARGET_DYNAMIC(e105_moveBattery)(tp);
        playertp[0] = pltp;
    }
    else
    {
        TARGET_DYNAMIC(e105_moveBattery)(tp);
    }
}
#pragma endregion

#pragma region chkPlayerRangeIn
static BOOL __cdecl e105_chkPlayerRangeIn_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        auto twp = tp->twp;

        if (IsPlayerInSphere(&tp->twp->pos, 150.0f))
        {
            twp->smode = 2;
            ccsi_flag = 1ui8;
        }

        return FALSE;
    }
    else
    {
        return TARGET_DYNAMIC(e105_chkPlayerRangeIn)(tp);
    }
}
#pragma endregion

void InitE105Patches()
{
    e105_moveBattery_t = new Trampoline(0x5A40B0, 0x5A40B7, e105_moveBattery_r);
    e105_calcSomeValue_t = new Trampoline(0x5A3860, 0x5A3865, e105_calcSomeValue_w);

    e105_chkPlayerRangeIn_t = new Trampoline(0x5A3670, 0x5A3677, e105_chkPlayerRangeIn_r);
    WriteCall((void*)((int)e105_chkPlayerRangeIn_t->Target() + 2), (void*)0x441AC0); // Patch trampoline
}
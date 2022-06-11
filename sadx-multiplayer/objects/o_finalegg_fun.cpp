#include "pch.h"
#include "SADXModLoader.h"
#include "multiplayer.h"

// Fans in Final Egg Act 3

DataPointer(float, FunAcc, 0x1AC4854);
DataPointer(float, FunAccRate, 0x1AC4858);

static void Fun_ExecATask_w();
Trampoline Fun_ExecATask_t(0x5B7480, 0x5B7487, Fun_ExecATask_w);

static void Fun_ExecATask_o(task* tp)
{
    auto target = Fun_ExecATask_t.Target();
    __asm
    {
        mov edi, [tp]
        call target
    }
}

static void ExecATask_m(task* tp)
{
    auto twp = tp->twp;

    bool sound_played = false;
    for (int i = 0; i < PLAYER_MAX; ++i)
    {
        auto ptwp = playertwp[i];

        if (!ptwp)
            continue;

        if (twp->pos.y - 3.0f < ptwp->pos.y)
        {
            auto x = ptwp->pos.x - twp->pos.x;
            auto z = ptwp->pos.z - twp->pos.z;
            if (z * z + x * x < 576.0f)
            {
                auto power = FunAcc - (ptwp->pos.y - twp->pos.y) * FunAccRate;
                
                if (power < 0.0f)
                {
                    power = 0.0f;
                }

                SetAscendPowerP(i, 0, power, 0);

                if (!sound_played && twp->counter.f == 0.0f)
                {
                    dsPlay_timer(289, (int)twp, 1, 0, 20);
                    sound_played = true;
                }
            }
        }
    }
    
    twp->counter.f += 1.0f;
    if (twp->counter.f >= action_fun_funflot.motion->nbFrame)
    {
        twp->counter.f = 0.0f;
    }
    EntryColliList(twp);
}

static void __cdecl Fun_ExecATask_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        ExecATask_m(tp);
    }
    else
    {
        Fun_ExecATask_o(tp);
    }
}

static void __declspec(naked) Fun_ExecATask_w()
{
    __asm
    {
        push edi
        call Fun_ExecATask_r
        pop edi
        retn
    }
}

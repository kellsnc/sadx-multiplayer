#include "pch.h"
#include "multiplayer.h"

// Breakable ice in Big's Ice Cap

static void CheckBreakFuta_w();
Trampoline CheckBreakFuta_t(0x4EEA10, 0x4EEA15, CheckBreakFuta_w);

BOOL CheckBreakFuta_o(task* tp)
{
	auto target = CheckBreakFuta_t.Target();
	BOOL r;
	__asm
	{
		mov edi, [tp]
		call target
		mov r, eax
	}
	return r;
}

BOOL CheckBreakFuta_m(task* tp)
{
    auto  twp = tp->twp;
    auto cwp = twp->cwp;

    if (cwp->hit_num == 1 && (cwp->flag & 1))
    {
       auto mytask = cwp->hit_cwp->mytask;
       if (mytask->twp->id == 6) // rock
       {
           auto mwp = mytask->mwp;
           if (mwp)
           {
               if (njScalor(&mwp->spd) > 0.1f)
               {
                   return TRUE;
               }
           }
       }
    }

    auto pnum = CheckCollisionCylinderP(&twp->pos, cwp->info->a, cwp->info->b) - 1;

    if (pnum >= 0)
    {
        auto ptwp = playertwp[pnum];
        auto pmwp = playermwp[pnum];

        if (ptwp && TASKWK_CHARID(ptwp) == Characters_Big && pmwp)
        {
            if (ptwp->pos.y + 3.0f <= twp->pos.y)
            {
                if (pmwp->spd.y > 1.5f)
                {
                    return TRUE;
                }
            }
            else if (pmwp->spd.y < -1.5f)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL __cdecl CheckBreakFuta_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		CheckBreakFuta_m(tp);
	}
	else
	{
		return CheckBreakFuta_o(tp);
	}
}

static void __declspec(naked) CheckBreakFuta_w()
{
	__asm
	{
		push edi
		call CheckBreakFuta_r
		add esp, 4
		retn
	}
}
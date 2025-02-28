#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"

void __cdecl Magmacnt_Main_r(task* obj);
FastFunctionHook<void, task*> Magmacnt_Main_t(0x608730);

void __cdecl Magmacnt_Main_r(task* tp)
{
	auto twp = tp->twp;

	if (!multiplayer::IsActive() || !twp->mode)
	{
		return Magmacnt_Main_t.Original(tp);
	}

	if (!CheckRangeOut(tp))
	{
		if (twp->mode == 1)
		{
			if (twp->cwp->flag & 1)
			{
				if (rd_mountain_twp && rd_mountain_twp->pos.y < twp->scl.y)
				{
					rd_mountain_twp->scl.x = twp->scl.y;
					rd_mountain_twp->scl.y = twp->scl.z;
				}
				twp->mode = 2;
			}
			EntryColliList(twp);
		}
		else if (twp->mode == 2)
		{
			if (!(twp->cwp->flag & 1))
			{
				twp->mode = 1;
			}
		}
	}
}

void patch_mountain_magmacnt_init()
{
	Magmacnt_Main_t.Hook(Magmacnt_Main_r);
	WriteData<2>((void*)0x6087D6, 0x90ui8); // Remove explicit P1 check. Note: this makesP1 tails able to trigger lava
}

RegisterPatch patch_mountain_magmacnt(patch_mountain_magmacnt_init);
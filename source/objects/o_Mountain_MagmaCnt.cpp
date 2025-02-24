#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"

void __cdecl Magmacnt_Main_r(task* obj);
FastFunctionHook<void, task*> Magmacnt_Main_t(0x608730, Magmacnt_Main_r);
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
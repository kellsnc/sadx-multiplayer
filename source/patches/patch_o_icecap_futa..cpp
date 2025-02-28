#include "pch.h"
#include "multiplayer.h"

// Breakable ice in Big's Ice Cap

Bool __cdecl CheckBreakFuta_r(task* tp);
FastUsercallHookPtr<decltype(&CheckBreakFuta_r), rEAX, rEDI> CheckBreakFuta_t(0x4EEA10);

Bool CheckBreakFuta_m(task* tp)
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

Bool __cdecl CheckBreakFuta_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		return CheckBreakFuta_m(tp);
	}
	else
	{
		return CheckBreakFuta_t.Original(tp);
	}
}

void patch_icecap_futa_init()
{
	CheckBreakFuta_t.Hook(CheckBreakFuta_r);
}

RegisterPatch patch_icecap_futa(patch_icecap_futa_init);
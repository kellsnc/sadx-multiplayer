#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"

// Patch multiple cannon in SD to make every player bounce on them

void dsHangPoleCom_r(taskwk* a1);
FastFunctionHook<void, taskwk*> dsHangPoleCom_h(0x5FE570);

void dsHangPoleCom_r(taskwk* a1)
{
	if (multiplayer::IsActive())
	{
		if (a1 && a1->cwp && a1->cwp->hit_cwp && a1->cwp->hit_cwp->mytask)
		{
			taskwk* twp = a1->cwp->hit_cwp->mytask->twp;

			for (int i = 0; i < PLAYER_MAX; i++)
			{
				if (twp == playertwp[i])
				{
					EnemyBounceThing(i, 0.0f, 2.0f, 0.0f);
					twp->pos.y = twp->pos.y + 1.0f;
					return;
				}
			}
		}
		else
		{
			return;
		}
	}

	dsHangPoleCom_h.Original(a1);
}

void patch_sky_cannon_cmn_init()
{
	dsHangPoleCom_h.Hook(dsHangPoleCom_r);
}

RegisterPatch patch_sky_cannon_cmn(patch_sky_cannon_cmn_init);
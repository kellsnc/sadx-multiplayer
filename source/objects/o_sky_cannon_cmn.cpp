#include "pch.h"

//patch multiple cannon in SD to make every player bounce on them
void dsHangPoleCom_r(taskwk* a1);
static FunctionHook<void, taskwk*> dsHangPoleCom_t((intptr_t)0x5FE570, dsHangPoleCom_r);

void dsHangPoleCom_r(taskwk* a1)
{
	if (multiplayer::IsActive())
	{
		if (a1 && a1->cwp && a1->cwp->hit_cwp && a1->cwp->hit_cwp->mytask)
		{
			auto data = a1->cwp->hit_cwp->mytask->twp;

			for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
			{
				if (data == playertwp[i])
				{
					EnemyBounceThing(i, 0.0, 2.0, 0.0);
					data->pos.y = data->pos.y + 1.0;
					return;
				}
			}
		}
		else
		{
			return;
		}
	}

	dsHangPoleCom_t.Original(a1);
}
#include "pch.h"

void __cdecl Magmacnt_Main_r(task* obj);

TaskHook Magmacnt_Main_t(0x608730, Magmacnt_Main_r);

void __cdecl Magmacnt_Main_r(task* obj)
{
	auto data = obj->twp;

	if (!multiplayer::IsEnabled() || !data->mode)
	{
		return 	Magmacnt_Main_t.Original(obj);
	}

	if (!ClipSetObject((ObjectMaster*)obj))
	{
		if (data->mode == 1)
		{
			for (int i = 0; i < multiplayer::GetPlayerCount(); i++) {
				if ((data->cwp->flag & 1) != 0 && data->cwp->hit_cwp->mytask == (task*)GetCharacterObject(i))
				{
					if (rd_mountain_twp)
					{
						rd_mountain_twp->scl.x = data->scl.y;
						rd_mountain_twp->scl.y = data->scl.z;
					}
					data->mode = 2;
				}
			}
			AddToCollisionList((EntityData1*)data);
		}
		else if (data->mode == 2)
		{
			for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
			{
				if (playerpwp[i])
				{
					if (playerpwp[i]->item & Powerups_Dead)
					{
						obj->twp->mode = 0;
						break;
					}
				}
			}
		}
	}
}
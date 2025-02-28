#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

FastFunctionHook<task*, NJS_POINT3*, NJS_POINT3*, float> SetCircleLimit_t(0x7AF3E0);

task* __cdecl SetCircleLimit_r(NJS_POINT3* pos, NJS_POINT3* center, float radius)
{
	if (multiplayer::IsActive())
	{
		for (int i = 1; i < PLAYER_MAX; ++i)
		{
			if (playertwp[i])
			{
				task* tp = CreateElementalTask((LoadObj_Data1), 0, CircleLimit);

				if (tp)
				{
					auto twp = tp->twp;
					twp->pos = *center;
					twp->scl.x = radius;
					twp->counter.ptr = &playertwp[i]->pos;
				}
			}
		}
	}

	return SetCircleLimit_t.Original(pos, center, radius);
}

void patch_chaos_misc_init()
{
	SetCircleLimit_t.Hook(SetCircleLimit_r);
}

RegisterPatch patch_chaos_misc(patch_chaos_misc_init);
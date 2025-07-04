#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "multiplayer.h"

FastFunctionHook<void, task*> Rd_Windy_h(0x4DDB30);

// Patch act swaps
void __cdecl Rd_Windy_r(task* tp)
{
	if (ssActNumber == 1 && multiplayer::IsActive())
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (playertwp[i] && playertwp[i]->pos.y > 2250.0f)
			{
				tp->twp->mode = 0;
				ChangeActM(1);
				return;
			}
		}
	}

	Rd_Windy_h.Original(tp);
}

void patch_rd_windy_init()
{
	Rd_Windy_h.Hook(Rd_Windy_r);
	WriteData((void**)0x4DDBFE, (void*)0x4DDB60); // Patch skybox mode
}

RegisterPatch patch_rd_windy(patch_rd_windy_init);
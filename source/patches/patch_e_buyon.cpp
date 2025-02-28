#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

// EBuyon is the only object to manually call AddEnemyScore
void __cdecl EBuyon_ScorePatch(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto player = CCL_IsHitPlayer(tp->twp);

		if (player)
		{
			AddEnemyScoreM(TASKWK_PLAYERID(player), 100);
		}
	}
	else
	{
		AddEnemyScore(100);
	}
}

void patch_buyon_init()
{
	WriteCall((void*)0x7B3273, EBuyon_ScorePatch); // Add 100 points to proper player
	WriteData<5>((void*)0x7B326D, 0x90); // Rremove original 100 points for player 0
}

RegisterPatch patch_buyon(patch_buyon_init);
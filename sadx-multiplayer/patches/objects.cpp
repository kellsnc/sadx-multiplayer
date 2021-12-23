#include "pch.h"
#include "objects.h"

Trampoline* Ring_t = nullptr;

// Patch for other players to collect rings
void __cdecl Ring_r(task* tp)
{
	if (IsMultiplayerEnabled())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1)
		{
			auto player = CCL_IsHitPlayer(twp);
			
			if (player)
			{
				int pID = TASKWK_PLAYERID(player);

				if (!(playerpwp[pID]->item & 0x4000))
				{
					twp->mode = 2;
					AddRingsM(pID, 1);
					dsPlay_oneshot(7, 0, 0, 0);
					tp->disp = RingDoneDisplayer;
					return;
				}
			}
		}
	}

	TARGET_DYNAMIC(Ring)(tp);
}

void InitObjectPatches()
{
	Ring_t = new Trampoline(0x450370, 0x450375, Ring_r);
}
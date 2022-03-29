#include "pch.h"
#include "multiplayer.h"

static void __cdecl ObjectFinalEggRainbowDoor_r(task* tp);
Trampoline ObjectFinalEggRainbowDoor_t(0x5B0910, 0x5B0916, ObjectFinalEggRainbowDoor_r);
static void __cdecl ObjectFinalEggRainbowDoor_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 1)
	{
		auto twp = tp->twp;

		if (IsPlayerInSphere(&twp->pos, 40.0f))
		{
			twp->mode = 4;
			dsPlay_oneshot_Dolby(283, 0, 0, 0, 120, twp);
		}

		EntryColliList(twp);
		tp->disp(tp);
	}
	else
	{
		TARGET_STATIC(ObjectFinalEggRainbowDoor)(tp);
	}
}
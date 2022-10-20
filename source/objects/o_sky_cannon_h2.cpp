#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

// Big Cannon at the end of act 1

static void __cdecl ObjectSkydeck_cannon_h2_Exec_r(task* tp);
Trampoline ObjectSkydeck_cannon_h2_Exec_t(0x5F38F0, 0x5F38F5, ObjectSkydeck_cannon_h2_Exec_r);
static void __cdecl ObjectSkydeck_cannon_h2_Exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto backup = playertwp[0];
		playertwp[0] = playertwp[GetTheNearestPlayerNumber(&tp->twp->pos)];
		TARGET_STATIC(ObjectSkydeck_cannon_h2_Exec)(tp);
		playertwp[0] = backup;
	}
	else
	{
		TARGET_STATIC(ObjectSkydeck_cannon_h2_Exec)(tp);
	}
}
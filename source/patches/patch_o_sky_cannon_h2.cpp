#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

// Big Cannon at the end of act 1

static void __cdecl ObjectSkydeck_cannon_h2_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectSkydeck_cannon_h2_Exec_r)> ObjectSkydeck_cannon_h2_Exec_h(0x5F38F0);

static void __cdecl ObjectSkydeck_cannon_h2_Exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto backup = playertwp[0];
		playertwp[0] = playertwp[GetTheNearestPlayerNumber(&tp->twp->pos)];
		ObjectSkydeck_cannon_h2_Exec_h.Original(tp);
		playertwp[0] = backup;
	}
	else
	{
		ObjectSkydeck_cannon_h2_Exec_h.Original(tp);
	}
}

void patch_sky_cannon_h2_init()
{
	ObjectSkydeck_cannon_h2_Exec_h.Hook(ObjectSkydeck_cannon_h2_Exec_r);
}

RegisterPatch patch_sky_cannon_h2(patch_sky_cannon_h2_init);
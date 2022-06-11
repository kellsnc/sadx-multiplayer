#include "pch.h"
#include "gravity.h"

static void __cdecl SonicTheHedgehog_r(task* tp);
Trampoline SonicTheHedgehog_t(0x49A9B0, 0x49A9B7, SonicTheHedgehog_r);
static void __cdecl SonicTheHedgehog_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(tp->twp);
		gravity::SaveGlobalGravity();
		gravity::SwapGlobalToUserGravity(pnum);
		TARGET_STATIC(SonicTheHedgehog)(tp);
		gravity::RestoreGlobalGravity();
	}
	else
	{
		TARGET_STATIC(SonicTheHedgehog)(tp);
	}
}
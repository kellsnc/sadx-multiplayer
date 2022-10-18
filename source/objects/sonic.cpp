#include "pch.h"
#include "gravity.h"


TaskHook SonicTheHedgehog_t(0x49A9B0);
static void __cdecl SonicTheHedgehog_r(task* tp)
{
	auto co2 = (playerwk*)tp->mwp->work.l;

	if (co2)
	{
		co2 = co2;
	}

	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(tp->twp);
		gravity::SaveGlobalGravity();
		gravity::SwapGlobalToUserGravity(pnum);
		SonicTheHedgehog_t.Original(tp);
		gravity::RestoreGlobalGravity();
	}
	else
	{
		SonicTheHedgehog_t.Original(tp);
	}
}


void initSonicPatch()
{
	SonicTheHedgehog_t.Hook(SonicTheHedgehog_r);
}
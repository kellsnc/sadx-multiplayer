#include "pch.h"
#include "multiplayer.h"

// Replace the ladder at the end of the casinopolis sewer with a capsule for multiplayer trial

static void __cdecl ObjectCasinoHasigo_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectCasinoHasigo_Exec_r)> ObjectCasinoHasigo_Exec_t(0x5CC5F0, ObjectCasinoHasigo_Exec_r);
static void __cdecl ObjectCasinoHasigo_Exec_r(task* tp)
{
	if (multiplayer::IsEnabled() && GameMode == GameModes_Trial)
	{
		tp->twp->pos.y = -2205.0f;
		tp->twp->pos.z = 1750.0f;
		tp->exec = ObjectReleaseBox;
		tp->exec(tp);
	}
	else
	{
		ObjectCasinoHasigo_Exec_t.Original(tp);
	}
}
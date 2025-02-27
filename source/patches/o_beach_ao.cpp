#include "pch.h"

void __cdecl ObjectBeachAttackOrukaSummon_r(task* tp);
void __cdecl ObjectBeachPathOrukaSummon_r(task* tp);

FastFunctionHookPtr<decltype(&ObjectBeachAttackOrukaSummon_r)> ObjectBeachAttackOrukaSummon_t(0x4FBE40);
FastFunctionHookPtr<decltype(&ObjectBeachPathOrukaSummon_r)> ObjectBeachPathOrukaSummon_t(0x4FBBE0);

void __cdecl ObjectBeachAttackOrukaSummon_r(task* tp)
{
	if (!multiplayer::IsEnabled())
	{
		ObjectBeachAttackOrukaSummon_t.Original(tp);
	}
}

void __cdecl ObjectBeachPathOrukaSummon_r(task* tp)
{
	if (!multiplayer::IsEnabled())
	{
		ObjectBeachPathOrukaSummon_t.Original(tp);
	}
}

void patch_beach_ao_init()
{
	ObjectBeachAttackOrukaSummon_t.Hook(ObjectBeachAttackOrukaSummon_r);
	ObjectBeachPathOrukaSummon_t.Hook(ObjectBeachPathOrukaSummon_r);
}

RegisterPatch patch_beach_ao(patch_beach_ao_init);
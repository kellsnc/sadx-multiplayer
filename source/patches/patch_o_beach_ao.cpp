#include "pch.h"

void __cdecl ObjectBeachAttackOrukaSummon_r(task* tp);
void __cdecl ObjectBeachPathOrukaSummon_r(task* tp);

FastFunctionHookPtr<decltype(&ObjectBeachAttackOrukaSummon_r)> ObjectBeachAttackOrukaSummon_h(0x4FBE40);
FastFunctionHookPtr<decltype(&ObjectBeachPathOrukaSummon_r)> ObjectBeachPathOrukaSummon_h(0x4FBBE0);

void __cdecl ObjectBeachAttackOrukaSummon_r(task* tp)
{
	if (!multiplayer::IsEnabled())
	{
		ObjectBeachAttackOrukaSummon_h.Original(tp);
	}
}

void __cdecl ObjectBeachPathOrukaSummon_r(task* tp)
{
	if (!multiplayer::IsEnabled())
	{
		ObjectBeachPathOrukaSummon_h.Original(tp);
	}
}

void patch_beach_ao_init()
{
	ObjectBeachAttackOrukaSummon_h.Hook(ObjectBeachAttackOrukaSummon_r);
	ObjectBeachPathOrukaSummon_h.Hook(ObjectBeachPathOrukaSummon_r);
}

RegisterPatch patch_beach_ao(patch_beach_ao_init);
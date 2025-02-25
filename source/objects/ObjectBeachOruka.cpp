#include "pch.h"

void __cdecl ObjectBeachAttackOrukaSummon_r(task* tp);
void __cdecl ObjectBeachPathOrukaSummon_r(task* tp);

FastFunctionHookPtr<decltype(&ObjectBeachAttackOrukaSummon_r)> ObjectBeachAttackOrukaSummon_t(0x4FBE40, ObjectBeachAttackOrukaSummon_r);
FastFunctionHookPtr<decltype(&ObjectBeachPathOrukaSummon_r)> ObjectBeachPathOrukaSummon_t(0x4FBBE0, ObjectBeachPathOrukaSummon_r);

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
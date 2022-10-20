#include "pch.h"

void __cdecl ObjectBeachAttackOrukaSummon_r(task* tp);
void __cdecl ObjectBeachPathOrukaSummon_r(task* tp);

Trampoline ObjectBeachAttackOrukaSummon_t(0x4FBE40, 0x4FBE47, ObjectBeachAttackOrukaSummon_r);
Trampoline ObjectBeachPathOrukaSummon_t(0x4FBBE0, 0x4FBBE7, ObjectBeachPathOrukaSummon_r);

void __cdecl ObjectBeachAttackOrukaSummon_r(task* tp)
{
	if (!multiplayer::IsEnabled())
	{
		TARGET_STATIC(ObjectBeachAttackOrukaSummon)(tp);
	}
}

void __cdecl ObjectBeachPathOrukaSummon_r(task* tp)
{
	if (!multiplayer::IsEnabled())
	{
		TARGET_STATIC(ObjectBeachPathOrukaSummon)(tp);
	}
}
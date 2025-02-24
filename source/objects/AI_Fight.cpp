#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "patches.h"

FastFunctionHook<void, int> LoadCharacterBoss_t(LoadCharacterBoss);

static uint8_t aiPAD = 7;

void SetNPCController()
{
	aiPAD = multiplayer::IsActive() ? 7 : 1;

	//Knux AI
	WriteData<1>((uint8_t*)0x4D5E3A, aiPAD);
	WriteData((int*)0x4D5E42, (int)&Controllers[aiPAD].HeldButtons);
	WriteData((int*)0x4D5E48, (int)&Controllers[aiPAD].PressedButtons);

	WriteData((int*)0x4D64EE, (int)&NormalizedAnalogs[aiPAD].direction);
	WriteData((int*)0x4D64F4, (int)&NormalizedAnalogs[aiPAD].magnitude);

	WriteData((int*)0x4D64FA, (int)&Controllers[aiPAD].HeldButtons);
	WriteData((int*)0x4D6500, (int)&Controllers[aiPAD].PressedButtons);
	WriteData((int*)0x4D644A, (int)&Controllers[aiPAD].LeftStickY);

	//Sonic AI
	WriteData((int*)0x4B646C, (int)&Controllers[aiPAD].HeldButtons);
	WriteData((int*)0x4B6472, (int)&Controllers[aiPAD].PressedButtons);

}

void __cdecl LoadCharacterBoss_r(int boss_id)
{
	//SetNPCController();

	if (multiplayer::IsActive())
	{
		auto P = playertp[1];
		if (P)
		{
			FreeTask(P);
		}
	}

	LoadCharacterBoss_t.Original(boss_id);
}

void init_AIFight_Patches()
{
	LoadCharacterBoss_t.Hook(LoadCharacterBoss_r);
}
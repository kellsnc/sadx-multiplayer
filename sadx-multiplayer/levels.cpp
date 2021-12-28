#include "pch.h"
#include "levels.h"

/*

Level-related adjustements for multiplayer

*/

Trampoline* FogColorChange_t = nullptr;
Trampoline* SetPlayerInitialPosition_t = nullptr;

// Put players side by side
void __cdecl SetPlayerInitialPosition_r(taskwk* twp)
{
	TARGET_DYNAMIC(SetPlayerInitialPosition)(twp);

	if (IsMultiplayerEnabled())
	{
		static const int dists[]
		{
			-5.0f,
			5.0f,
			-10.0f,
			10.0f
		};

		twp->pos.x += njCos(twp->ang.y + 0x4000) * dists[TASKWK_PLAYERID(twp)];
		twp->pos.z += njSin(twp->ang.y + 0x4000) * dists[TASKWK_PLAYERID(twp)];
	}
}

static void FogColorChange_r(task* tp)
{
	if (!IsMultiplayerEnabled())
	{
		auto target = TARGET_DYNAMIC(FogColorChange);

		__asm
		{
			mov eax, [tp]
			call target
		}
	}
}

static void __declspec(naked) FogColorChange_w()
{
	__asm
	{
		push eax
		call FogColorChange_r
		pop eax
		retn
	}
}

void InitLevels()
{
	// Windy Valley tornade effects
	FogColorChange_t = new Trampoline(0x4DD240, 0x4DD246, FogColorChange_w);
	SetPlayerInitialPosition_t = new Trampoline(0x414810, 0x414815, SetPlayerInitialPosition_r);

	// Patch Skyboxes (display function managing mode)
	WriteData((void**)0x4F723E, (void*)0x4F71A0); // Emerald Coast
	WriteData((void**)0x4DDBFE, (void*)0x4DDB60); // Windy Valley
	WriteData((void**)0x61D57E, (void*)0x61D4E0); // Twinkle Park
	WriteData((void**)0x610A7E, (void*)0x6109E0); // Speed Highway
	WriteData((void**)0x5E1FCE, (void*)0x5E1F30); // Lost World
	WriteData((void**)0x4EA26E, (void*)0x4EA1D0); // Ice Cap
}
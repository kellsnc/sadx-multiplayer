#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

// Emerald Coast launching ramp for all players (staticly fixed)

void patch_beach_jumpraft_init()
{
	WriteData((uint8_t*)0x500017, (uint8_t)PLAYER_MAX);
}

RegisterPatch patch_beach_jumpraft(patch_beach_jumpraft_init);
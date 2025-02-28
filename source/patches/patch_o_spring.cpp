#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

// Springs for all players (staticly fixed)

void patch_spring_init()
{
	WriteData((uint8_t*)0x7A4DC4, (uint8_t)PLAYER_MAX); // ObjectSpring
	WriteData((uint8_t*)0x7A4FF7, (uint8_t)PLAYER_MAX); // ObjectSpringB
	WriteData((uint8_t*)0x79F77C, (uint8_t)PLAYER_MAX); // spring_h_exec
}

RegisterPatch patch_spring(patch_spring_init);
#include "pch.h"
#include "RegisterPatch.hpp"

void patch_ta_twinkle_init()
{
	WriteData((intptr_t*)0x61E3DB, 0); // Remove exec function in display callback
}

RegisterPatch patch_ta_twinkle(patch_ta_twinkle_init);
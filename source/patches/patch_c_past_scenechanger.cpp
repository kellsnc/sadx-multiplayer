#include "pch.h"
#include "SADXModLoader.h"
#include "RegisterPatch.hpp"

void patch_c_past_scenechanger_init()
{
	WriteData((uint8_t*)0x54575C, 0x7Cui8); // Replace == 1 with != 0
}

RegisterPatch patch_c_past_scenechanger(patch_c_past_scenechanger_init);
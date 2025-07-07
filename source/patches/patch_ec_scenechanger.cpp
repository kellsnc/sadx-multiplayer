#include "pch.h"

//scene change when entering door on EC

void patch_ec_scenechanger_init()
{
	WriteData((uint8_t*)0x52D803, 0x7Cui8); // Replace == 1 with != 0
	WriteData((uint8_t*)0x525161, 0x7Cui8); // Replace == 1 with != 0
}

RegisterPatch patch_ec_scenechanger(patch_ec_scenechanger_init);
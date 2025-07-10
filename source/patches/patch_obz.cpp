#include "pch.h"


//this also patch lost world and final egg when capusle isn't there
void patch_beach_obz_init()
{
	WriteData((uint8_t*)0x5B24D6, 0x7Cui8); // Replace == 1 with != 0
}

RegisterPatch patch_beach_obz(patch_beach_obz_init);
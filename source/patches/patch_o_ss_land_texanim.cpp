#include "pch.h"
#include "RegisterPatch.hpp"

void patch_ss_land_texanim_init()
{
	WriteData((intptr_t*)0x63179A, 0); // Remove exec function in display callback
}

RegisterPatch patch_ss_land_texanim(patch_ss_land_texanim_init);
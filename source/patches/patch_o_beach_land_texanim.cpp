#include "pch.h"
#include "RegisterPatch.hpp"

void patch_beach_land_texanim_init()
{
	// Remove exec function in display callback
	WriteData((intptr_t*)0x4F734B, 0); // BeachTexAnimAct1
	WriteData((intptr_t*)0x4F73BB, 0); // BeachTexAnimWaveAct1
	WriteData((intptr_t*)0x4F743B, 0); // BeachTexAnimWaveAct2
	WriteData((intptr_t*)0x4F734B, 0); // BeachTexAnimSandAct2
	WriteData((intptr_t*)0x4F752B, 0); // BeachTexAnimWtAct2
	WriteData((intptr_t*)0x4F759B, 0); // BeachTexAnimWaveAct3
	WriteData((intptr_t*)0x4F761B, 0); // BeachTexAnimSandAct3
	WriteData((intptr_t*)0x4F768B, 0); // BeachTexAnimWtAct3
}

RegisterPatch patch_beach_land_texanim(patch_beach_land_texanim_init);
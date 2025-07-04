#include "pch.h"
#include "RegisterPatch.hpp"

void patch_ruin_texanim_init()
{
	// Remove exec function in display callback
	WriteData((intptr_t*)0x5E20DB, 0); // RuinTexAnimAct1
	WriteData((intptr_t*)0x5E214B, 0); // RuinTexAnimAct2
	WriteData((intptr_t*)0x5E229B, 0); // RuinTexAnimAct3
}

RegisterPatch patch_ruin_texanim(patch_ruin_texanim_init);
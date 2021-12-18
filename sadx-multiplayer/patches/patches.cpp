#include "pch.h"
#include "player_patches.h"
#include "deathzones.h"

void InitPatches()
{
	InitPlayerPatches();
	init_DeathPatches();
}
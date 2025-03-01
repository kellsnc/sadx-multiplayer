#include "pch.h"
#include "SADXModLoader.h"
#include "RegisterPatch.hpp"
#include "multiplayer.h"
#include "teleport.h"

// Teleport all players in multiplayer mode
void SetPlayerInitialPosition_AllHack(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		TeleportPlayersToStart();
	}
	else
	{
		SetPlayerInitialPosition(twp);
	}
}

void patch_game_init()
{
	// Patch SetPlayerInitialPosition occurences that should teleport all players
	WriteCall((void*)0x4150FA, SetPlayerInitialPosition_AllHack); // General
	WriteCall((void*)0x4151B1, SetPlayerInitialPosition_AllHack); // General
	WriteJump((void*)0x7B0B00, SetPlayerInitialPosition_AllHack); // General
	WriteCall((void*)0x4DD52D, SetPlayerInitialPosition_AllHack); // Windy Valley
	WriteCall((void*)0x5E15CA, SetPlayerInitialPosition_AllHack); // Lost World
	WriteCall((void*)0x5EDC66, SetPlayerInitialPosition_AllHack); // Sky Deck
	WriteCall((void*)0x5EFA31, SetPlayerInitialPosition_AllHack); // Sky Deck
	WriteCall((void*)0x5EDD27, SetPlayerInitialPosition_AllHack); // Sky Deck
	WriteCall((void*)0x5602F1, SetPlayerInitialPosition_AllHack); // Perfect Chaos
}

RegisterPatch patch_game(patch_game_init);
#pragma once

#include "ninja.h"

void TeleportPlayer(int pnum, float x, float y, float z);
void TeleportPlayer(int pnum, NJS_VECTOR* pPos);
void TeleportPlayers(float x, float y, float z);
void TeleportPlayers(NJS_VECTOR* pPos);
void TeleportPlayerArea(int pnum, NJS_POINT3* pin, Float dist);
void TeleportPlayersToPlayer(int pnum);
void TeleportPlayerToStart(int pnum);
void TeleportPlayersToStart();

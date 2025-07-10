#pragma once

#define PLAYER_MAX 4

extern const HelperFunctions* gHelperFunctions;
extern bool DreamcastConversionEnabled;
extern bool CharacterSelectEnabled;

#define TaskHook FastFunctionHook<void, task*>
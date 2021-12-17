#pragma once

#define TARGET_DYNAMIC(name) ((decltype(name##_r)*)name##_t->Target())
#define PLAYER_MAX 4

bool IsMultiplayerEnabled();

extern const HelperFunctions* gHelperFunctions;
extern unsigned int player_count;
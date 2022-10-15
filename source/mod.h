#pragma once

#define TARGET_DYNAMIC(name) ((decltype(name##_r)*)name##_t->Target())
#define TARGET_STATIC(name) ((decltype(name##_r)*)name##_t.Target())
#define PLAYER_MAX 4
#define BYTEn(x, n)   (*((uint8_t*)&(x)+n))
#define TaskHook static FunctionHook<void, task*>
#define SDCannonMode 110

extern const HelperFunctions* gHelperFunctions;
extern bool DreamcastConversionEnabled;

void initEvents();
void initGammaPatch();
void Init_KnuxPatches();
void initSonicPatch();
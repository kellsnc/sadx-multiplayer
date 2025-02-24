#pragma once

#define TARGET_DYNAMIC(name) ((decltype(name##_r)*)name##_t->Target())
#define TARGET_STATIC(name) ((decltype(name##_r)*)name##_t.Target())
#define PLAYER_MAX 4
#define BYTEn(x, n)   (*((uint8_t*)&(x)+n))

enum customAction
{
	SDCannonMode = 110,
	SDCylStd,
	SDCylDown,
	SDCylLeft,
	SDCylRight
};

extern const HelperFunctions* gHelperFunctions;
extern bool DreamcastConversionEnabled;

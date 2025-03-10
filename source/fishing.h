#pragma once

struct BIGETC // struct doesn't actually exist, but all of the global variables are adjacent
{
	int Sakanu_Num;
	__int16 Big_Fish_Flag;
	int Big_Sakana_Weight_Limit;
	float reel_tension_add;
	task* Big_Fish_Ptr;
	int Big_Fishing_Timer;
	task* Big_Lure_Ptr;
	NJS_POINT3 big_item_pos;
	int Big_Sakana_Weight;
	int Big_Sakana_Kind_High;
	int Big_Stg12_Flag;
	__int16 Big_Sakana_Weight_High;
	float water_level;
	float reel_length;
	float reel_length_d;
	float reel_tension;
	float reel_tension_aim;
	Angle reel_angle;
	int caution_timer;
	NJS_OBJECT* lure_kind[7];
	SaveFileData sd;
	float distance;
	float distancep;
};

enum : Uint16
{
	LUREFLAG_HIT = 0x1,       // Hit mode
	LUREFLAG_RELEASE = 0x2,   // Fish released
	LUREFLAG_RANGEOUT = 0x4,  // Hook outside of range
	LUREFLAG_ESCAPE = 0x8,    // Resistance from fish
	LUREFLAG_COL = 0x10,      // Fish touched floor/wall
	LUREFLAG_SWING = 0x20,    // Move line left/right
	LUREFLAG_FISH = 0x40,     // Currently fishing
	LUREFLAG_HOOK = 0x80,     // Hot shelter hook
	LUREFLAG_KAERU = 0x400,   // Froggy caught
	LUREFLAG_MISS = 0x800,    // Fish caught
	LUREFLAG_REEL = 0x1000,   // Currently reeling
	LUREFLAG_PTCL = 0x2000,   // Particule spawned
	LUREFLAG_CANCEL = 0x4000, // Player cancelled fishing
	LUREFLAG_LAUNCH = 0x8000, // Enable fishing
};

float GetWaterLevel_m(BIGETC* etc);
float GetReelLength_m(BIGETC* etc);
void CalcHookPos_m(BIGETC* etc, NJS_POINT3* ret);
bool ChkFishingThrowNow_m(int pnum);

BIGETC* GetBigEtc(int pnum);
void InitFishing();
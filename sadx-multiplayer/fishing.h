#pragma once

__declspec(align(4)) struct BIGETC // struct doesn't actually exists, but all of the global variables are adjacent
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

enum : __int16 // made up
{
	LUREFLAG_HIT = 0x1,       // Hit mode
	LUREFLAG_RELEASE = 0x2,   // Fish released
	LUREFLAG_RANGEOUT = 0x4,  // Hook outside of range
	LUREFLAG_8 = 0x8,
	LUREFLAG_10 = 0x10,
	LUREFLAG_SWING = 0x20,    // Move line left/right
	LUREFLAG_FISH = 0x40,     // Currently fishing
	LUREFLAG_HOOK = 0x80,     // Hot shelter hook
	LUREFLAG_GET = 0x800,     // Fish caught
	LUREFLAG_1000 = 0x1000,
	LUREFLAG_PTCL = 0x2000,   // Particule spawned
	LUREFLAG_4000 = 0x4000,
	LUREFLAG_LAUNCH = 0x8000, // Enable fishing
};

BIGETC* GetBigEtc(int pnum);
void InitFishing();
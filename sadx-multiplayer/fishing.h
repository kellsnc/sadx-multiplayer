#pragma once

struct BIGETC // struct doesn't actually exists, but all of the global variables are adjacent
{
	int Sakanu_Num;
	int Big_Fish_Flag;
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

BIGETC* GetBigEtc(int pnum);
void InitFishing();
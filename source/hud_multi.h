#pragma once

enum MHudTex : __int16
{
	MHudTex_Score,
	MHudTex_Time,
	MHudTex_Ring,
	MHudTex_Arrow,
	MHudTex_CPU1,
	MHudTex_CPU2,
	MHudTex_P,
	MHudTex_P1,
	MHudTex_P2,
	MHudTex_P3,
	MHudTex_P4,
	MHudTex_Cream,
	MHudTex_Cheese,
	MHudTex_Alphabet
};

extern NJS_TEXLIST CON_MULTI_TEXLIST;

extern NJS_SPRITE MULTIHUD_SPRITE;
extern NJS_SPRITE MULTIHUDDIGIT_SPRITE;

void MultiHudScore(int num);
void MultiHudTime(__int8 minutes, __int8 seconds, __int8 frames);
void MultiHudRings(int num);
void MultiHudLives(int num);

void DrawWaitScreen(int num);
void DisplayMultiHud(int num);
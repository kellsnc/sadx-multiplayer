#pragma once

extern NJS_TEXLIST CON_MULTI_TEXLIST;

extern NJS_SPRITE MULTIHUD_SPRITE;
extern NJS_SPRITE MULTIHUDDIGIT_SPRITE;

void MultiHudScore(int num);
void MultiHudTime(int num);
void MultiHudRings(int num);
void MultiHudLives(int num);

void DrawWaitScreen(int num);
void DisplayMultiHud(int num);
void MultiHudInit();
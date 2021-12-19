#pragma once

extern unsigned int numScreen;

struct ScreenRatio
{
    float x, y, w, h;
};

ScreenRatio* GetScreenRatio(int num);
void __cdecl DisplayTask_r();
void InitSplitScreen();
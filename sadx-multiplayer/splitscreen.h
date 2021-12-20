#pragma once

extern unsigned int numScreen;
extern signed int numViewPort;

struct ScreenRatio
{
    float x, y, w, h;
};

const ScreenRatio* GetScreenRatio(int num);
bool ChangeViewPort(int num);
void ResetViewPort();
void __cdecl DisplayTask_r();
void InitSplitScreen();
#pragma once

void DrawSADXText(const char* text, __int16 y);
int MenuSelectButtonsPressedM(int pnum);
int MenuBackButtonsPressedM(int pnum);
short tolevelnum(short num);
short toactnum(short num);
void ToggleControllers(bool enabled);
void TeleportPlayersInitialPosition(int pNum);
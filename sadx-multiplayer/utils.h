#pragma once

void DrawSADXText(const char* text, __int16 y);
int MenuSelectButtonsPressedM(int pnum);
int MenuBackButtonsPressedM(int pnum);
short tolevelnum(short num);
short toactnum(short num);
void ToggleControllers(bool enabled);
void TeleportPlayersInitialPosition(int pNum);
void SetAllPlayersInitialPosition();
float GetDistance(NJS_VECTOR* v1, NJS_VECTOR* v2);
bool IsPlayerInSphere(NJS_POINT3* p, float r);
bool IsPlayerInSphere(float x, float y, float z, float r);
bool IsCameraInSphere(NJS_POINT3* p, float r);
bool IsCameraInSphere(float x, float y, float z, float r);
#pragma once

#define MAKEVARMULTI(type, name, addr) \
	static type name##P2{}; \
	static type name##P3{}; \
	static type name##P4{}; \
	static type* const name##_m[PLAYER_MAX] = { (type*)addr, &##name##P2, &##name##P3, &##name##P4 };

void DrawSADXText(const char* text, __int16 y);
int MenuSelectButtonsPressedM(int pnum);
int MenuBackButtonsPressedM(int pnum);
short tolevelnum(short num);
short toactnum(short num);
void SetAllPlayersInitialPosition();
void SetAllPlayersPosition(float x, float y, float z, Angle angy);
float GetDistance(NJS_VECTOR* v1, NJS_VECTOR* v2);
int GetClosestPlayerNum(NJS_POINT3* pos);
int GetClosestPlayerNum(float x, float y, float z);
int IsPlayerInSphere(NJS_POINT3* p, float r);
int IsPlayerInSphere(float x, float y, float z, float r);
bool IsCameraInSphere(NJS_POINT3* p, float r);
bool IsCameraInSphere(float x, float y, float z, float r);
int IsPlayerOnDyncol(task* tp);
void njRotateX_(Angle ang);
void njRotateY_(Angle ang);
void njRotateZ_(Angle ang);
void ChangeActM(int amount);
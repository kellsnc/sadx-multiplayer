#pragma once

#define BYTEn(x, n)   (*((uint8_t*)&(x)+n))

float GetDistance(NJS_VECTOR* v1, NJS_VECTOR* v2);
float GetDistance2(NJS_VECTOR* v1, NJS_VECTOR* v2);
int GetClosestPlayerNum(NJS_POINT3* pos);
int GetClosestPlayerNum(float x, float y, float z);
int GetClosestPlayerNumRange(NJS_POINT3* pos, float range);
int GetClosestPlayerNumRange(float x, float y, float z, float range);
int IsPlayerInSphere(NJS_POINT3* p, float r);
int IsPlayerInSphere(float x, float y, float z, float r);
bool IsCameraInSphere(NJS_POINT3* p, float r);
bool IsCameraInSphere(float x, float y, float z, float r);
int IsPlayerOnDyncol(task* tp);
void ChangeActM(int amount);
bool IsInAdventureField();
bool IsPlayerInCart(char pnum);
bool IsPlayerOnSnowboard(char pnum);
int GetHoldingPlayerId(task* htp);
int GetBossTargetPlayerRandom();
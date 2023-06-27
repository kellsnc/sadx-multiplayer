#pragma once

short tolevelnum(short num);
short toactnum(short num);
float GetDistance(NJS_VECTOR* v1, NJS_VECTOR* v2);
int GetClosestPlayerNum(NJS_POINT3* pos);
int GetClosestPlayerNum(float x, float y, float z);
int GetClosestPlayerNumRange(NJS_POINT3* pos, float range);
int GetClosestPlayerNumRange(float x, float y, float z, float range);
int IsPlayerInSphere(NJS_POINT3* p, float r);
int IsPlayerInSphere(float x, float y, float z, float r);
bool IsCameraInSphere(NJS_POINT3* p, float r);
bool IsCameraInSphere(float x, float y, float z, float r);
int IsPlayerOnDyncol(task* tp);
void njRotateX_(Angle ang);
void njRotateY_(Angle ang);
void njRotateZ_(Angle ang);
void ChangeActM(int amount);
void __cdecl SetAndDisp(task* obj, TaskFuncPtr disp);
bool isInHubWorld();
bool isPlayerInCart(char pnum);
bool isPlayerOnSnowBoard(char pnum);
bool isOnePlayerSpecifiedChar(char charID);
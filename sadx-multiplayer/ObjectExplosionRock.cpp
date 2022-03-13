#include "pch.h"
#include <UsercallFunctionHandler.h>


struct steamtbl
{
    NJS_POINT3 pos;
    unsigned __int16 init_time;
    unsigned __int16 wtimer;
};

auto GenerateSteam = GenerateUsercallWrapper<void (*)(int num, steamtbl* tbl)>(noret, 0x601A10, rEAX, rECX);
FunctionPointer(void, SetParabolicMotionP, (int playerNum, float a2, NJS_VECTOR* a3), 0x446D90);
DataPointer(float, flt_24987AC, 0x24987AC);
FunctionPointer(void, BlowPlayer3, (float x, float y, float z), 0x601DC0);

FunctionPointer(void, BlowPlayer2, (float x, float y, float z), 0x601C80);
DataPointer(float, flt_24987A8, 0x24987A8);

static void __cdecl BlowPlayer3_r(float x, float y, float z);
Trampoline BlowPlayer3_t(0x601DC0, 0x601DC8, BlowPlayer3_r);
void __cdecl BlowPlayer3_r(float x, float y, float z)
{

    if (!multiplayer::IsActive())
    {
        return TARGET_STATIC(BlowPlayer3)(x, y, z);
    }

    double v3; 
    float result; 
    float v5;
    float pPosX; 
    float pPosZ; 
    float PGrav;  
    float pPosY; 
    NJS_VECTOR v; 
    NJS_VECTOR pos; 

    for (uint8_t i = 0; i < PLAYER_MAX; i++) {

        if (!EntityData1Ptrs[i])
            continue;

        pPosX = EntityData1Ptrs[i]->Position.x;
        pPosY = EntityData1Ptrs[i]->Position.y;
        pPosZ = EntityData1Ptrs[i]->Position.z;
        PGrav = GetCharObj2(i)->PhysicsData.Gravity;
        v5 = (pPosZ - z) * (pPosZ - z) + (pPosX - x) * (pPosX - x);
        v3 = squareroot(v5) / flt_24987AC;
        v.x = (x - pPosX) * (1.0 / v3);
        v.z = (z - pPosZ) * (1.0 / v3);
        v.y = (v3 * v3 * PGrav * 0.5 + y - pPosY) * (1.0 / v3);
        pos = v;
        njUnitVector(&pos);
        result = njScalor(&v);
        SetParabolicMotionP(i, result, &pos);
    }
}

static void __cdecl BlowPlayer2_r(float x, float y, float z);
Trampoline BlowPlayer2_t(0x601C80, 0x601C88, BlowPlayer2_r);
void __cdecl BlowPlayer2_r(float x, float y, float z)
{
    if (!multiplayer::IsActive())
    {
        return TARGET_STATIC(BlowPlayer2)(x, y, z);
    }

    double v3;  
    float v4; 
    float v5; 
    float pPosY; 
    float pGrav; 
    float pPosX; 
    float pPosZ; 
    float pSpdCap;
    NJS_VECTOR v; 
    NJS_VECTOR pos;

    for (uint8_t i = 0; i < PLAYER_MAX; i++) {

        if (!EntityData1Ptrs[i])
            continue;

        pPosX = EntityData1Ptrs[0]->Position.x;
        pPosY = EntityData1Ptrs[0]->Position.y;
        pPosZ = EntityData1Ptrs[0]->Position.z;
        pGrav = GetCharObj2(0)->PhysicsData.Gravity;
        pSpdCap = GetCharObj2(0)->PhysicsData.VSpeedCap;
        v5 = (pPosZ - z) * (pPosZ - z) + (pPosX - x) * (pPosX - x);
        v3 = squareroot(v5) / flt_24987A8;
        v.x = (x - pPosX) * (1.0 / v3);
        v.z = (z - pPosZ) * (1.0 / v3);
        if (pPosY <= (double)y)
        {
            v.y = (v3 * v3 * pGrav * 0.5 + y - pPosY) * (1.0 / v3);
        }
        else
        {
            v.y = ((v3 - pSpdCap / pGrav) * pSpdCap + pPosY - pPosY + pSpdCap / pGrav * (pSpdCap / pGrav) * pGrav * 0.5)
                / (pSpdCap
                    / pGrav);
        }
        pos = v;
        njUnitVector(&pos);
        v4 = njScalor(&v);
        SetParabolicMotionP(i, v4, &pos);
    }
}


#pragma once

#include "VariableHook.hpp"

extern VariableHook<NJS_POINT3, 0x3C4ABFC> CamPathCam2Core_Pos_m;
extern VariableHook<Angle3, 0x3C4ACDC> CamPathCam2Core_Angle_m;
extern VariableHook<Bool, 0x3C4AC98> CamPathCam2Core_AliveFlag_m;

CAM_ANYPARAM* GetCamAnyParam(int pnum);
void SetRuinWaka1Data(NJS_POINT3* pos, NJS_POINT3* tgt, int pnum);

void __cdecl CameraKlamath_m(_OBJ_CAMERAPARAM* pParam);
void __cdecl CameraCart_m(_OBJ_CAMERAPARAM* pParam);
void __cdecl CameraRuinWaka1_m(_OBJ_CAMERAPARAM* pParam);
void __cdecl PathCamera1_m(_OBJ_CAMERAPARAM* pParam);
void __cdecl PathCamera2Core_m(_OBJ_CAMERAPARAM* pParam);

void __cdecl AdjustNormal_m(taskwk* pTaskWork, taskwk* pOldTaskWork, _OBJ_ADJUSTPARAM* pCameraAdjustWork);
void __cdecl AdjustForFreeCamera_m(taskwk*, taskwk*, _OBJ_ADJUSTPARAM*);
void __cdecl AdjustThreePoint_m(taskwk*, taskwk*, _OBJ_ADJUSTPARAM*);

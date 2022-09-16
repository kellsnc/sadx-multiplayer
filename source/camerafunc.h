#pragma once

CAM_ANYPARAM* GetCamAnyParam(int pnum);

void __cdecl CameraKlamath_m(_OBJ_CAMERAPARAM* pParam);
void __cdecl CameraCart_m(_OBJ_CAMERAPARAM* pParam);
void __cdecl CameraRuinWaka1_m(_OBJ_CAMERAPARAM* pParam);

void __cdecl AdjustNormal_m(taskwk* pTaskWork, taskwk* pOldTaskWork, _OBJ_ADJUSTPARAM* pCameraAdjustWork);
void __cdecl AdjustForFreeCamera_m(taskwk*, taskwk*, _OBJ_ADJUSTPARAM*);
void __cdecl AdjustThreePoint_m(taskwk*, taskwk*, _OBJ_ADJUSTPARAM*);

#pragma once

CAM_ANYPARAM* GetCamAnyParam(int pnum);

void CameraKlamath_m(_OBJ_CAMERAPARAM* pParam);
void CameraRuinWaka1_m(_OBJ_CAMERAPARAM* pParam);

void AdjustNormal_m(taskwk* pTaskWork, taskwk* pOldTaskWork, _OBJ_ADJUSTPARAM* pCameraAdjustWork);
void AdjustForFreeCamera_m(taskwk*, taskwk*, _OBJ_ADJUSTPARAM*);
void AdjustThreePoint_m(taskwk*, taskwk*, _OBJ_ADJUSTPARAM*);

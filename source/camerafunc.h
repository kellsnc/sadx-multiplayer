#pragma once

#include "VariableHook.hpp"

extern VariableHook<NJS_POINT3, 0x3C4ABFC> CamPathCam2Core_Pos_m;
extern VariableHook<Angle3, 0x3C4ACDC> CamPathCam2Core_Angle_m;
extern VariableHook<Bool, 0x3C4AC98> CamPathCam2Core_AliveFlag_m;

CAM_ANYPARAM* GetCamAnyParam(int pnum);
void SetRuinWaka1Data(NJS_POINT3* pos, NJS_POINT3* tgt, int pnum);
void SetLocalPathCamera_m(pathtag* ptp, Sint32 mode, Sint32 timer, int pnum);

void PatchCameraFuncs();

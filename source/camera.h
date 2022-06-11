#pragma once

NJS_POINT3* GetCameraPosition(int pnum);
Angle3* GetCameraAngle(int pnum);
void SetCameraPosition(int pnum, float x, float y, float z);
void SetCameraAngle(int pnum, Angle x, Angle y, Angle z);
Angle ds_GetPerspectiveM(int pnum);
void njSetPerspectiveM(int pnum, Angle bams);
void ResetPerspectiveM(int pnum);
void ApplyMultiCamera(int pnum);
void __cdecl Camera_r(task* tp);
void __cdecl CameraPause_r(task* tp);
void InitCamera();
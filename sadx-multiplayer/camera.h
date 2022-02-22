#pragma once

NJS_VECTOR* GetCameraPosition(int pnum);
Angle3* GetCameraAngle(int pnum);
Angle ds_GetPerspectiveM(int pnum);
void njSetPerspectiveM(int pnum, Angle bams);
void ApplyMultiCamera(int pnum);
void __cdecl Camera_r(task* tp);
void __cdecl CameraPause_r(task* tp);
void InitCamera();
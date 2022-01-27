#pragma once

NJS_VECTOR* GetCameraPosition(int pnum);
Angle3* GetCameraAngle(int pnum);
void MultiCam_InitialPosition(int num);
void ApplyMultiCamera(int pnum);
void __cdecl Camera_r(task* tp);
void __cdecl CameraPause_r(task* tp);
void InitCamera();
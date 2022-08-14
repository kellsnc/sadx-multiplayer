#pragma once

NJS_POINT3* GetCameraPosition(int pnum);
Angle3* GetCameraAngle(int pnum);
static void SetCameraPosition(int pnum, float x, float y, float z) {};
static void SetCameraAngle(int pnum, Angle x, Angle y, Angle z) {};

void SetFreeCamera_m(int pnum, Sint32 sw);
Bool GetFreeCamera_m(int pnum);

void SetAdjustMode_m(int pnum, Sint32 AdjustType);
Sint32 GetAdjustMode_m(int pnum);
Sint32 GetCameraMode_m(int pnum);
void SetCameraMode_m(int pnum, Sint32 mode);

static Angle ds_GetPerspectiveM(int pnum) { return 0; };
static void njSetPerspectiveM(int pnum, Angle bams) {};
static void ResetPerspectiveM(int pnum) {};

void ApplyMultiCamera(int pnum);
void __cdecl Camera_r(task* tp);
void __cdecl CameraPause_r(task* tp);
void InitCamera();

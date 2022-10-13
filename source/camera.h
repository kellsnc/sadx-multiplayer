#pragma once

#include "VariableHook.hpp"
#include "camerafunc.h"

extern VariableHook<Bool, 0x3B2C6C0> flagCameraNoUnderWater_m;

_OBJ_CAMERAPARAM* GetExternCameraParam(int pnum);

NJS_POINT3* GetCameraPosition(int pnum);
Angle3* GetCameraAngle(int pnum);
static void SetCameraPosition(int pnum, float x, float y, float z) {};
static void SetCameraAngle(int pnum, Angle x, Angle y, Angle z) {};

void SetFreeCamera_m(int pnum, Sint32 sw);
Bool GetFreeCamera_m(int pnum);
void SetFreeCameraMode_m(int pnum, Sint32 sw);
Bool GetFreeCameraMode_m(int pnum);

void SetAdjustMode_m(int pnum, Sint32 AdjustType);
Sint32 GetAdjustMode_m(int pnum);
Sint32 GetCameraMode_m(int pnum);
void SetCameraMode_m(int pnum, Sint32 mode);

void CameraSetNormalCamera_m(int pnum, Sint16 ssCameraMode, Uint8 ucAdjustType);
void CameraReleaseCollisionCamera_m(int pnum);
void CameraSetCollisionCamera(int pnum, Sint16 ssCameraMode, Uint8 ucAdjustType);
void CameraSetCollisionCameraFunc_m(int pnum, void(__cdecl* fnCamera)(_OBJ_CAMERAPARAM*), Uint8 ucAdjustType, Sint8 scCameraDirect);
void CameraReleaseEventCamera_m(int pnum);
void CameraSetEventCamera_m(int pnum, Sint16 ssCameraMode, Uint8 ucAdjustType);
void CameraSetEventCameraFunc_m(int pnum, void(__cdecl* fnCamera)(_OBJ_CAMERAPARAM*), Uint8 ucAdjustType, Sint8 scCameraDirect);
Bool CameraCameraAction(int pnum, NJS_CACTION* caction, Float frame);

void ResetCameraTimer_m(int pnum);
Bool IsEventCamera_m(int pnum);
Bool IsCompulsionCamera_m(int pnum);

Angle ds_GetPerspective_m(int pnum);
void njSetPerspective_m(int pnum, Angle ang);
void ResetPerspective_m(int pnum);

void ApplyMultiCamera(int pnum);
void __cdecl Camera_r(task* tp);
void __cdecl CameraPause_r(task* tp);
void InitCamera();

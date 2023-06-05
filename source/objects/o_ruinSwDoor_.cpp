#include "pch.h"

static void DisableController_r(uint8_t index)
{
	if (multiplayer::IsActive())
	{
		return;
	}

	return DisableController(index);
}

static void SetFreeCameraMode_r(Bool a1)
{
	if (multiplayer::IsActive())
	{
		return;
	}

	return SetFreeCameraMode(a1);
}

static void CameraSetEventCamera_r(__int16 a1, char a2)
{
	if (multiplayer::IsActive())
	{
		return;
	}

	return CameraSetEventCamera(a1, a2);
}

void init_SwDoorPatch()
{
	WriteCall((void*)0x5E7E3C, DisableController_r);
	WriteCall((void*)0x5E7E22, SetFreeCameraMode_r);	
	WriteCall((void*)0x5E7E2B, CameraSetEventCamera_r);
}
#include "pch.h"
#include "splitscreen.h"
#include "camera.h"

Trampoline* CameraPause_t = nullptr;
Trampoline* CameraDisplay_t = nullptr;
Trampoline* Camera_t = nullptr;

NJS_VECTOR MultiCamPos[PLAYER_MAX];
Angle3 MultiCamAng[PLAYER_MAX];

NJS_VECTOR* GetCameraPosition(int pnum)
{
	if (IsMultiplayerEnabled() && pnum < player_count && playertp[pnum])
	{
		return &MultiCamPos[pnum];
	}
	else
	{
		return &camera_twp->pos;
	}
}

Angle3* GetCameraAngle(int pnum)
{
	if (IsMultiplayerEnabled() && pnum < player_count && playertp[pnum])
	{
		return &MultiCamAng[pnum];
	}
	else
	{
		return &camera_twp->ang;
	}
}

void ApplyMultiCamera(taskwk* twp, int pnum)
{
	if (!twp || !playertwp[pnum])
	{
		return;
	}

	twp->pos = MultiCamPos[pnum];
	twp->ang = MultiCamAng[pnum];

	Camera_Display_((EntityData1*)twp);
}

void MultiCamera(int pnum)
{
	if (!playertwp[pnum])
	{
		return;
	}

	auto pltwp = playertwp[pnum];
	auto& pos = MultiCamPos[pnum];
	auto& ang = MultiCamAng[pnum];
	
	pos = { -50, 10.0f, 0.0f };
	PConvertVector_P2G(pltwp, &pos);
	njAddVector(&pos, &pltwp->pos);

	NJS_VECTOR playerpos_unit;
	playerpos_unit.x = pos.x - pltwp->pos.x;
	playerpos_unit.y = pos.y - pltwp->pos.y - 10.5f;
	playerpos_unit.z = pos.z - pltwp->pos.z;
	njUnitVector(&playerpos_unit);
	ang.y = (atan2f(playerpos_unit.x, playerpos_unit.z) * 65536.0 * 0.1591549762031479);
	ang.x = (asinf(playerpos_unit.y) * 65536.0 * -0.1591549762031479);
}

void __cdecl CameraDisplay_r(task* tp)
{
	if (IsMultiplayerEnabled())
	{
		ApplyMultiCamera(tp->twp, SplitScreen::numScreen);
	}
	else
	{
		TARGET_DYNAMIC(CameraDisplay)(tp);
	}
}

void __cdecl CameraPause_r(task* tp)
{
	if (IsMultiplayerEnabled())
	{
		ApplyMultiCamera(tp->twp, SplitScreen::numScreen);
	}
	else
	{
		TARGET_DYNAMIC(CameraPause)(tp);
	}
}

void __cdecl Camera_r(task* tp)
{
	camera_twp = tp->twp;
	
	// If multiplayer is enabled, run custom cameras
	if (IsMultiplayerEnabled())
	{
		cameraready = TRUE;

		for (int i = 0; i < player_count; ++i)
		{
			MultiCamera(i);
		}

		// in exec, apply camera for first player
		ApplyMultiCamera(tp->twp, 0);
	}
	else
	{
		TARGET_DYNAMIC(Camera)(tp);
	}
}

void InitCamera()
{
	Camera_t = new Trampoline(0x438090, 0x438097, Camera_r);
	CameraPause_t = new Trampoline(0x4373D0, 0x4373D7, CameraPause_r);
	CameraDisplay_t = new Trampoline(0x4370F0, 0x4370F5, CameraDisplay_r);
}
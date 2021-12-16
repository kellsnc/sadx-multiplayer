#include "pch.h"
#include <math.h>
#include <SADXModLoader.h>
#include <Trampoline.h>
#include "mod.h"
#include "camera.h"
#include "splitscreen.h"

TaskFunc(Camera, 0x438090);

Trampoline* CameraPause_t = nullptr;
Trampoline* CameraDisplay_t = nullptr;
Trampoline* Camera_t = nullptr;

void MultiCamera(taskwk* twp, int pnum)
{
	if (!playertwp[pnum])
	{
		return;
	}

	auto pltwp = playertwp[pnum];

	twp->pos = { -50, 10.0f, 0.0f };
	PConvertVector_P2G(pltwp, &twp->pos);
	njAddVector(&twp->pos, &pltwp->pos);

	NJS_VECTOR playerpos_unit;
	playerpos_unit.x = twp->pos.x - pltwp->pos.x;
	playerpos_unit.y = twp->pos.y - pltwp->pos.y - 10.5f;
	playerpos_unit.z = twp->pos.z - pltwp->pos.z;
	njUnitVector(&playerpos_unit);
	twp->ang.y = (atan2f(playerpos_unit.x, playerpos_unit.z) * 65536.0 * 0.1591549762031479);
	twp->ang.x = (asinf(playerpos_unit.y) * 65536.0 * -0.1591549762031479);

	Camera_Display_((EntityData1*)twp);
}

void __cdecl CameraDisplay_r(task* tp)
{
	if (player_count == 0)
	{
		TARGET_DYNAMIC(CameraDisplay)(tp);
	}
	else
	{
		MultiCamera(tp->twp, current_screen);
	}
}

void __cdecl CameraPause_r(task* tp)
{
	if (player_count == 0)
	{
		TARGET_DYNAMIC(CameraPause)(tp);
	}
	else
	{
		MultiCamera(tp->twp, current_screen);
	}
}

void __cdecl Camera_r(task* tp)
{
	Camera_Data1 = (EntityData1*)tp->twp;
	
	if (player_count == 0)
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
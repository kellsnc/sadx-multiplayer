#include "pch.h"
#include "camerafunc.h"
#include <camera.h>

DataPointer(int, PinballCameraMode, 0x3C748F7);

DataArray(Float[9], PinballCameraTable, 0x1E78F18, 6);

// Pinball in Lost World

#define PINBALL_PNUM(twp) twp->smode

static void __cdecl CameraPinball_r(task* tp);
FastFunctionHookPtr<decltype(&CameraPinball_r)> CameraPinball_h(0x5DCEC0);

static void Normal_m(task* tp)
{
	taskwk* twp = tp->twp;
	auto pnum = PINBALL_PNUM(twp);

	if ((Uint8)twp->btimer == PinballCameraMode)
	{
		if (PinballCameraMode & 0x80)
		{			
			NJS_POINT3 cam_pos, cam_tgt;
			if (GetRuinWaka1Data(&cam_pos, &cam_tgt, pnum))
			{
				Sint32 count = 0;
				Float camCompare = (cam_pos.x - twp->scl.x) * 0.125f;
				if (camCompare == cam_pos.x)
				{
					cam_pos.x = twp->scl.x;
					++count;
				}
				else
				{
					cam_pos.x -= camCompare;
				}
				camCompare = (cam_pos.y - twp->scl.y) * 0.125f;
				if (camCompare == cam_pos.x)
				{
					cam_pos.y = twp->scl.y;
					++count;
				}
				else
				{
					cam_pos.y -= camCompare;
				}
				camCompare = (cam_pos.z - twp->scl.z) * 0.125f;
				if (camCompare == cam_pos.x)
				{
					cam_pos.z = twp->scl.z;
					++count;
				}
				else
				{
					cam_pos.z -= camCompare;
				}
				camCompare = (cam_tgt.x - twp->counter.f) * 0.125f;
				if (camCompare == cam_pos.x)
				{
					cam_tgt.x = twp->counter.f;
					++count;
				}
				else
				{
					cam_tgt.x -= camCompare;
				}
				camCompare = (cam_tgt.y - twp->timer.f) * 0.125f;
				if (camCompare == cam_pos.x)
				{
					cam_tgt.y = twp->timer.f;
					++count;
				}
				else
				{
					cam_tgt.y -= camCompare;
				}
				camCompare = (cam_tgt.z - twp->value.f) * 0.125f;
				if (camCompare == cam_pos.x)
				{
					cam_tgt.z = twp->value.f;
					++count;
				}
				else
				{
					cam_tgt.z -= camCompare;
				}
				if (count == 6)
				{
					PinballCameraMode &= ~0x80;
					twp->btimer = PinballCameraMode;
				}
			}
			SetRuinWaka1Data(&cam_pos, &cam_tgt, pnum);
		}	
	}
	else
	{
		twp->btimer = PinballCameraMode;
		Uint8 camID = PinballCameraMode & 0x7F;
		twp->scl.x = PinballCameraTable[camID][0];
		twp->scl.y = PinballCameraTable[camID][1];
		twp->scl.z = PinballCameraTable[camID][2];
		twp->counter.f = PinballCameraTable[camID][3];
		twp->timer.f = PinballCameraTable[camID][4];
		twp->value.f = PinballCameraTable[camID][5];
	}
	LoopTaskC(tp);
}

static void __cdecl CameraPinball_r(task* tp)
{
	CameraSetEventCamera_m(0, CAMMD_RuinWaka1, 0);
	auto twp = tp->twp;
	NJS_POINT3 cam_pos, cam_tgt;
	cam_pos = twp->scl;
	cam_tgt.x = twp->counter.f;
	cam_tgt.y = twp->timer.f;
	cam_tgt.z = twp->value.f;
	SetRuinWaka1Data(&cam_pos, &cam_tgt, 0);
	tp->exec = Normal_m;

	for (uint8_t i = 1; i < multiplayer::GetPlayerCount(); ++i)
	{
		CameraSetEventCamera_m(i, CAMMD_RuinWaka1, 0);
		cam_pos = twp->scl;
		cam_tgt.x = twp->counter.f;
		cam_tgt.y = twp->timer.f;
		cam_tgt.z = twp->value.f;
		SetRuinWaka1Data(&cam_pos, &cam_tgt, i);
		auto ctp = CreateElementalTask(2u, LEV_4, Normal_m);
		*ctp->twp = *tp->twp;
		PINBALL_PNUM(ctp->twp) = i;
	}
}

static void SetPinballMode(uint8_t pnum, uint8_t mode)
{
	SetInputP(pnum, mode);

	for (uint8_t i = 1; i < multiplayer::GetPlayerCount(); i++)
	{
		SetInputP(i, mode);
	}

}

void patch_pinball_init()
{
	CameraPinball_h.Hook(CameraPinball_r);
	WriteCall((void*)0x5C0941, SetPinballMode);
}

RegisterPatch patch_pinbal(patch_pinball_init);

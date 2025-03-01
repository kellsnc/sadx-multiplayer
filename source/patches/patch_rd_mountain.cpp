#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "multiplayer.h"

FastFunctionHook<void, task*> Rd_Mountain_h(0x601550);

// Patch act swaps
void __cdecl Rd_Mountain_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		if (ssActNumber == 0)
		{
			if (IsPlayerInSphere(-3667.0f, -400.0f, -2319.0f, 400.0f))
			{
				ChangeActM(1);
				dsEditLightInit();
				FreeTaskC(tp);
				RdMountainInit(tp);
				return;
			}
		}
		else if (ssActNumber == 1 && CurrentCharacter != Characters_Gamma)
		{
			// Reset lava at the beginning
			if (rd_mountain_twp && rd_mountain_twp->scl.x != 130.0f && IsPlayerInSphere(-380.0f, 440.0f, 1446.0f, 200.0f))
			{
				rd_mountain_twp->scl.x = 130.0f;
				rd_mountain_twp->scl.y = -0.5f;
			}
		}

		//patch an issue where the original function was taking priority for act swap
		if (tp->twp->mode != 1)
			Rd_Mountain_h.Original(tp);
		else
			LoopTaskC(tp);
	}
	else
	{
		Rd_Mountain_h.Original(tp);
	}
}

// Load cloud task into slot 1 instead of 0 (to not run before the camera)
static void __cdecl Create_Mountain_Cloud()
{
	CreateElementalTask(IM_TASKWK, LEV_1, (TaskFuncPtr)0x601230);
}

void patch_rd_mountain_init()
{
	Rd_Mountain_h.Hook(Rd_Mountain_r);

	// Red Mountain cloud layer
	WriteCall((void*)0x60147B, Create_Mountain_Cloud);
	WriteCall((void*)0x601404, Create_Mountain_Cloud);
}

RegisterPatch patch_rd_mountain(patch_rd_mountain_init);
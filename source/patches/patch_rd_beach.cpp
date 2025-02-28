#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "multiplayer.h"
#include "camera.h"

FastFunctionHook<void, task*> Rd_Beach_t(0x4F6D60);

static void __cdecl Rd_Beach_r(task* tp)
{
	if (ssActNumber == 0 && multiplayer::IsActive())
	{
		if (IsPlayerInSphere(5746.0f, 406.0f, 655.0f, 22.0f))
		{
			tp->twp->mode = 0;
			ChangeActM(1);

			DataPointer(task*, BeachWaveTp, 0x3C5E3D0);

			if (BeachWaveTp)
			{
				FreeTask(BeachWaveTp);
				BeachWaveTp = nullptr;
			}

			return;
		}
	}

	Rd_Beach_t.Original(tp);
}

void patch_rd_beach_init()
{
	Rd_Beach_t.Hook(Rd_Beach_r);
	WriteData((void**)0x4F723E, (void*)0x4F71A0); // Patch skybox mode
}

RegisterPatch patch_rd_beach(patch_rd_beach_init);
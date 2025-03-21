#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"

// Fix Final Egg camera paths

static void __cdecl ObjectCamPathCam_FinalEgg_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectCamPathCam_FinalEgg_Exec_r)> ObjectCamPathCam_FinalEgg_Exec_h(0x5B1860);

static void __cdecl ObjectCamPathCam_FinalEgg_Exec_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		// Use the general path camera instead until I find the courage to rewrite this
		tp->twp->smode = 1;
		tp->dest = nullptr;
		InitPathworkCamera(tp);
	}
	else
	{
		ObjectCamPathCam_FinalEgg_Exec_h.Original(tp);
	}
}

void patch_finalegg_pathcam_init()
{
	ObjectCamPathCam_FinalEgg_Exec_h.Hook(ObjectCamPathCam_FinalEgg_Exec_r);
}

RegisterPatch patch_finalegg_pathcam(patch_finalegg_pathcam_init);
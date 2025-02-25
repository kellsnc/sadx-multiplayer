#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"

// Fix Final Egg camera paths

static void __cdecl ObjectCamPathCam_FinalEgg_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectCamPathCam_FinalEgg_Exec_r)> ObjectCamPathCam_FinalEgg_Exec_t(0x5B1860, ObjectCamPathCam_FinalEgg_Exec_r);
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
		ObjectCamPathCam_FinalEgg_Exec_t.Original(tp);
	}
}
#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix Final Egg camera paths

static void __cdecl ObjectCamPathCam_FinalEgg_Exec_r(task* tp);
Trampoline ObjectCamPathCam_FinalEgg_Exec_t(0x5B1860, 0x5B1867, ObjectCamPathCam_FinalEgg_Exec_r);
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
		TARGET_STATIC(ObjectCamPathCam_FinalEgg_Exec)(tp);
	}
}

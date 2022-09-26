#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix Casinopolis camera paths

static void __cdecl ObjectCamPathCam_Casino_Exec_r(task* tp);
Trampoline ObjectCamPathCam_Casino_Exec_t(0x5C3040, 0x5C304A, ObjectCamPathCam_Casino_Exec_r);
static void __cdecl ObjectCamPathCam_Casino_Exec_r(task* tp)
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
		TARGET_STATIC(ObjectCamPathCam_Casino_Exec)(tp);
	}
}

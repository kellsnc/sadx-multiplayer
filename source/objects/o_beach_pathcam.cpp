#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix Emerald Coast camera paths

static void __cdecl ObjectCamPathCam_Beach_Exec_r(task* tp);
Trampoline ObjectCamPathCam_Beach_Exec_t(0x4F9A90, 0x4F9A9A, ObjectCamPathCam_Beach_Exec_r);
static void __cdecl ObjectCamPathCam_Beach_Exec_r(task* tp)
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
		TARGET_STATIC(ObjectCamPathCam_Beach_Exec)(tp);
	}
}

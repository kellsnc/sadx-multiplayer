#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"
#include "camera.h"
#include "camerafunc.h"

// Fix Lost World camera paths

static void __cdecl ObjectCamPathCam_Ruin_Exec_r(task* tp);
Trampoline ObjectCamPathCam_Ruin_Exec_t(0x5E5A90, 0x5E5A9A, ObjectCamPathCam_Ruin_Exec_r);
static void __cdecl ObjectCamPathCam_Ruin_Exec_r(task* tp)
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
		TARGET_STATIC(ObjectCamPathCam_Ruin_Exec)(tp);
	}
}
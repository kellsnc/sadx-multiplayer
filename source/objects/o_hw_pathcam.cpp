#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix Speed Highway camera paths

static void __cdecl ObjectCamPathCam_Highway_Exec_r(task* tp);
Trampoline ObjectCamPathCam_Highway_Exec_t(0x613100, 0x61310A, ObjectCamPathCam_Highway_Exec_r);
static void __cdecl ObjectCamPathCam_Highway_Exec_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		// Use the general path camera instead until I find the courage to rewrite this
		tp->twp->smode = 1;
		InitPathworkCamera(tp);
	}
	else
	{
		TARGET_STATIC(ObjectCamPathCam_Highway_Exec)(tp);
	}
}

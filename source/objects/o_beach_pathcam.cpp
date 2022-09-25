#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix camera paths
// TODO: find the courage to do this

static void __cdecl SetupCamPathCam_Beach_r(task* tp);
Trampoline SetupCamPathCam_Beach_t(0x4F9CF0, 0x4F9CF6, SetupCamPathCam_Beach_r);
static void __cdecl SetupCamPathCam_Beach_r(task* tp)
{
	if (multiplayer::IsActive())
	{

	}
	else
	{
		TARGET_STATIC(SetupCamPathCam_Beach)(tp);
	}
}

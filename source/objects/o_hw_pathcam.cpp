#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix camera paths
// TODO: find the courage to do this

static void __cdecl SetupCamPathCam_Highway_r(task* tp);
Trampoline SetupCamPathCam_Highway_t(0x613370, 0x613377, SetupCamPathCam_Highway_r);
static void __cdecl SetupCamPathCam_Highway_r(task* tp)
{
	if (multiplayer::IsActive())
	{

	}
	else
	{
		TARGET_STATIC(SetupCamPathCam_Highway)(tp);
	}
}

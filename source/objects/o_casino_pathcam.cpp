#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix camera paths
// TODO: find the courage to do this

static void __cdecl SetupCamPathCam_Casino_r(task* tp);
Trampoline SetupCamPathCam_Casino_t(0x5C32D0, 0x5C32D6, SetupCamPathCam_Casino_r);
static void __cdecl SetupCamPathCam_Casino_r(task* tp)
{
	if (multiplayer::IsActive())
	{

	}
	else
	{
		TARGET_STATIC(SetupCamPathCam_Casino)(tp);
	}
}

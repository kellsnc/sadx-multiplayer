#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix camera paths
// TODO: find the courage to do this

static void __cdecl SetupCamPathCam_FinalEgg_r(task* tp);
Trampoline SetupCamPathCam_FinalEgg_t(0x5B1960, 0x5B1965, SetupCamPathCam_FinalEgg_r);
static void __cdecl SetupCamPathCam_FinalEgg_r(task* tp)
{
	if (multiplayer::IsActive())
	{

	}
	else
	{
		TARGET_STATIC(SetupCamPathCam_FinalEgg)(tp);
	}
}

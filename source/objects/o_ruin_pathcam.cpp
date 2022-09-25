#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix camera paths
// TODO: find the courage to do this

static void __cdecl SetupCamPathCam_Ruin_r(task* tp);
Trampoline SetupCamPathCam_Ruin_t(0x5E5D10, 0x5E5D16, SetupCamPathCam_Ruin_r);
static void __cdecl SetupCamPathCam_Ruin_r(task* tp)
{
	if (multiplayer::IsActive())
	{

	}
	else
	{
		TARGET_STATIC(SetupCamPathCam_Ruin)(tp);
	}
}

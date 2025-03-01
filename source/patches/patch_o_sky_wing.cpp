#include "pch.h"
#include "multiplayer.h"

// Object that makes the beginning of SkyDeck act 2 disappear, we disable that in multiplayer

static void __cdecl ObjectSkydeck_wings_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectSkydeck_wings_r)> ObjectSkydeck_wings_h(0x5F2C60);

static void __cdecl ObjectSkydeck_wings_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		MaskBlock = 2;
	}
	else
	{
		ObjectSkydeck_wings_h.Original(tp);
	}
}

void patch_sky_wing_init()
{
	ObjectSkydeck_wings_h.Hook(ObjectSkydeck_wings_r);
}

RegisterPatch patch_sky_wing(patch_sky_wing_init);
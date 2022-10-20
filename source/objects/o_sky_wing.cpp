#include "pch.h"
#include "multiplayer.h"

// Object that makes the beginning of SkyDeck act 2 disappear, we disable that in multiplayer

static void __cdecl ObjectSkydeck_wings_r(task* tp);
Trampoline ObjectSkydeck_wings_t(0x5F2C60, 0x5F2C66, ObjectSkydeck_wings_r);
static void __cdecl ObjectSkydeck_wings_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		MaskBlock = 2;
	}
	else
	{
		TARGET_STATIC(ObjectSkydeck_wings)(tp);
	}
}
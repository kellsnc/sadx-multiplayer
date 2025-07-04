#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl ObjectSkydeck_c_talap0_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectSkydeck_c_talap0_Exec_r)> ObjectSkydeck_c_talap0_Exec_h(0x5FB5F0);

static void __cdecl ObjectSkydeck_c_talap0_Exec_r(task* tp)
{
	ObjectSkydeck_c_talap0_Exec_h.Original(tp);

	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto ocm = (OCMDATA*)twp->timer.ptr;

		if (IsPlayerOnDyncol(tp))
			ocm->flag |= 1;
		else
			ocm->flag &= ~1;
	}
}

void patch_sky_talap0_init()
{
	ObjectSkydeck_c_talap0_Exec_h.Hook(ObjectSkydeck_c_talap0_Exec_r);
}

RegisterPatch patch_sky_talap0(patch_sky_talap0_init);
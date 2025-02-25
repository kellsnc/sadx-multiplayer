#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl ObjectSkydeck_c_talap0_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectSkydeck_c_talap0_Exec_r)> ObjectSkydeck_c_talap0_Exec_t(0x5FB5F0, ObjectSkydeck_c_talap0_Exec_r);
static void __cdecl ObjectSkydeck_c_talap0_Exec_r(task* tp)
{
	ObjectSkydeck_c_talap0_Exec_t.Original(tp);

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
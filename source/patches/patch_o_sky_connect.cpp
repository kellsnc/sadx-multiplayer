#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl ObjectSkydeck_c_connect_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectSkydeck_c_connect_Exec_r)> ObjectSkydeck_c_connect_Exec_t(0x5FAE90);

static void __cdecl ObjectSkydeck_c_connect_Exec_r(task* tp)
{
	ObjectSkydeck_c_connect_Exec_t.Original(tp);

	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto ocm = (OCMDATA*)twp->timer.ptr;

		if (IsPlayerOnDyncol(tp))
			ocm->flag |= 1;
		else
			ocm->flag &= ~1;

		if (twp->mode >= 1)
		{
			tp->exec = (TaskFuncPtr)0x5FB130;
			return;
		}
	}
}

void patch_sky_connect_init()
{
	ObjectSkydeck_c_connect_Exec_t.Hook(ObjectSkydeck_c_connect_Exec_r);
}

RegisterPatch patch_sky_connect(patch_sky_connect_init);
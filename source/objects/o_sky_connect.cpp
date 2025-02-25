#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl ObjectSkydeck_c_connect_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectSkydeck_c_connect_Exec_r)> ObjectSkydeck_c_connect_Exec_t(0x5FAE90, ObjectSkydeck_c_connect_Exec_r);
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
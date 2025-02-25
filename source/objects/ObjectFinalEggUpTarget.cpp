#include "pch.h"
#include "multiplayer.h"

static void __cdecl uptarget_exec_wait_r(task* tp);
FastFunctionHookPtr<decltype(&uptarget_exec_wait_r)> uptarget_exec_wait_t(0x5B5740, uptarget_exec_wait_r);
static void __cdecl uptarget_exec_wait_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (IsPlayerInSphere(&twp->pos, twp->scl.x))
		{
			twp->mode = 1;
			twp->wtimer = 7;
		}
	}
	else
	{
		uptarget_exec_wait_t.Original(tp);
	}
}
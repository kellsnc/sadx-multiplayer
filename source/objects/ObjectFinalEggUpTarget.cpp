#include "pch.h"
#include "multiplayer.h"

static void __cdecl uptarget_exec_wait_r(task* tp);
Trampoline uptarget_exec_wait_t(0x5B5740, 0x5B5747, uptarget_exec_wait_r);
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
		TARGET_STATIC(uptarget_exec_wait)(tp);
	}
}
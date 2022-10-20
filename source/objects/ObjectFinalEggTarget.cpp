#include "pch.h"
#include "multiplayer.h"
#include "result.h"

static void __cdecl dummytarget_exec_r(task* tp);
Trampoline dummytarget_exec_t(0x5B5960, 0x5B5965, dummytarget_exec_r);
static void __cdecl dummytarget_exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto hit_twp = CCL_IsHitBullet(tp->twp);

		// Set the winner when a bullet collides
		// The last collision will be actual winner
		if (hit_twp)
		{
			SetWinnerMulti(hit_twp->btimer); // player number is stored in btimer thanks to patch in E102.cpp
		}
	}

	TARGET_STATIC(dummytarget_exec)(tp);
}
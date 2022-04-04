#include "pch.h"
#include "multiplayer.h"

// Water switches in Lost World (and the water iself)

static void __cdecl cNormal_r(task* tp);
Trampoline ObjectSuimen_cNormal_t(0x5E8410, 0x5E8415, cNormal_r);
static void __cdecl cNormal_r(task* tp)
{
	auto twp = tp->twp;
	auto awp = tp->awp;

	if (twp->cwp->flag & 1)
	{
		auto ptwp = playertwp[GetClosestPlayerNum(&twp->pos)];

		float dist = sqrtf((twp->pos.x - ptwp->pos.x) * (twp->pos.x - ptwp->pos.x)
			+ (twp->pos.z - ptwp->pos.z) * (twp->pos.z - ptwp->pos.z));

		if (dist < 11.0f)
		{
			if (awp->work.f[0] == 0.0f)
			{
				dsPlay_oneshot(200, 0, 0, 0);
				awp->work.f[0] = 1.0f;
			}

			tp->ptp->twp->flag |= 0x200;
			tp->ptp->twp->scl.z = twp->pos.y - 5.0f; // Set next water height
		}
		else
		{
			awp->work.f[0] = 0.0f;
		}
	}
	else
	{
		awp->work.f[0] = 0.0f;
	}

	EntryColliList(twp);
	ObjectSetupInput(twp, 0);

	tp->disp(tp);
}
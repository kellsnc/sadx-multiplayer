#include "pch.h"
#include "multiplayer.h"

// Water switches in Lost World (and the water iself)

static void __cdecl ObjectSuimen_cNormal_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectSuimen_cNormal_r)> ObjectSuimen_cNormal_h(0x5E8410);

static void __cdecl ObjectSuimen_cNormal_r(task* tp)
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

void patch_suimen_init()
{
	ObjectSuimen_cNormal_h.Hook(ObjectSuimen_cNormal_r);
}

RegisterPatch patch_suimen(patch_suimen_init);
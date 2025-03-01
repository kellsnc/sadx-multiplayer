#include "pch.h"
#include "FastFunctionHook.hpp"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

void ALO_FruitHeldP_r(task* tp);
static FastUsercallHookPtr<decltype(&ALO_FruitHeldP_r), noret, rEAX> ALO_FruitHeldP_h(0x007227C0);

void ALO_FruitHeldP_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;
		if (!twp->smode)
		{
			twp->smode = GetHoldingPlayerId(tp) + 1;
		}

		taskwk* orig_twp = playertwp[0];
		playertwp[0] = playertwp[twp->smode - 1];
		ALO_FruitHeldP_h.Original(tp);
		playertwp[0] = orig_twp;
	}
	else
	{
		ALO_FruitHeldP_h.Original(tp);
	}
}

void patch_alo_fruit_init()
{
	ALO_FruitHeldP_h.Hook(ALO_FruitHeldP_r);
}

RegisterPatch patch_alo_fruit(patch_alo_fruit_init);
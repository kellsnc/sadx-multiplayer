#include "pch.h"
#include "FastFunctionHook.hpp"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

void ALO_ObakeHeadHeldP_r(task* tp);
static FastUsercallHookPtr<decltype(&ALO_ObakeHeadHeldP_r), noret, rEAX> ALO_ObakeHeadHeldP_h(0x007230C0);

void ALO_ObakeHeadHeldP_r(task* tp)
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
		ALO_ObakeHeadHeldP_h.Original(tp);
		playertwp[0] = orig_twp;
	}
	else
	{
		ALO_ObakeHeadHeldP_h.Original(tp);
	}
}

void patch_alo_obake_head_init()
{
	ALO_ObakeHeadHeldP_h.Hook(ALO_ObakeHeadHeldP_r);
}

RegisterPatch patch_alo_obake_head(patch_alo_obake_head_init);
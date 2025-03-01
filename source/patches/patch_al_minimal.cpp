#include "pch.h"
#include "FastFunctionHook.hpp"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

void AL_MinimalHeldP_r(task* tp);
FastUsercallHookPtr<decltype(&AL_MinimalHeldP_r), noret, rEDI> AL_MinimalHeldP_h(0x00721C10);

void AL_MinimalHeldP_r(task* tp)
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
		AL_MinimalHeldP_h.Original(tp);
		playertwp[0] = orig_twp;
	}
	else
	{
		AL_MinimalHeldP_h.Original(tp);
	}
}

void patch_al_minimal_init()
{
	AL_MinimalHeldP_h.Hook(AL_MinimalHeldP_r);
}

RegisterPatch patch_al_minimal(patch_al_minimal_init);
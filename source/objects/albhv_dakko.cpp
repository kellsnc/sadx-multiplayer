#include "pch.h"
#include "FastFunctionHook.hpp"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

void DakkoControl_r(task* tp);
FastFunctionHookPtr<decltype(&DakkoControl_r)> DakkoControl_t(0x00739050, DakkoControl_r);
void DakkoControl_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		chaowk* twp = (chaowk*)tp->twp;
		if (twp->flag & TWK_FLAG_HELD)
		{
			twp->Behavior.SubMode = GetHoldingPlayerId(tp);
		}

		taskwk* orig_twp = playertwp[0];
		playertwp[0] = playertwp[twp->Behavior.SubMode];
		DakkoControl_t.Original(tp);
		playertwp[0] = orig_twp;
	}
	else
	{
		DakkoControl_t.Original(tp);
	}
}

Bool AL_CheckDakko_r(task* tp);
FastFunctionHookPtr<decltype(&AL_CheckDakko_r)> AL_CheckDakko_t(0x00739670, AL_CheckDakko_r);
Bool AL_CheckDakko_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* orig_twp = playertwp[0];
		playertwp[0] = playertwp[GetClosestPlayerNum(&tp->twp->pos)];
		Bool result = AL_CheckDakko_t.Original(tp);
		playertwp[0] = orig_twp;
		return result;
	}
	else
	{
		return AL_CheckDakko_t.Original(tp);
	}
}
#include "pch.h"
#include "Trampoline.h"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

void DakkoControl_r(task* tp);
Trampoline DakkoControl_t(0x00739050, 0x00739055, DakkoControl_r);
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
		TARGET_STATIC(DakkoControl)(tp);
		playertwp[0] = orig_twp;
	}
	else
	{
		TARGET_STATIC(DakkoControl)(tp);
	}
}

Bool AL_CheckDakko_r(task* tp);
Trampoline AL_CheckDakko_t(0x00739670, 0x00739675, AL_CheckDakko_r);
Bool AL_CheckDakko_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* orig_twp = playertwp[0];
		playertwp[0] = playertwp[GetClosestPlayerNum(&tp->twp->pos)];
		Bool result = TARGET_STATIC(AL_CheckDakko)(tp);
		playertwp[0] = orig_twp;
		return result;
	}
	else
	{
		return TARGET_STATIC(AL_CheckDakko)(tp);
	}
}
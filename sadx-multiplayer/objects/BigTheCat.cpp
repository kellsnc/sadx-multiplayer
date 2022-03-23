#include "pch.h"
#include "multiplayer.h"
#include "fishing.h"

static void __cdecl BigTheCat_r(task* tp);
Trampoline BigTheCat_t(0x490A00, 0x490A05, BigTheCat_r);
static void __cdecl BigTheCat_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		auto pnum = TASKWK_PLAYERID(tp->twp);
		auto etc = GetBigEtc(pnum);

		if (pnum >= 1 && pnum <= 3 && etc)
		{
			auto backup_ptr = Big_Lure_Ptr;
			auto backup_flag = Big_Fish_Flag;
			Big_Lure_Ptr = etc->Big_Lure_Ptr;
			Big_Fish_Flag = etc->Big_Fish_Flag;
			TARGET_STATIC(BigTheCat)(tp);
			etc->Big_Lure_Ptr = Big_Lure_Ptr;
			etc->Big_Fish_Flag = Big_Fish_Flag;
			Big_Lure_Ptr = backup_ptr;
			Big_Fish_Flag = backup_flag;
		}
		else
		{
			TARGET_STATIC(BigTheCat)(tp);
		}
	}
	else
	{
		TARGET_STATIC(BigTheCat)(tp);
	}
}
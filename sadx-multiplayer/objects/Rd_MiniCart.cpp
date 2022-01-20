#include "pch.h"

/*

Rewrite Twinkle Circuit manager to work for all players

*/

static void __cdecl deadRoundM(task* tp)
{
	ResetMleriRangeRad();
}

static void __cdecl execRoundM(task* tp)
{
	EnableController(0);
	EnableControl();
}

static void LoadAdditionalCarts()
{
	for (int i = 1; i < multiplayer::GetPlayerCount(); ++i)
	{
		auto tp = CreateElementalTask(LoadObj_Data1 | LoadObj_UnknownA | LoadObj_UnknownB, LEV_3, EnemyCart);

		if (tp && tp->twp)
		{
			tp->twp->btimer = i;

			if (playertwp[i])
			{
				tp->twp->scl.y = 1.0f;
				tp->twp->pos = { 1513.0f, 9.0f, 74.0f };
				tp->twp->ang.y = 0xC000;
			}
		}
	}
}

static void __cdecl initRoundM(task* tp, void* param_p)
{
	InitFreeCamera();
	LoadAdditionalCarts();
}

static const TaskInfo RdTaskInfoM = { 1, 2, initRoundM, execRoundM, 0, deadRoundM };

void __cdecl Rd_MiniCart_r(task* tp);
Trampoline Rd_MiniCart_t(0x4DAA80, 0x4DAA86, Rd_MiniCart_r);
void __cdecl Rd_MiniCart_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		MirenInitTask(tp, &RdTaskInfoM, nullptr);
	}
	else
	{
		TARGET_STATIC(Rd_MiniCart)(tp);
	}
}
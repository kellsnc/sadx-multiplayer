#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "multiplayer.h"

FastFunctionHook<void, task*> Bg_Past_h(0x542030);

void __cdecl Bg_Past_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		Bg_Past_h.Original(tp);
		return;
	}

	auto twp = tp->twp;

	switch (twp->mode)
	{
	case 0:
		InitBgAct(0, tp);
		twp->mode++;
		break;
	case 1:
		tp->disp = Past_Disp;
		twp->mode++;
		break;
	case 2:
		if (ssActNumber == twp->wtimer)
		{
			if (twp->wtimer == 2)
			{
				njSin(twp->value.l);
				twp->value.l += 1024;
			}

			tp->disp(tp);
		}
		else
		{
			twp->mode = 3;
		}
		break;
	case 3:
		InitBgAct(0, tp);
		twp->mode = 2;
		break;
	default:
		return;
	}
}

void patch_rd_past_init()
{
	Bg_Past_h.Hook(Bg_Past_r);
}

RegisterPatch patch_rd_past(patch_rd_past_init);
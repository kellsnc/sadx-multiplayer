#include "pch.h"
#include "utils.h"
#include "multiplayer.h"

static void execSkySinkRane_w();
Trampoline execSkySinkRane_t(0x5F5E50, 0x5F5E55, execSkySinkRane_w);

static void execSkySinkRane_o(task* tp)
{
	auto target = execSkySinkRane_t.Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

static void __cdecl execSkySinkRane_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 1)
	{
		auto twp = tp->twp;
		checkSkySinkRaneColli(twp);

		if (twp->scl.y < 0.0f)
		{
			auto pnum = GetClosestPlayerNumRange(&twp->pos, 300.0f);

			if (pnum >= 0)
			{
				dsPlay_oneshot_v(160, 0, 0, 64, twp->pos.x, twp->pos.y, twp->pos.z);
				twp->mode = 2i8;
			}
		}

		LoopTaskC(tp);
		tp->disp(tp);
	}
	else
	{
		execSkySinkRane_o(tp);
	}
}

static void __declspec(naked) execSkySinkRane_w()
{
	__asm
	{
		push eax
		call execSkySinkRane_r
		pop eax
		retn
	}
}
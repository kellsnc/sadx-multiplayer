#include "pch.h"
#include "multiplayer.h"

static void __cdecl ObjectHebigate_Normal_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectHebigate_Normal_r)> ObjectHebigate_Normal_h(0x5EADE0);

static void Normal_m(task* tp)
{
	if (!CheckRangeOut(tp))
	{
		auto twp = tp->twp;
		auto dyncol = (NJS_OBJECT*)twp->counter.ptr;

		MirenObjCheckCollisionP(twp, dyncol->basicdxmodel->r + 20.0f);
		LoopTaskC(tp);

		if (twp->value.l == 0)
		{
			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				auto ptwp = playertwp[i];

				if (ptwp && twp->pos.x - 400.0f < ptwp->pos.x)
				{
					twp->value.l = 1;
					dsPlay_timer_v(199, (int)twp, 1, 96, 2, twp->pos.x, twp->pos.y, twp->pos.z);
					break;
				}
			}
		}
		else
		{
			if (twp->value.l == 1)
			{
				twp->timer.f -= 1.0f;
				if (twp->timer.f < 1.0f)
				{
					twp->timer.f = 1.0f;
					twp->value.l = 2;
				}
			}
		}

		tp->disp(tp);
	}
}

static void __cdecl ObjectHebigate_Normal_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		Normal_m(tp);
	}
	else
	{
		ObjectHebigate_Normal_h.Original(tp);
	}
}

void patch_hebigate_init()
{
	ObjectHebigate_Normal_h.Hook(ObjectHebigate_Normal_r);
}

RegisterPatch patch_hebigate(patch_hebigate_init);
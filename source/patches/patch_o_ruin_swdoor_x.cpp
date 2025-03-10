#include "pch.h"
#include "multiplayer.h"

enum : __int8
{
	MODE_NORMAL,
	MODE_OPEN,
	MODE_OPENWAIT,
	MODE_CLOSE,
	MODE_END
};

static void __cdecl ObjectRuinSwdoorX_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectRuinSwdoorX_r)> ObjectRuinSwdoorX_h(0x5E7660);

static void ObjectRuinSwdoorX_m(task* tp)
{
	if (CheckRangeOut(tp))
		return;

	auto twp = tp->twp;

	if (CheckCollisionCylinderP(&twp->pos, 26.0f, 3.0f))
	{
		auto pnum = GetClosestPlayerNum(&twp->pos);

		NJS_POINT3 p;
		GetPlayerPosition(pnum, 0, &p, 0);
		if ((p.z - twp->pos.z) * twp->scl.z + (p.x - twp->pos.x) * twp->scl.x > 0.0f)
		{
			twp->mode = MODE_OPENWAIT;
			twp->wtimer = 0ui16;
			twp->counter.l = 0;
			twp->value.l = 1;
			dsPlay_iloop(194, (int)twp, 1, 0);
		}
	}

	EntryColliList(twp);
	tp->disp(tp);
}

static void __cdecl ObjectRuinSwdoorX_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 0)
	{
		ObjectRuinSwdoorX_m(tp);
	}
	else
	{
		ObjectRuinSwdoorX_h.Original(tp);
	}
}

void patch_ruin_swdoor_x_init()
{
	ObjectRuinSwdoorX_h.Hook(ObjectRuinSwdoorX_r);
}

RegisterPatch patch_ruin_swdoor_x(patch_ruin_swdoor_x_init);
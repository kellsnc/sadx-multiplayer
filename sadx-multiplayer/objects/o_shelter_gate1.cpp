#include "pch.h"
#include "multiplayer.h"
#include "fishing.h"

enum : __int8
{
	MODE_NORMAL,
	MODE_OPEN,
	MODE_WAIT,
	MODE_CLOSE,
	MODE_END
};

static void __cdecl execShelterGate1_r(task* tp); // "ExecATask"
Trampoline execShelterGate1_t(0x5A1F70, 0x5A1F78, execShelterGate1_r);
static void __cdecl execShelterGate1_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == MODE_NORMAL)
	{
		auto twp = tp->twp;
		auto pnum = CheckCollisionCylinderP(&twp->pos, 22.4f, 30.0f) - 1;

		if (pnum >= 0)
		{
			NJS_POINT3 v;
			GetPlayerPosition(pnum, 0, &v, 0);
			if ((v.z - twp->pos.z) * twp->value.f + (v.x - twp->pos.x) * twp->timer.f > 0.0f)
			{
				twp->mode = MODE_OPEN;
				twp->counter.l = 0;
			}
		}
	}
	else
	{
		TARGET_STATIC(execShelterGate1)(tp);
	}
}
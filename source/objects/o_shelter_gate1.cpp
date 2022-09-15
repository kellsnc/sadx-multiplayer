#include "pch.h"
#include "multiplayer.h"
#include "fishing.h"

enum : __int8
{
	MODE_INIT,
	MODE_NORMAL,
	MODE_OPEN,
	MODE_WAIT,
	MODE_CLOSE,
	MODE_END
};

static bool CheckCollisionCylinder(NJS_POINT3* pt, NJS_POINT3* vp, float r, float h)
{
	NJS_VECTOR v = *pt;
	njSubVector(&v, vp);
	return v.x * v.x + v.z * v.z - r * r <= 0.0f && njAbs(v.y) <= h;
}

static void __cdecl execShelterGate1_r(task* tp); // "ExecATask"
Trampoline execShelterGate1_t(0x5A1F70, 0x5A1F78, execShelterGate1_r);
static void __cdecl execShelterGate1_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == MODE_NORMAL)
	{
		auto twp = tp->twp;

		for (int pnum = 0; pnum < PLAYER_MAX; ++pnum)
		{
			NJS_POINT3 v;
			if (GetPlayerPosition(pnum, 0, &v, 0))
			{
				if (CheckCollisionCylinder(&v, &twp->pos, 22.4f, 30.0f))
				{
					if ((v.z - twp->pos.z) * twp->value.f + (v.x - twp->pos.x) * twp->timer.f > 0.0f)
					{
						twp->mode = MODE_OPEN;
						twp->counter.l = 0;
						break;
					}
				}
			}
		}

		tp->disp(tp);
	}
	else
	{
		TARGET_STATIC(execShelterGate1)(tp);
	}
}

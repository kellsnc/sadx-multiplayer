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

FastFunctionHook<void, task*> ObjShelterGate1_ExecATask_t(0x5A1F70);
FastFunctionHook<void, task*> ObjShelterGate2_ExecATask_t(0x59C850);

static bool CheckCollisionCylinder(NJS_POINT3* pt, NJS_POINT3* vp, float r, float h)
{
	NJS_VECTOR v = *pt;
	njSubVector(&v, vp);
	return v.x * v.x + v.z * v.z - r * r <= 0.0f && njAbs(v.y) <= h;
}

static void __cdecl ObjShelterGate1_ExecATask_r(task* tp)
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
		ObjShelterGate1_ExecATask_t.Original(tp);
	}
}

void __cdecl ObjShelterGate2_ExecATask_r(task* tp)
{
	auto twp = tp->twp;

	//if none of the player is Amy 
	if (CurrentCharacter != PLNO_AMY)
	{
		if (twp->mode < 2)
			twp->mode = 2; //force the door to open
	}

	ObjShelterGate2_ExecATask_t.Original(tp);
}

void patch_shelter_gate_init()
{
	ObjShelterGate1_ExecATask_t.Hook(ObjShelterGate1_ExecATask_r);
	ObjShelterGate2_ExecATask_t.Hook(ObjShelterGate2_ExecATask_r);
}

RegisterPatch patch_shelter_gate(patch_shelter_gate_init);
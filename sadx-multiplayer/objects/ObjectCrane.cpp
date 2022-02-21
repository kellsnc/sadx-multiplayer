#include "pch.h"
#include "multiplayer.h"

static void checkPlayerRideOnTheCage_w();
Trampoline checkPlayerRideOnTheCage_t(0x61B060, 0x61B067, checkPlayerRideOnTheCage_w);

BOOL checkPlayerRideOnTheCage_o(task* tp)
{
	auto target = checkPlayerRideOnTheCage_t.Target();
	BOOL ret;
	__asm
	{
		mov eax, [tp]
		call target
		mov ret, eax
	}
	return ret;
}

BOOL __cdecl checkPlayerRideOnTheCage_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;
		
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto ptwp = playertwp[i];

			if (ptwp && ptwp->flag & Status_Ground)
			{
				NJS_POINT3 p;
				p.x = ptwp->pos.x - twp->pos.x;
				p.y = ptwp->pos.y - twp->pos.y - 20.0f;
				p.z = ptwp->pos.z - twp->pos.z;

				njPushMatrix(_nj_unit_matrix_);
				if (twp->ang.y != 0x10000)
				{
					njRotateY(0, (uint16_t)-LOWORD(twp->ang.y));
				}
				njCalcVector(nullptr, &p, &p);
				njPopMatrixEx();

				if (fabs(p.x) < 20.0f && fabs(p.y) < 10.0f && fabs(p.z) < 20.0f)
				{
					return TRUE;
				}
			}
		}
		
		return FALSE;
	}
	else
	{
		return checkPlayerRideOnTheCage_o(tp);
	}
}

static void __declspec(naked) checkPlayerRideOnTheCage_w()
{
	__asm
	{
		push eax
		call checkPlayerRideOnTheCage_r
		add esp, 4
		retn
	}
}
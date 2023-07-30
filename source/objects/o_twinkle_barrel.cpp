#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

static void __cdecl ObjectTPBarrel_r(task* tp);
Trampoline ObjectTPBarrel_t(0x624020, 0x624026, ObjectTPBarrel_r);
static void __cdecl ObjectTPBarrel_r(task* tp)
{
	TARGET_STATIC(ObjectTPBarrel)(tp);

	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		if (twp->mode == 1 && CheckCollisionP(&twp->pos, 30.0f))
		{
			twp->mode = 2;
			dsPlay_iloop(SE_TPE_DOKA, -1, 8, 0);
		}
	}
}

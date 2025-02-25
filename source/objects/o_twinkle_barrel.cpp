#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"

static void __cdecl ObjectTPBarrel_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectTPBarrel_r)> ObjectTPBarrel_t(0x624020, ObjectTPBarrel_r);
static void __cdecl ObjectTPBarrel_r(task* tp)
{
	ObjectTPBarrel_t.Original(tp);

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

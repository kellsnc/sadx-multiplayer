#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "mod.h"
#include "multiplayer.h"

// Disable for now

void __cdecl ObjectTikalHint_r(task* tp);
FastFunctionHook<void, task*> ObjectTikalHint_h(0x7A1AA0);

void __cdecl ObjectTikalHint_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		CheckRangeOut(tp);
		return;
	}

	ObjectTikalHint_h.Original(tp);
}

void patch_tikal_hint_init()
{
	ObjectTikalHint_h.Hook(ObjectTikalHint_r);
}

RegisterPatch patch_tikal_hint(patch_tikal_hint_init);
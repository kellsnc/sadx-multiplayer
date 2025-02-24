#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "mod.h"
#include "multiplayer.h"

void __cdecl ObjectTikalHint_r(task* tp);
FastFunctionHook<void, task*> ObjectTikalHint_h(0x7A1AA0, ObjectTikalHint_r);

void __cdecl ObjectTikalHint_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		CheckRangeOut(tp);
		return;
	}

	ObjectTikalHint_h.Original(tp);
}

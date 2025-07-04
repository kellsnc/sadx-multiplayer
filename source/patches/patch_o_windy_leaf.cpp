#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

FastFunctionHookPtr<TaskFuncPtr> PathScanLeaf_h(0x4E4430);

// Use CheckRangeOutWithR as it should have
void __cdecl PathScanLeaf_r(task* tp)
{
	if (!CheckRangeOutWithR(tp, 1960000.0f))
	{
		PathScanLeaf_h.Original(tp);
	}
}

void patch_windy_leaf_init()
{
	PathScanLeaf_h.Hook(PathScanLeaf_r);
	WriteData((uint8_t*)0x4E4A04, 0xEBui8); // Ignore FreeTask in exec after checking P1 distance...
	WriteData((uint8_t*)0x4E4344, 0xEBui8); // Ignore FreeTask in display...

	WriteData((uint8_t*)0x4E5253, (uint8_t)PLAYER_MAX); // PushUpWind for all players
}

RegisterPatch patch_windy_leaf(patch_windy_leaf_init);
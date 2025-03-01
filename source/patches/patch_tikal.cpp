#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "splitscreen.h"

FastFunctionHook<void, task*> TikalDisplay_h(0x7B33A0);

// Add shadow rendering in Tikal's display routine because they forgot it
void TikalDisplay_r(task* tp)
{
	TikalDisplay_h.Original(tp);

	if (ChkPause() || SplitScreen::numScreen != 0)
	{
		DrawCharacterShadow(tp->twp, &((playerwk*)tp->mwp->work.ptr)->shadow);
	}
}

void patch_tikal_init()
{
	TikalDisplay_h.Hook(TikalDisplay_r);
}

RegisterPatch patch_tikal(patch_tikal_init);
#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "splitscreen.h"

// Minimal stack hud

void __cdecl ExtraDisplayExecutor_r(task* tp);
FastFunctionHook<void, task*> ExtraDisplayExecutor_h(0x46B330, ExtraDisplayExecutor_r);
void __cdecl ExtraDisplayExecutor_r(task* tp)
{
	if (tp->twp->mode != 0)
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		ExtraDisplayExecutor_h.Original(tp);
		SplitScreen::RestoreViewPort();
	}
	else
	{
		ExtraDisplayExecutor_h.Original(tp);
	}
}

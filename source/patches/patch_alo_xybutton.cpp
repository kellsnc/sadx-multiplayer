#include "pch.h"
#include "FastFunctionHook.hpp"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

// Just disabled for now

static void XYButton_FDisp_r();
static FastFunctionHookPtr<decltype(&XYButton_FDisp_r)> XYButton_FDisp_h(0x0071B210);

static void XYButton_FDisp_r()
{
	if (!multiplayer::IsActive())
	{
		XYButton_FDisp_h.Original();
	}
}

void patch_alo_xybutton_init()
{
	XYButton_FDisp_h.Hook(XYButton_FDisp_r);
}

RegisterPatch patch_alo_xybutton(patch_alo_xybutton_init);
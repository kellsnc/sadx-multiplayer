#include "pch.h"
#include "FastFunctionHook.hpp"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

// Just disabled for now

static void XYButton_FDisp_r();
static FastFunctionHookPtr<decltype(&XYButton_FDisp_r)> XYButton_FDisp_t(0x0071B210, XYButton_FDisp_r);
static void XYButton_FDisp_r()
{
	if (!multiplayer::IsActive())
	{
		XYButton_FDisp_t.Original();
	}
}
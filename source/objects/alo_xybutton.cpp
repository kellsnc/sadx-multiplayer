#include "pch.h"
#include "Trampoline.h"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

// Just disabled for now

static void XYButton_FDisp_r();
static Trampoline XYButton_FDisp_t(0x0071B210, 0x0071B215, XYButton_FDisp_r);
static void XYButton_FDisp_r()
{
	if (!multiplayer::IsActive())
	{
		TARGET_STATIC(XYButton_FDisp)();
	}
}
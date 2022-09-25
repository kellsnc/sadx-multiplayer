#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Fix camera paths
// TODO: find the courage to do this

static void __cdecl InitPathworkCamera_r(task* tp);
Trampoline InitPathworkCamera_t(0x4BBF80, 0x4BBF87, InitPathworkCamera_r);
static void __cdecl InitPathworkCamera_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		
	}
	else
	{
		TARGET_STATIC(InitPathworkCamera)(tp);
	}
}


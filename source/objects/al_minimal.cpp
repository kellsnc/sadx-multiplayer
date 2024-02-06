#include "pch.h"
#include "Trampoline.h"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

static void AL_MinimalHeldP_asm();
static Trampoline AL_MinimalHeldP_t(0x00721C10, 0x00721C17, AL_MinimalHeldP_asm);

void __cdecl AL_MinimalHeldP_orig(task* tp)
{
	const auto orig = AL_MinimalHeldP_t.Target();
	__asm
	{
		mov edi, tp
		call orig
	}
}

void AL_MinimalHeldP_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;
		if (!twp->smode)
		{
			twp->smode = GetHoldingPlayerId(tp) + 1;
		}

		taskwk* orig_twp = playertwp[0];
		playertwp[0] = playertwp[twp->smode - 1];
		AL_MinimalHeldP_orig(tp);
		playertwp[0] = orig_twp;
	}
	else
	{
		AL_MinimalHeldP_orig(tp);
	}
}

static void __declspec(naked) AL_MinimalHeldP_asm()
{
	__asm
	{
		push edi
		call AL_MinimalHeldP_r
		pop edi
		retn
	}
}
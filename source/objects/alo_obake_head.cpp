#include "pch.h"
#include "Trampoline.h"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

static void ALO_ObakeHeadHeldP_asm();
static Trampoline ALO_ObakeHeadHeldP_t(0x007230C0, 0x007230C5, ALO_ObakeHeadHeldP_asm);

void __cdecl ALO_ObakeHeadHeldP_orig(task* tp)
{
	const auto orig = ALO_ObakeHeadHeldP_t.Target();
	__asm
	{
		mov eax, tp
		call orig
	}
}

void ALO_ObakeHeadHeldP_r(task* tp)
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
		ALO_ObakeHeadHeldP_orig(tp);
		playertwp[0] = orig_twp;
	}
	else
	{
		ALO_ObakeHeadHeldP_orig(tp);
	}
}

static void __declspec(naked) ALO_ObakeHeadHeldP_asm()
{
	__asm
	{
		push eax
		call ALO_ObakeHeadHeldP_r
		pop eax
		retn
	}
}
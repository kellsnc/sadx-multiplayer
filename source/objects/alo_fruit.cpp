#include "pch.h"
#include "Trampoline.h"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"

static void ALO_FruitHeldP_asm();
static Trampoline ALO_FruitHeldP_t(0x007227C0, 0x007227C5, ALO_FruitHeldP_asm);

void __cdecl ALO_FruitHeldP_orig(task* tp)
{
	const auto orig = ALO_FruitHeldP_t.Target();
	__asm
	{
		mov eax, tp
		call orig
	}
}

void ALO_FruitHeldP_r(task* tp)
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
		ALO_FruitHeldP_orig(tp);
		playertwp[0] = orig_twp;
	}
	else
	{
		ALO_FruitHeldP_orig(tp);
	}
}

static void __declspec(naked) ALO_FruitHeldP_asm()
{
	__asm
	{
		push eax
		call ALO_FruitHeldP_r
		pop eax
		retn
	}
}
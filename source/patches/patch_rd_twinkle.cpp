#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "multiplayer.h"
#include "splitscreen.h"
#include "camera.h"
#include "teleport.h"

static auto setTPFog = GenerateUsercallWrapper<void (*)(unsigned __int8 mode)>(noret, 0x61CAC0, rAL);

FunctionPointer(void, TwinkleMaskBlock, (taskwk* twp), 0x60FEE0); // checkCamera

FastFunctionHook<void, task*> Rd_Twinkle_t(0x61D150);
FastFunctionHook<void, task*> dispBgTwinkle_t(0x61D1F0);

// Patch act swaps
void __cdecl Rd_Twinkle_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		switch (twp->mode)
		{
		case 0i8:
			SetFreeCameraMode(1);
			rdTwinkleInit(tp);
			tp->dest = (TaskFuncPtr)0x61CA80;
			break;
		case 1i8:
			SetFreeCameraMode(0);

			if (IsPlayerInSphere(-6550.0f, -6720.0f, 23320.0f, 50.0f))
			{
				ChangeActM(1);
				rdTwinkleInit(tp);
				return;
			}
			break;
		case 2i8:
			SetFreeCameraMode(0);

			if (IsPlayerInSphere(-55.0f, 153.0f, -1000.0f, 50.0f))
			{
				tp->twp->mode = 3;
				EV_NpcMilesStandByOff();
				rdTwinkleInit(tp);
				setTPFog(tp->twp->mode);
				return;
			}
			break;
		case 3i8:
		{
			SetFreeCameraMode(1);

			if (IsPlayerInSphere(80.0f, 0.0f, -300.0f, 50.0f))
			{
				ChangeActM(1);
				rdTwinkleInit(tp);
				TeleportPlayers(82.0f, 0.0f, -305.0f);
			}

			break;
		}
		case 4i8:
		{
			SetFreeCameraMode(0);

			if (IsPlayerInSphere(350.0f, 100.0f, 550.0f, 36.0f))
			{
				ChangeActM(-1);
				rdTwinkleInit(tp);
				TeleportPlayers(328.0f, 100.0f, 566.0f);
			}

			break;
		}
		case 5i8:
			ResetMleriRangeRad();
			break;
		}
	}
	else
	{
		Rd_Twinkle_t.Original(tp);
	}
}

// Fix display masks
static void __cdecl dispBgTwinkle_r(task* tp)
{
	dispBgTwinkle_t.Original(tp);

	if (camera_twp && pRd_Master && SplitScreen::IsActive())
	{
		TwinkleMaskBlock(pRd_Master->twp);
	}
}

// Use layout character instead of player 1 character ID
Sint32 __cdecl GetPlayerCharacterName_TP_Hack(Uint8 pno)
{
	if (multiplayer::IsActive())
	{
		return CurrentCharacter;
	}

	return GetPlayerCharacterName(pno);
}

void patch_rd_twinkle_init()
{
	Rd_Twinkle_t.Hook(Rd_Twinkle_r);
	dispBgTwinkle_t.Hook(dispBgTwinkle_r);

	WriteCall((void*)0x61CB77, GetPlayerCharacterName_TP_Hack);
	WriteData((void**)0x61D57E, (void*)0x61D4E0); // Patch skybox mode
}

RegisterPatch patch_rd_twinkle(patch_rd_twinkle_init);
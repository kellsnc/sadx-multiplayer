#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "teleport.h"

// In multiplayer, check the character layout rather than player 1 character ID

Sint32 __cdecl RdCasinoCheckAct_Hack(Uint8 pno)
{
	if (multiplayer::IsActive() && CurrentCharacter != Characters_Knuckles)
	{
		return Characters_Sonic;
	}

	return GetPlayerCharacterName(pno);
}

void ResetBallCount();
FastFunctionHook<void, task*> Rd_Casinopolis_h(0x5C0F40);
void Rd_Casinopolis_r(task* tp)
{
	auto twp = tp->twp;

	switch (twp->mode)
	{
	case 0:
		ResetBallCount();
		break;
	case 2:
		if (IsIngame() && (RdCasino_JumpAct == 3 || RdCasino_JumpAct == 4))
		{
			ResetBallCount();
		}
		break;
	}


	Rd_Casinopolis_h.Original(tp);
}

void Casino_StartPos_Hack(Uint8 pno, Float x, Float y, Float z)
{
	if (multiplayer::IsActive())
	{
		NJS_POINT3 pos = { x, y, z };
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (playertwp[i])
			{
				TeleportPlayerArea(i, &pos, 5.0f);
			}
		}
	}
	else
	{
		SetPositionP(pno, x, y, z);
	}
}

void patch_rd_casino_init()
{
	// Casino knuckles checks
	WriteCall((void*)0x5C060B, RdCasinoCheckAct_Hack);
	WriteCall((void*)0x5C058B, RdCasinoCheckAct_Hack);
	WriteCall((void*)0x5C068B, RdCasinoCheckAct_Hack);
	WriteCall((void*)0x5C441A, RdCasinoCheckAct_Hack);

	// Casino area positions
	WriteCall((void*)0x5C0D67, Casino_StartPos_Hack);
	WriteCall((void*)0x5C0D9B, Casino_StartPos_Hack);
	WriteCall((void*)0x5C0DCB, Casino_StartPos_Hack);
	WriteCall((void*)0x5C0E19, Casino_StartPos_Hack);
	WriteCall((void*)0x5C0E77, Casino_StartPos_Hack); // pinball
	WriteCall((void*)0x5C0EF1, Casino_StartPos_Hack); // pinball

	Rd_Casinopolis_h.Hook(Rd_Casinopolis_r);
}

RegisterPatch patch_rd_casino(patch_rd_casino_init);
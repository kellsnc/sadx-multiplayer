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

void __cdecl Casino_StartPos_Hack(Uint8 pno, Float x, Float y, Float z)
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
	//WriteCall((void*)0x5C0E77, Casino_StartPos_Hack); // pinball
	//WriteCall((void*)0x5C0EF1, Casino_StartPos_Hack); // pinball
}

RegisterPatch patch_rd_casino(patch_rd_casino_init);
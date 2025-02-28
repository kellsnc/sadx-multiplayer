#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

// In multiplayer, check the character layout rather than player 1 character ID

Sint32 __cdecl RdCasinoCheckAct_Hack(Uint8 pno)
{
	if (multiplayer::IsActive() && CurrentCharacter != Characters_Knuckles)
	{
		return Characters_Sonic;
	}

	return GetPlayerCharacterName(pno);
}

void patch_rd_casino_init()
{
	// Casino knuckles checks
	WriteCall((void*)0x5C060B, RdCasinoCheckAct_Hack);
	WriteCall((void*)0x5C058B, RdCasinoCheckAct_Hack);
	WriteCall((void*)0x5C068B, RdCasinoCheckAct_Hack);
	WriteCall((void*)0x5C441A, RdCasinoCheckAct_Hack);
}

RegisterPatch patch_rd_casino(patch_rd_casino_init);
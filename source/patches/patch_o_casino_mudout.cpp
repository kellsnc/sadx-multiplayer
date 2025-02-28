#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "multiplayer.h"

// Do not run shaking mud off at the end of sewers (crashes Tails)
// Todo: fix Tails instead.

void __cdecl ExecMud_SetInputP_Hack(Uint8 pno, Uint8 mode)
{
	if (!multiplayer::IsActive())
	{
		SetInputP(pno, mode);
	}
}

void patch_casino_mudout_init()
{
	WriteCall((void*)0x5C5906, ExecMud_SetInputP_Hack); 
}

RegisterPatch patch_casino_mudout(patch_casino_mudout_init);
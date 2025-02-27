#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "patches.h"

// Change character boss ID from 1 to 7 to avoid conflict with other players.

FastFunctionHook<void, int> SeqExecBossPlayer_t(0x413840);

static uint8_t aiPAD = 7;

void SetNPCController()
{
	aiPAD = multiplayer::IsActive() ? 7 : 1;
	
	//Knux AI
	WriteData<1>((uint8_t*)0x4D5E3A, aiPAD);
	WriteData((int*)0x4D5E42, (int)&perG[aiPAD].on);
	WriteData((int*)0x4D5E48, (int)&perG[aiPAD].press);

	WriteData((int*)0x4D64EE, (int)&input_dataG[aiPAD].angle);
	WriteData((int*)0x4D64F4, (int)&input_dataG[aiPAD].stroke);

	WriteData((int*)0x4D64FA, (int)&perG[aiPAD].on);
	WriteData((int*)0x4D6500, (int)&perG[aiPAD].press);
	WriteData((int*)0x4D644A, (int)&perG[aiPAD].y1);

	//Sonic AI
	WriteData((int*)0x4B646C, (int)&perG[aiPAD].on);
	WriteData((int*)0x4B6472, (int)&perG[aiPAD].press);
}

void __cdecl SeqExecBossPlayer_r(int boss_id)
{
	//SetNPCController();

	if (multiplayer::IsActive())
	{
		auto P = playertp[1];
		if (P)
		{
			FreeTask(P);
		}
	}

	SeqExecBossPlayer_t.Original(boss_id);
}

void patch_seqctrl_init()
{
	SeqExecBossPlayer_t.Hook(SeqExecBossPlayer_r);
}

RegisterPatch patch_seqctrl(patch_seqctrl_init);
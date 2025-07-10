#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"

FastFunctionHook<void, int> EV_Load2_h(EV_Load2);
FastFunctionHook<void> EV_Exec2_h(EV_Exec2);
FastFunctionHook<void, int> EV_InitPlayer_h(EV_InitPlayer);
FastFunctionHook<Bool> CheckAndPlayLevelCutscene_h(0x413B20);
FastFunctionHook<void, int> SeqEvent_h(0x4136E0);

static char backupMode = 0;

static int bannedCutscene[] = { 0x2 };
void EV_Load2_r(int no)
{
	if (multiplayer::IsActive())
	{
		return;


		/**for (uint16_t j = 0; j < LengthOfArray(bannedCutscene); j++)
		{
			if (bannedCutscene[j] == no)
			{
				return EV_Load2_h.Original(no);
			}
		}


		for (uint8_t i = 1u; i < PLAYER_MAX; i++)
		{
			auto P = playertp[i];
			if (P)
			{
				FreeTask(P);
			}

		}*/
	}
	
	return EV_Load2_h.Original(no);
}

void EV_Exec2_r(void)
{
	if (multiplayer::IsActive())
	{
		return;
	}

	EV_Exec2_h.Original();
}

Bool CheckAndPlayLevelCutscene_r()
{
	if (multiplayer::IsActive())
	{
		return FALSE;
	}

	return CheckAndPlayLevelCutscene_h.Original();
}

void RecreateMultiPlayer()
{
	SetOtherPlayers();
	return RunLevelDestructor(5);
}

//fix arbitrary crashes and random bugs in cutscenes (race issue)
void DelayCutsceneStart(int a1)
{
	EV_InitPlayer_h.Original(a1);

	if (multiplayer::IsActive())
		EV_Wait(5);
}


void __cdecl SeqEvent_r(int flag)
{
	if (multiplayer::IsActive())
	{
		return;
	}

	SeqEvent_h.Original(flag);

}

void initEvents()
{

	WriteData<1>((int*)0x42FD98, 0x3); //Load Event Char: change task lvl index from 1 to 3, this adds a small delay to let us time to delete other players
	EV_Load2_h.Hook(EV_Load2_r);
	EV_Exec2_h.Hook(EV_Exec2_r);
	//WriteJump((void*)0x431286, RecreateMultiPlayer);
	//WriteCall((void*)0x431385, RecreateMultiPlayer);
	//<--patches-->
	EV_InitPlayer_h.Hook(DelayCutsceneStart);
	CheckAndPlayLevelCutscene_h.Hook(CheckAndPlayLevelCutscene_r);

	WriteData<5>((int*)0x41362D, 0x90);

	SeqEvent_h.Hook(SeqEvent_r);

	//WriteCall((void*)0x413573, sub_44ED40_r);
}
#include "pch.h"
#include "players.h"

static FunctionHook<void, int> EV_Load2_t(EV_Load2);
static char backupMode = 0;
static FunctionHook<void> EV_Exec2_t(EV_Exec2);

static int bannedCutscene[] = { 0x2 };
void EV_Load2_r(int no)
{
	if (multiplayer::IsActive())
	{
		for (uint16_t j = 0; j < LengthOfArray(bannedCutscene); j++)
		{
			if (bannedCutscene[j] == no)
			{
				return EV_Load2_t.Original(no);
			}
		}

		for (uint8_t i = 1u; i < PLAYER_MAX; i++)
		{
			auto P = playertp[i];
			if (P)
			{
				FreeTask(P);
			}

		}
	}
	
	return EV_Load2_t.Original(no);
}

void RecreateMultiPlayer()
{
	SetOtherPlayers();
	return RunLevelDestructor(5);
}

//fix arbitrary crash on some cutscene (race issue)
void DelayCutsceneStart(int a1)
{
	EV_InitPlayer(a1);
	EV_Wait(5);
}

void initEvents()
{

	WriteData<1>((int*)0x42FD98, 0x3); //Load Event Char: change task lvl index from 1 to 3 (fix cutscene char crash, race process weld issue)
	EV_Load2_t.Hook(EV_Load2_r);
	WriteJump((void*)0x431286, RecreateMultiPlayer);
	WriteCall((void*)0x431385, RecreateMultiPlayer);
	//<--patches-->
	//patch Post Sky Deck cutscene (race)
	WriteCall((void*)0x656FE7, DelayCutsceneStart);
	WriteCall((void*)0x655B66, DelayCutsceneStart);
}
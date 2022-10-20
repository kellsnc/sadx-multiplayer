#include "pch.h"
#include "players.h"

static FunctionHook<void, int> EV_Load2_t(EV_Load2);
static char backupMode = 0;

int timer = 0;
void EV_Load2_r(int no)
{
	if (multiplayer::IsActive())
	{
		backupMode = multiplayer::IsCoopMode() ? 0 : 1; //unused for now

		for (int i = 1; i < PLAYER_MAX; i++)
		{
			if (playertp[i])
			{
				FreeTask(playertp[i]);
			}
		}

		//multiplayer::TemporaryDisable();
	}

	return EV_Load2_t.Original(no);
}

void RecreateMultiPlayer()
{
	SetOtherPlayers();
	return RunLevelDestructor(5);
}

void initEvents()
{
	WriteData<1>((int*)0x42FD98, 0x3); //Load Event Char: change task lvl index from 1 to 3 (fix cutscene char crash, race process weld issue)
	EV_Load2_t.Hook(EV_Load2_r);
	WriteJump((void*)0x431286, RecreateMultiPlayer);
	WriteCall((void*)0x431385, RecreateMultiPlayer);
}
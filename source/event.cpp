#include "pch.h"
#include "players.h"

static FunctionHook<void, int> EV_Load2_t(EV_Load2);
static char backupMode = 0;

void EV_Load2_r(int no)
{
	if (multiplayer::IsActive())
	{
		backupMode = multiplayer::IsCoopMode() ? 0 : 1;

		for (int i = 1; i < PLAYER_MAX; i++)
		{
			if (playertwp[i])
			{
				FreeTask((task*)GetCharacterObject(i));
			}
		}

		//multiplayer::TemporaryDisable();
	}

	return EV_Load2_t.Original(no);
}

void RecreateMultiPlayer()
{
	//ultiplayer::Enable(multiplayer::GetPlayerCount(), (multiplayer::mode)backupMode);
	SetOtherPlayers();
	return RunLevelDestructor(5);
}

void initEvents()
{
	EV_Load2_t.Hook(EV_Load2_r);
	WriteJump((void*)0x431286, RecreateMultiPlayer);
	WriteCall((void*)0x431385, RecreateMultiPlayer);
}
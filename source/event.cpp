#include "pch.h"
#include "players.h"

static FunctionHook<void, int> EV_Load2_t(EV_Load2);

void EV_Load2_r(int no)
{
	if (multiplayer::IsActive())
	{
		for (int i = 1; i < PLAYER_MAX; i++)
		{
			if (playertwp[i])
			{
				FreeTask((task*)GetCharacterObject(i));
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

void initEvents()
{
	EV_Load2_t.Hook(EV_Load2_r);
	WriteJump((void*)0x431286, RecreateMultiPlayer);
	WriteCall((void*)0x431385, RecreateMultiPlayer);
}
#include "pch.h"
#include "result.h"
#include "hud_result.h"

Trampoline* SetFinishAction_t = nullptr;

int GetWinnerMulti()
{
	return MRaceResult;
}

void SetFinishAction_r()
{
	if (multiplayer::IsActive())
	{
		ToggleControllers(false);
		PauseEnabled = FALSE;
		SleepTimer();

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			ForcePlayerAction(i, 19);
		}

		LoadMultiplayerResult();
	}
	else
	{
		TARGET_DYNAMIC(SetFinishAction)();
	}
}

void InitResult()
{
	SetFinishAction_t = new Trampoline(0x415540, 0x415545, SetFinishAction_r);
}
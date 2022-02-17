#include "pch.h"
#include "result.h"
#include "hud_result.h"

Trampoline* SetFinishAction_t = nullptr;

int GetWinnerMulti()
{
	return MRaceResult;
}

void SetWinnerMulti(int pnum)
{
	MRaceResult = pnum;
}

static void PlayCharaWinSound()
{
	auto pnum = GetWinnerMulti();

	if (pnum >= 0)
	{
		auto twp = playertwp[pnum];

		if (twp)
		{
			switch (TASKWK_CHARID(twp))
			{
			case Characters_Sonic:
				Load_DelayedSound_SFX(1495);
				break;
			case Characters_Tails:
				Load_DelayedSound_SFX(1458);
				break;
			case Characters_Knuckles:
				Load_DelayedSound_SFX(1445);
				break;
			case Characters_Amy:
				Load_DelayedSound_SFX(1388);
				break;
			case Characters_Gamma:
				Load_DelayedSound_SFX(1425);
				break;
			case Characters_MetalSonic:
				PlayVoice(2044);
				break;
			}
		}
	}
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

		if (CurrentLevel == LevelIDs_TwinkleCircuit)
		{
			ADX_Close();
		}
		else
		{
			Load_DelayedSound_BGM(75);
			PlayCharaWinSound();
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
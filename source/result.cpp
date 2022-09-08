#include "pch.h"
#include "result.h"
#include "hud_result.h"
#include "splitscreen.h"

static FunctionHook<void> SetFinishAction_t(SetFinishAction);
static FunctionHook<void, task*> CalcTotalScore_t((intptr_t)0x42BCC0);

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

static void __cdecl SetFinishAction_r()
{
	if (multiplayer::IsActive())
	{
		PadReadOffP(-1);
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

		if (multiplayer::IsBattleMode())
			LoadMultiplayerResult();
	}
	else
	{
		SetFinishAction_t.Original();
	}
}

static void __cdecl CalcTotalScore_r(task* tp)
{
	SplitScreen::SaveViewPort();
	SplitScreen::ChangeViewPort(-1);
	CalcTotalScore_t.Original(tp);
	SplitScreen::RestoreViewPort();
}

void InitResult()
{
	SetFinishAction_t.Hook(SetFinishAction_r); 
	CalcTotalScore_t.Hook(CalcTotalScore_r);
}
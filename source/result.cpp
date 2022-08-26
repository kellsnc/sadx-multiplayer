#include "pch.h"
#include "result.h"
#include "hud_result.h"
#include "splitscreen.h"

Trampoline* SetFinishAction_t = nullptr;
Trampoline* CalcTotalScore_t = nullptr;

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
		TARGET_DYNAMIC(SetFinishAction)();
	}
}

static void __cdecl CalcTotalScore_r(task* tp)
{
	SplitScreen::SaveViewPort();
	SplitScreen::ChangeViewPort(-1);
	TARGET_DYNAMIC(CalcTotalScore)(tp);
	SplitScreen::RestoreViewPort();
}

void InitResult()
{
	SetFinishAction_t = new Trampoline(0x415540, 0x415545, SetFinishAction_r);
	CalcTotalScore_t = new Trampoline(0x42BCC0, 0x42BCC5, CalcTotalScore_r);
}
#include "pch.h"
#include "result.h"
#include "hud_result.h"
#include "splitscreen.h"
#include "teleport.h"
#include "milesrace.h"
#include "result.h"

static FunctionHook<void> SetFinishAction_t(SetFinishAction);
static FunctionHook<void, task*> CalcTotalScore_t((intptr_t)0x42BCC0);

static int gBattleResult = 0;

bool CheckDefeat(int pnum)
{
	if (multiplayer::IsCoopMode())
	{
		return (pnum != NPC_PNUM && RaceWinnerPlayer == 2) || (pnum == NPC_PNUM && RaceWinnerPlayer == 1);
	}
	else
	{
		return gBattleResult != pnum;
	}
}

int GetWinnerMulti()
{
	return gBattleResult;
}

void SetWinnerMulti(int pnum)
{
	if (multiplayer::IsCoopMode())
	{
		if (pnum != NPC_PNUM)
			SetTailsRaceVictory();
		else
			SetOpponentRaceVictory();
	}

	gBattleResult = pnum;
}

void MovePlayersToWinnerPos(NJS_VECTOR* endpos)
{
	if (multiplayer::IsCoopMode())
	{
		auto pnum = GetWinnerMulti();

		if (pnum >= 0 && pnum < PLAYER_MAX)
		{
			taskwk* winner = playertwp[pnum];
			NJS_VECTOR pos = winner->pos;

			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				taskwk* ptwp = playertwp[i];
				if (ptwp && GetDistance(endpos, &ptwp->pos) > 40.0f)
				{
					ptwp->ang.y = winner->ang.y;
					TeleportPlayerArea(i, &pos, 8.0f);
				}
			}
		}
	}
}

static void PlayCharaResultSound(int pnum)
{
	if (pnum >= 0 && pnum < PLAYER_MAX && playertwp[pnum])
	{
		if (CheckDefeat(pnum))
		{
			switch (TASKWK_CHARID(playertwp[pnum]))
			{
			case Characters_Sonic:
				GM_SECall(SE_SV_DEAD);
				break;
			case Characters_Tails:
				GM_SECall(SE_MV_DEAD);
				break;
			case Characters_Knuckles:
				GM_SECall(SE_KV_DEAD);
				break;
			case Characters_Amy:
				GM_SECall(SE_AV_DEAD);
				break;
			case Characters_Gamma:
				GM_SECall(SE_EV_DEAD);
				break;
			case Characters_MetalSonic:
				PlayVoice(2044);
				break;
			}
		}
		else
		{
			switch (TASKWK_CHARID(playertwp[pnum]))
			{
			case Characters_Sonic:
				GM_SECall(SE_SV_CLEAR);
				break;
			case Characters_Tails:
				GM_SECall(SE_MV_CLEAR);
				break;
			case Characters_Knuckles:
				GM_SECall(SE_KV_CLEAR);
				break;
			case Characters_Amy:
				GM_SECall(SE_AV_CLEAR);
				break;
			case Characters_Gamma:
				GM_SECall(SE_EV_CLEAR);
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

		if (CurrentLevel == LevelIDs_TwinkleCircuit)
		{
			ADX_Close();
		}
		else
		{
			MovePlayersToWinnerPos(&playertwp[0]->pos);
			Load_DelayedSound_BGM(75);
			multiplayer::IsBattleMode() ? PlayCharaResultSound(GetWinnerMulti()) : PlayCharaResultSound(0);
			SetLocalPathCamera(&pathtag_s_camera, 3, 720);

			for (int i = 0; i < 8; ++i)
			{
				if (playertwp[i])
				{
					ForcePlayerAction(i, PL_OP_PLACEWITHKIME);
				}
			}
		}

		pdVibMxStop(0);
		multiplayer::IsBattleMode() ? LoadMultiplayerResult() : LoadMultiplayerCoopResult();
		return;
	}

	SetFinishAction_t.Original();
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
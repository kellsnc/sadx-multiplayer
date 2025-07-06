#include "pch.h"

static uint8_t BallCount[PLAYER_MAX - 1]; //we don't need to do Player 1
FastFunctionHook<void> Casino_Setup_h(0x5C0960);
FunctionPointer(int, initMissionCtrl, (), 0x5919E0);

#define BallMax 3

static bool SubBallCount(uint8_t pnum)
{
	if (pnum > multiplayer::GetPlayerCount())
		return false;

	if (!pnum)
		pnum++;

	return --BallCount[pnum - 1] > 0;
}

void ResetBallCount()
{
	memset(BallCount, BallMax, sizeof(uint8_t) * PLAYER_MAX - 1);
}

static int8_t GetLastPlayerAlive()
{
	int count = 0;
	uint8_t id = 0;
	const int playerCount = multiplayer::GetPlayerCount();
	for (uint8_t i = 0; i < playerCount; i++)
	{
		if (i > 0 && BallCount[i - 1] == 0)
		{
			count++;
		}
		else if (!i && Casino_BallCount == 0)
		{
			count++;
		}
		else
		{
			id = i; //alive
		}
	}

	if (count != playerCount)
		return -1;

	return id;
}

void __cdecl Casino_Setup_r()
{
	Casino_Setup_h.Original();
	ResetBallCount();
}

FastFunctionHook<void, task*> RdCasinoCheckAct3toAct12_h(0x5C0700);

static void RdCasinoCheckAct3toAct12_r(task* tp)
{

	if (multiplayer::IsActive() == false)
	{
		return RdCasinoCheckAct3toAct12_h.Original(tp);
	}


	if (ChkGameMode())
	{
		for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
		{
			auto p = playertwp[i];
			if (p->pos.y < -10.0f)
			{
				dsPlay_oneshot(SE_CA_OUTFALL, 0, 0, 0);

				if (!i && --Casino_BallCount || i && SubBallCount(i))
				{
					playerwk* pl_pwp = playerpwp[i];
					Angle3* ang = &playertwp[i]->ang;
					SetPositionP(i, 137.0f, 35.0f, 64.0f);
					pl_pwp->free.sb[0] &= ~2;
					ang->y = 0;
					ang->z = 0;
					ang->x = 0;
					pl_pwp->spd.z = 0.0f;
					pl_pwp->spd.y = 0.0f;
					pl_pwp->spd.x = 0.0f;
				}
				else if (GetLastPlayerAlive() != -1)
				{
					if (ulGlobalMode == MD_MISSION)
					{
						initMissionCtrl();
					}
					ADX_Close();
					ReleaseTextureOnCasino(1);
					pause_flg = 1;
					CameraReleaseEventCamera();
					SetInputP(0, PL_OP_LETITGO);
					// Don't dump in sewers if over 100 Rings.
					if (GetTotalRingsM() >= 100) 
					{
						LandChangeStage(-2);
						AddSetStage(-2);
						AddCameraStage(-2);
						AdvanceAct(-2);
						Casino_Got100Rings = 1;
					}
					else
					{
						LandChangeStage(-1);
						AddSetStage(-1);
						AddCameraStage(-1);
						AdvanceAct(-1);
					}
					
					for (int j = 0; j < multiplayer::GetPlayerCount(); j++)
					{
						SetPlayerInitialPosition(playertwp[j]);
					}
					tp->twp->mode = 0;
					break;
				}
				else
				{
					SetPositionP(i, 0.0f, 50000.0f, 0.0f);
					SetInputP(i, PL_OP_PLACEON);
				}
			}
		}
	}
	else
	{
		Casino_BallCount = BallMax;
		ResetBallCount();
	}
}

void patch_ball_init()
{
	Casino_Setup_h.Hook(Casino_Setup_r);
	RdCasinoCheckAct3toAct12_h.Hook(RdCasinoCheckAct3toAct12_r);
}

RegisterPatch patch_ball(patch_ball_init);
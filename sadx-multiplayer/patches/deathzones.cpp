#include "pch.h"
#include "deathzones.h"

void __cdecl GamePlayerMissed_r(task* tp)
{
	auto awp = tp->awp;
	auto pNum = awp->work.ul[1];

	if (++awp->work.ul[0] > 0x78)
	{
		if (!GetDebugMode() && playertp[pNum])
		{
			task* ptp = tp->ptp;

			if (GetLives_r(pNum) <= 0)
			{
				SetChangeGameMode(2);
				TempEraseSound();
			}
			else
			{
				SetLives_r(pNum, -1);
				SetPlayerInitialPosition(playertwp[pNum]);
			}
		}

		FreeTask(tp);
	}
}

void __cdecl KillPlayer_r(unsigned __int8 pNum)
{
	if (!GetDebugMode())
	{
		playertwp[pNum]->flag |= 0x1000;
		playerpwp[pNum]->item |= Powerups_Dead;
		SetInputP(pNum, 50);

		auto tp = CreateElementalTask(LoadObj_UnknownB, 0, GamePlayerMissed_r);
		tp->awp->work.ul[1] = pNum;
	}
}

void ExecFallingDownP_r(task* tp, int pNum)
{
	auto ptwp = playertwp[pNum];
	auto ppwp = playerpwp[pNum];

	auto ctp = CreateChildTask(LoadObj_UnknownB, GamePlayerMissed_r, tp);
	ctp->awp->work.ul[1] = pNum;

	CameraSetEventCameraFunc(CameraStay, 0, 0);

	if (!ppwp || !(ppwp->item & Powerups_Dead))
	{
		switch (TASKWK_CHARID(ptwp))
		{
		case Characters_Sonic:
			if (MetalSonicFlag)
			{
				PlayVoice(2046);
			}
			else
			{
				dsPlay_oneshot(1503, 0, 0, 0);
			}
			break;
		case Characters_Tails:
			dsPlay_oneshot(1465, 0, 0, 0);

			if (ptwp)
			{
				ptwp->flag |= 0x10u;
				SetInputP(pNum, 24);
			}
			break;
		case Characters_Knuckles:
			dsPlay_oneshot(1453, 0, 0, 0);

			if (ptwp)
			{
				ptwp->flag |= 0x10u;
				SetInputP(pNum, 24);
			}
			break;
		case Characters_Amy:
			dsPlay_oneshot(1396, 0, 0, 0);
			break;
		case Characters_Gamma:
			dsPlay_oneshot(1433, 0, 0, 0);
			break;
		case Characters_Big:
			dsPlay_oneshot(1412, 0, 0, 0);
			break;
		}
	}
}

void __cdecl KillPlayerFallingDownStageP_r(task* tp)
{
	LoopTaskC(tp);

	if (tp->ctp)
	{
		return;
	}

	auto dz = *KillingCollisionModelsListList[HIBYTE(GetStageNumber())];

	if (dz)
	{
		zxsdwstr carry;

		while (dz->character) {

			for (int i = 0; i < PLAYER_MAX; i++)
			{
				auto ptwp = playertwp[i];

				if (ptwp == nullptr)
				{
					continue;
				}
				
				carry.pos = ptwp->pos;

				if (GetZxShadowOnFDPolygon(&carry, dz->object))
				{
					if (!carry.lower.findflag)
					{
						if (carry.upper.findflag)
						{
							if (fabs(carry.pos.y - carry.upper.onpos) <= 30.0f)
							{
								ExecFallingDownP_r(tp, i); // also run the death cutscene
								continue;
							}
						}
					}
				}
			}

			++dz;
		}
	}
}

// Series of hacks to not reset the game if player 1 dies and make every players able to die
void init_DeathPatches()
{
	WriteJump((void*)0x440CD0, KillPlayer_r);
	WriteJump(DeathZoneHandler, KillPlayerFallingDownStageP_r); // Manage player death
}
#include "pch.h"
#include "deathzones.h"

void __cdecl GamePlayerMissedFree(task* tp)
{
	BYTEn(tp->ptp->twp->counter.l, tp->awp->work.ul[1]) = FALSE;
}

void __cdecl GamePlayerMissed_r(task* tp)
{
	auto awp = tp->awp;
	auto pNum = awp->work.ul[1];

	if (++awp->work.ul[0] > 0x78)
	{
		if (!GetDebugMode() && playertp[pNum])
		{
			if (GetLivesM(pNum) <= 0)
			{
				SetChangeGameMode(2);
				TempEraseSound();
			}
			else
			{
				SetLivesM(pNum, -1);
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

	BYTEn(tp->twp->counter.l, pNum) = TRUE;

	auto ctp = CreateChildTask(LoadObj_UnknownB, GamePlayerMissed_r, tp);
	ctp->dest = GamePlayerMissedFree;
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

	auto dz = *KillingCollisionModelsListList[HIBYTE(GetStageNumber())];

	if (dz)
	{
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			auto ptwp = playertwp[i];

			if (ptwp == nullptr || BYTEn(tp->twp->counter.l, i) == TRUE)
			{
				continue;
			}
			zxsdwstr carry;
			carry.pos = ptwp->pos;

			auto dz_ = dz;

			while (dz_->character)
			{
				if (GetZxShadowOnFDPolygon(&carry, dz_->object))
				{
					if (!carry.lower.findflag)
					{
						if (carry.upper.findflag)
						{
							if (fabs(carry.pos.y - carry.upper.onpos) <= 30.0f)
							{
								ExecFallingDownP_r(tp, i); // also run the death cutscene
								break;
							}
						}
					}
				}

				++dz_;
			}
		}
	}
}

// Series of hacks to not reset the game if player 1 dies and make every players able to die
void init_DeathPatches()
{
	WriteJump((void*)0x440CD0, KillPlayer_r);
	WriteJump(DeathZoneHandler, KillPlayerFallingDownStageP_r); // Manage player death
}
#include "pch.h"
#include "death.h"

Trampoline* KillHimP_t                    = nullptr;
Trampoline* KillHimByFallingDownP_t       = nullptr;
Trampoline* KillPlayerFallingDownStageP_t = nullptr;

void __cdecl GamePlayerMissedFree(task* tp)
{
	BYTEn(tp->ptp->twp->counter.l, tp->awp->work.ul[1]) = FALSE;
}

void __cdecl GamePlayerMissed_r(task* tp)
{
	auto awp = tp->awp;
	auto pNum = awp->work.ul[1];
	auto twp = playertwp[pNum];
	auto pwp = playerpwp[pNum];

	if (!twp)
	{
		FreeTask(tp);
		return;
	}

	pwp->item |= Powerups_Dead;

	if (++awp->work.ul[0] > 0x78)
	{
		if (!CheckEditMode() && playertp[pNum])
		{
			if (GetNumPlayerM(pNum) <= 0)
			{
				SetChangeGameMode(2);
				TempEraseSound();
			}
			else
			{
				// Remove one life
				AddNumPlayerM(pNum, -1);
				SetPlayerInitialPosition(playertwp[pNum]);

				if (CurrentLevel != LevelIDs_TwinkleCircuit)
				{
					SetInputP(pNum, 24);
					twp->mode = 1;
				}

				pwp->item &= ~Powerups_Dead;
			}
		}

		FreeTask(tp);
	}
}

void __cdecl KillHimP_r(unsigned __int8 pNum)
{
	if (multiplayer::IsActive())
	{
		if (!CheckEditMode())
		{
			playertwp[pNum]->flag |= 0x1000;
			playerpwp[pNum]->item |= Powerups_Dead;
			SetInputP(pNum, 50);

			auto tp = CreateElementalTask(LoadObj_UnknownB, 0, GamePlayerMissed_r);
			tp->awp->work.ul[1] = pNum;
		}
	}
	else
	{
		TARGET_DYNAMIC(KillHimP)(pNum);
	}
}

void ExecFallingDownP_r(int pNum)
{
	auto ptwp = playertwp[pNum];
	auto ppwp = playerpwp[pNum];

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

void __cdecl KillHimByFallingDownP_r(int pno)
{
	if (multiplayer::IsActive())
	{
		auto ctp = CreateElementalTask(LoadObj_UnknownB, LEV_0, GamePlayerMissed_r);
		ctp->awp->work.ul[1] = pno;
		ExecFallingDownP_r(pno);
	}
	else
	{
		TARGET_DYNAMIC(KillHimByFallingDownP)(pno);
	}
}

void __cdecl KillPlayerFallingDownStageP_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		TARGET_DYNAMIC(KillPlayerFallingDownStageP)(tp);
		return;
	}

	LoopTaskC(tp);

	auto dzl = KillingCollisionModelsListList[CurrentLevel];

	if (!dzl)
	{
		return;
	}

	auto dz = dzl[CurrentAct];

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
								BYTEn(tp->twp->counter.l, i) = TRUE;

								auto ctp = CreateChildTask(LoadObj_UnknownB, GamePlayerMissed_r, tp);
								ctp->dest = GamePlayerMissedFree;
								ctp->awp->work.ul[1] = i;

								ExecFallingDownP_r(i);
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
void InitDeathPatches()
{
	KillHimP_t                    = new Trampoline(0x440CD0, 0x440CD7, KillHimP_r);
	KillHimByFallingDownP_t       = new Trampoline(0x446AD0, 0x446AD7, KillHimByFallingDownP_r);
	KillPlayerFallingDownStageP_t = new Trampoline(0x44AE80, 0x44AE88, KillPlayerFallingDownStageP_r);
}
#include "pch.h"
#include "camera.h"

static void __cdecl KillHimP_r(int pNum);
static void __cdecl KillHimByFallingDownP_r(int pno);
static void __cdecl KillPlayerFallingDownStageP_r(task* tp);

static FunctionHook<void, int> KillHimP_t(KillHimP, KillHimP_r);
static FunctionHook<void, int> KillHimByFallingDownP_t(KillHimByFallingDownP, KillHimByFallingDownP_r);
TaskHook KillPlayerFallingDownStageP_t(0x44AE80, KillPlayerFallingDownStageP_r);

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
				SetChangeGameMode(1);
				TempEraseSound();
			}
			else
			{
				if (multiplayer::IsActive() || pNum == 0) AddNumPlayerM(pNum, -1); // Remove one life
				{
					CharColliOff(playertwp[pNum]);
					SetPlayerInitialPosition(playertwp[pNum]);
				}

				CameraReleaseEventCamera_m(pNum);

				// Don't reset mode if player is riding something
				if (!isPlayerInCart(pNum) && !isPlayerOnSnowBoard(pNum))
				{
					SetInputP(pNum, PL_OP_LETITGO);
					twp->mode = 1;
				}

				pwp->item &= ~Powerups_Dead;
				CharColliOn(playertwp[pNum]);
			}
		}

		FreeTask(tp);
	}
}

static void __cdecl KillHimP_r(int pNum)
{
	if (multiplayer::IsActive())
	{
		if (playerpwp[pNum] && !(playerpwp[pNum]->item & Powerups_Dead))
		{
			playertwp[pNum]->flag |= 0x1000;
			playerpwp[pNum]->item |= Powerups_Dead;
			SetInputP(pNum, PL_OP_KILLED);
			auto tp = CreateElementalTask(LoadObj_UnknownB, 0, GamePlayerMissed_r);
			tp->awp->work.ul[1] = pNum;
		}
	}
	else
	{
		KillHimP_t.Original(pNum);
	}
}

void ExecFallingDownP_r(int pNum)
{
	auto ptwp = playertwp[pNum];
	auto ppwp = playerpwp[pNum];

	CameraSetEventCameraFunc_m(pNum, CameraStay, CAMADJ_NONE, CDM_NONE);

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
				SetInputP(pNum, PL_OP_LETITGO);
			}
			break;
		case Characters_Knuckles:
			dsPlay_oneshot(1453, 0, 0, 0);

			if (ptwp)
			{
				ptwp->flag |= 0x10u;
				SetInputP(pNum, PL_OP_LETITGO);
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

static void __cdecl KillHimByFallingDownP_r(int pno)
{
	if (multiplayer::IsActive())
	{
		auto ctp = CreateElementalTask(LoadObj_UnknownB, LEV_0, GamePlayerMissed_r);
		ctp->awp->work.ul[1] = pno;
		ExecFallingDownP_r(pno);
	}
	else
	{
		KillHimByFallingDownP_t.Original(pno);
	}
}

static void __cdecl KillPlayerFallingDownStageP_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		return KillPlayerFallingDownStageP_t.Original(tp);
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

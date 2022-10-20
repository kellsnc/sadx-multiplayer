#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"
#include "result.h"

// Mess to make Speed Highway playable (especially the second act)

VoidFunc(RdHighwayManageLandMask, 0x60FEE0); // custom name

static void RdHighwayCheckArriveAtTheBuilding_w();
static void __cdecl subRd_Highway_r(task* tp);

Trampoline RdHighwayCheckArriveAtTheBuilding_t(0x610050, 0x610057, RdHighwayCheckArriveAtTheBuilding_w);
Trampoline subRd_Highway_t(0x6104C0, 0x6104C5, subRd_Highway_r);

#pragma region subRd_Highway
static void RdHighwayAct2Multi(taskwk* twp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			if (ptwp->pos.y < -18800.0f)
			{
				if (ptwp->smode != 24)
				{
					SetInputP(i, PL_OP_LETITGO);
					SetVelocityP(i, 1.4f, -2.0f, playermwp[i]->spd.z);

					if (!i) twp->mode = 5;
				}

				if (ptwp->pos.x >= 230.0f)
				{
					if (!i) twp->mode = 6;

					if (ptwp->pos.x >= 280.0f)
					{
						twp->mode = 7;
						PadReadOnP(-1);
						ChangeActM(1);
						dsPlay_iloop(961, -1, 8, 0);
						auto tp = CreateElementalTask(LoadObj_Data1, LEV_0, (TaskFuncPtr)0x61CA90);
						tp->twp->mode = MusicIDs_SpeedHighwayAtDawn;
						tp->twp->wtimer = 10;
						return;
					}
				}
			}
			else if (ptwp->pos.y < -150.0f)
			{
				if (ptwp->smode != 43)
				{
					if (CurrentSong != MusicIDs_SpeedHighwayGoinDown)
					{
						auto tp = CreateElementalTask(LoadObj_Data1, LEV_0, (TaskFuncPtr)0x61CA90);
						tp->twp->mode = MusicIDs_SpeedHighwayGoinDown;
						tp->twp->wtimer = 5;
					}

					SetInputP(i, PL_OP_BUILDING);
					PadReadOnP(i);

					if (!i) twp->mode = 4;
				}
			}
			else if (ptwp->pos.x > 0.0f)
			{
				PadReadOffP(i);
				if (!i) twp->mode = 3;
			}
		}
	}
}

static void RdHighwayCheckArriveAtTheBuilding_m(taskwk* twp)
{
	if (IsPlayerInSphere(4002.0f, -1500.0f, 4750.0f, 60.0f))
	{
		twp->mode = 2;
		ChangeActM(1);
		dsPlay_iloop(963, -1, 8, 0);
	}
}

// Custom display to manage landtable display flags for every player
static void __cdecl RdHighwayDisp(task* tp)
{
	if (multiplayer::IsActive())
	{
		RdHighwayManageLandMask();
	}
}

static void __cdecl subRd_Highway_r(task* tp)
{
	auto twp = tp->twp;

	if (multiplayer::IsActive())
	{
		switch (twp->mode)
		{
		case 0:
			RdHighwayInit(tp);
			tp->disp = RdHighwayDisp;
		case 1:

			// End point is hardcoded in Tails' Speed Highway so we must check it here
			if (CurrentCharacter == Characters_Tails)
			{
				if (twp->smode == 0)
				{
					for (int i = 0; i < PLAYER_MAX; ++i)
					{
						auto ptwp = playertwp[i];

						if (ptwp)
						{
							VecTemp0.x = 10335.0f - ptwp->pos.x;
							VecTemp0.y = -1974.0f - ptwp->pos.y;
							VecTemp0.z = 10177.0f - ptwp->pos.z;

							if (njScalor2(&VecTemp0) < 22500.0f)
							{
								SetWinnerMulti(i);
								SetFinishAction();
								twp->smode = 1;
							}
						}
					}
				}
			}
			else
			{
				RdHighwayCheckArriveAtTheBuilding_m(twp);
			}

			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			RdHighwayAct2Multi(twp);
			break;
		case 7:
			RdHighwayManageLandMask();
			break;
		}
	}
	else
	{
		TARGET_STATIC(subRd_Highway)(tp);
	}
}
#pragma endregion

#pragma region RdHighwayCheckArriveAtTheBuilding
static void RdHighwayCheckArriveAtTheBuilding_o(task* tp)
{
	auto target = RdHighwayCheckArriveAtTheBuilding_t.Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

static void __cdecl RdHighwayCheckArriveAtTheBuilding_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		RdHighwayCheckArriveAtTheBuilding_m(tp->twp);
	}
	else
	{
		RdHighwayCheckArriveAtTheBuilding_o(tp);
	}
}

static void __declspec(naked) RdHighwayCheckArriveAtTheBuilding_w()
{
	__asm
	{
		push eax
		call RdHighwayCheckArriveAtTheBuilding_r
		pop eax
		retn
	}
}
#pragma endregion
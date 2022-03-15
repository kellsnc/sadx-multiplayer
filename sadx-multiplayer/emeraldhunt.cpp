#include "pch.h"
#include "multiplayer.h"
#include "result.h"
#include "emeraldhunt.h"

Trampoline* Knuckles_KakeraGame_Set_PutEme_t = nullptr;
Trampoline* Knuckles_KakeraGameInit_t = nullptr;

static int found_feme_nmb_m[PLAYER_MAX]{};

static void __cdecl Knuckles_KakeraGameFinish_m(task* tp)
{
	auto twp = tp->twp;

	if (!isMissionClearDisp())
	{
		++twp->wtimer;
		DisablePause();

		if (twp->wtimer > 0x3Cu)
		{
			ke_ongame_flg = FALSE;
			SetFinishAction();
			FreeTask(tp);
		}
	}
}

void Knuckles_KakeraGame_Set_PutEme_m(int pnum, unsigned __int8 emeid, NJS_POINT3* emepos)
{
	if (found_feme_nmb >= 3 || ke_ongame_flg == FALSE || emeid < 0x10u || emeid > 0x4Fu)
	{
		return;
	}

	for (int i = 2; i >= 0; --i)
	{
		auto& fragm = fragmset_tbl[i];

		if (fragm.id == emeid && fragm.boutflag == 0)
		{
			fragm.boutflag = pnum + 1; // hack: add player number to that bool

			++found_feme_nmb;
			++found_feme_nmb_m[pnum];

			dsPlay_oneshot(787, 0, 0, 0);

			bool win = false;
			if (found_feme_nmb_m[pnum] >= 2)
			{
				PadReadOff();
				DisablePause();
				SleepTimer();
				SetWinnerMulti(pnum);
				CreateElementalTask(2u, LEV_0, Knuckles_KakeraGameFinish_m);
				win = true;
			}
			else if (found_feme_nmb >= 3)
			{
				PadReadOff();
				DisablePause();
				SleepTimer();
				SetWinnerMulti(-1);
				CreateElementalTask(2u, LEV_0, Knuckles_KakeraGameFinish_m);
				win = true;
			}

			auto ef_tp = CreateElementalTask(2u, 6, FragmEmeraldDigDisplay);
			if (ef_tp)
			{
				ef_tp->twp->btimer = pnum;

				if (emeid & 0x70 && playertwp[pnum])
				{
					ef_tp->twp->pos = playertwp[pnum]->pos;
				}
				else
				{
					ef_tp->twp->pos = *emepos;
				}
			}

			auto se_tp = CreateElementalTask(2u, 3, KnucklesLaterSE);
			if (se_tp)
			{
				se_tp->twp->wtimer = win ? 12 : 60;
				se_tp->twp->ang.x = found_feme_nmb + 1267;
			}

			break;
		}
	}
}

static void __cdecl Knuckles_KakeraGame_Set_PutEme_r(int emeid, NJS_POINT3* emepos)
{
	if (multiplayer::IsActive())
	{
		Knuckles_KakeraGame_Set_PutEme_m(GetTheNearestPlayerNumber(emepos), emeid, emepos);
	}
	else
	{
		TARGET_DYNAMIC(Knuckles_KakeraGame_Set_PutEme)(emeid, emepos);
	}
}

static void __cdecl Knuckles_KakeraGameInit_r()
{
	TARGET_DYNAMIC(Knuckles_KakeraGameInit)();

	for (auto& i : found_feme_nmb_m)
	{
		i = 0;
	}
}

void InitEmeraldHunt()
{
	Knuckles_KakeraGame_Set_PutEme_t = new Trampoline(0x477D90, 0x477D95, Knuckles_KakeraGame_Set_PutEme_r);
	Knuckles_KakeraGameInit_t = new Trampoline(0x475840, 0x475846, Knuckles_KakeraGameInit_r);
}
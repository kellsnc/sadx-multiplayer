#include "pch.h"
#include "multiplayer.h"
#include "result.h"
#include "hud_emerald.h"
#include "splitscreen.h"
#include "emeraldhunt.h"

Trampoline* Knuckles_KakeraGame_Set_PutEme_t = nullptr;
Trampoline* Knuckles_KakeraGameInit_t = nullptr;
Trampoline* Knuckles_KakeraGame_t = nullptr;

static int found_feme_nmb_m[PLAYER_MAX]{};
static int scales[PLAYER_MAX]{};
static int timers[PLAYER_MAX]{};
static bool is_enabled = false;

static void __cdecl Knuckles_KakeraGame_MultiDisp(task* tp)
{
	if (!MissedFrames && HideHud >= 0 && !EV_CheckCansel())
	{
		if (SplitScreen::IsActive())
		{
			SplitScreen::SaveViewPort();
			SplitScreen::ChangeViewPort(-1);
			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				if (SplitScreen::IsScreenEnabled(i))
				{
					DrawBattleEmeRadar(i, scales[i]);
				}
			}
			SplitScreen::RestoreViewPort();
		}
		else
		{
			DrawBattleEmeRadar(0, scales[0]);
		}
	}
}

static void Knuckles_KakeraGame_Timer(int pnum)
{
	auto ptwp = playertwp[pnum];
	auto& timer = timers[pnum];
	auto& scale = scales[pnum];

	float closest = 800.0f;
	int closest_id = 0;

	for (int i = 0; i < 3; ++i)
	{
		auto& fragm = fragmset_tbl[i];
		
		if (fragm.boutflag == 0)
		{
			float dist = GetDistance(&ptwp->pos, &fragm.pos);

			if (dist < closest)
			{
				closest = dist;
				closest_id = i;
			}
		}

		if (BYTEn(scale, i) > 0)
		{
			--BYTEn(scale, i);
		}
	}

	if (closest < 400.0f)
	{
		++timer;

		float interv = closest * 0.0025f;

		if (interv > 1.0f)
		{
			interv = 1.0f;
		}

		if (interv >= 0.0f)
		{
			if (interv > 0.5f)
			{
				interv = (((interv - 0.5f) * 1.4f + 0.3f) * 60.0f);
			}
			else
			{
				interv = (interv * 36.0f);
				if (interv < 9.0f)
				{
					interv = 9.0f;
				}
			}
		}
		else
		{
			interv = 9.0f;
		}

		if (timer > interv)
		{
			BYTEn(scale, closest_id) = 76ui8 - static_cast<uint8_t>(interv);
			timer = 0;
			dsPlay_oneshot(788, 0, 0, 0);
		}
	}
}

static void __cdecl Knuckles_KakeraGame_MultiExec(task* tp)
{
	if (!ke_ongame_flg)
	{
		FreeTask(tp);
		return;
	}

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (SplitScreen::IsScreenEnabled(i))
		{
			Knuckles_KakeraGame_Timer(i);
		}
	}
	
	tp->disp(tp);
}

static void __cdecl Knuckles_KaheraGame_Dest(task* tp)
{
	is_enabled = false;
}

static void __cdecl Knuckles_KakeraGame_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		if (is_enabled)
		{
			FreeTask(tp);
		}
		else
		{
			tp->dest = Knuckles_KaheraGame_Dest;
			is_enabled = true;

			for (int i = 0; i < 5; ++i)
			{
				if (fragmnmb_tbl[i].stgnmb == GetStageNumber())
				{
					tp->exec = Knuckles_KakeraGame_MultiExec;
					tp->disp = Knuckles_KakeraGame_MultiDisp;
					return;
				}
			}
		}
	}
	else
	{
		TARGET_DYNAMIC(Knuckles_KakeraGame)(tp);
	}
}

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

static void __cdecl Knuckles_KakeraGame_Set_PutEme_r(int emeid, NJS_POINT3* emepos);
void Knuckles_KakeraGame_Set_PutEme_m(int pnum, unsigned __int8 emeid, NJS_POINT3* emepos)
{
	if (!multiplayer::IsActive())
	{
		TARGET_DYNAMIC(Knuckles_KakeraGame_Set_PutEme)(emeid, emepos);
		return;
	}

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
		Knuckles_KakeraGame_Set_PutEme_m(GetClosestPlayerNum(emepos), emeid, emepos);
	}
	else
	{
		TARGET_DYNAMIC(Knuckles_KakeraGame_Set_PutEme)(emeid, emepos);
	}
}

static void __cdecl Knuckles_KakeraGameInit_r()
{
	TARGET_DYNAMIC(Knuckles_KakeraGameInit)();

	is_enabled = false;
	for (auto& i : found_feme_nmb_m)
	{
		i = 0;
	}
}

void InitEmeraldHunt()
{
	Knuckles_KakeraGame_Set_PutEme_t = new Trampoline(0x477D90, 0x477D95, Knuckles_KakeraGame_Set_PutEme_r);
	Knuckles_KakeraGameInit_t = new Trampoline(0x475840, 0x475846, Knuckles_KakeraGameInit_r);
	Knuckles_KakeraGame_t = new Trampoline(0x476440, 0x476448, Knuckles_KakeraGame_r);
}

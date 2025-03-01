#include "pch.h"
#include "SADXModLoader.h"
#include "GameText.hpp"
#include "hud_result.h"
#include "result.h"
#include "splitscreen.h"

enum MD_RESULT
{
	RESULT_IN,
	RESULT_STAT,
	RESULT_DIAL,
	RESULT_OUT,
	RESULT_DEAD
};

const char* continue_texts[]{
	"\aContinue?",
	"\aContinue?",
	"\aContinuer ?",
	"\a¿Continuar?",
	"\aContinue?"
};

const PanelPrmType PanelContinueSet[]{
	{ -114.0f, 22.5f, 9 },
	{ 114.0f, 22.5f, 4 }
};

static GameText msgc_continue;

static void DialogContinueProc(DDlgType* ddltype)
{
	msgc_continue.Draw();
}

const DialogPrmType DialogAskContinue{ DLG_PNLSTYLE_MARU, DialogContinueProc, &ava_dlg_e_TEXLIST, (PanelPrmType*)PanelContinueSet, (DlgSndPrmType*)0x7DFE08, 0x97008740, 0x97008740, 320.0f, 379.0f, 10.0f, 568.0f, 120.0f, 1.625f, 0.8f, 2, 1 };

static bool CanShowDialog()
{
	return ulGlobalMode == MD_TRIAL;
}

static void DisplayMultiResultScreeen(taskwk* twp, int pnum)
{
	auto ratio = splitscreen::GetScreenRatio(pnum);

	float screenX = HorizontalResolution * ratio->x;
	float screenY = VerticalResolution * ratio->y;
	float scaleY = VerticalStretch * ratio->h;
	float scaleX = HorizontalStretch * ratio->w;
	float scale = min(scaleX, scaleY);

	// Dim screen
	late_DrawBoxFill2D(screenX, screenY, screenX + 640.0f * scaleX, screenY + 480.0f * scaleY, 22048.0, static_cast<int>(twp->value.f * 30.0f) << 24, LATE_WZ);

	if (twp->value.f > 0.0f)
	{
		// Draw win/lose status
		Spr_MRaceDisp.tlist = &MILESRACE_TEXLIST;
		Spr_MRaceDisp.sx = Spr_MRaceDisp.sy = scale * twp->value.f;
		Spr_MRaceDisp.tanim = &TailsRace_TEXANIM;
		Spr_MRaceDisp.p.y = screenY + 200.0f * scaleY;
		Spr_MRaceDisp.p.x = screenX + 320.0f * scaleX;

		for (int i = 0; i < 4; ++i)
		{
			late_DrawSprite2D(&Spr_MRaceDisp, (GetWinnerMulti() == pnum ? 3 : 7) + i, 22046.4f, NJD_SPRITE_ALPHA, LATE_LIG);
		}
	}
}

static void __cdecl late_DisplayTotalScoreM(task* tp)
{
	if (MissedFrames)
	{
		return;
	}

	njSetTexture(&MILESRACE_TEXLIST);
	ghDefaultBlendingMode();

	if (splitscreen::IsActive())
	{
		auto pnum = splitscreen::GetCurrentScreenNum();

		if (splitscreen::IsScreenEnabled(pnum))
		{
			splitscreen::SaveViewPort();
			splitscreen::ChangeViewPort(-1);
			DisplayMultiResultScreeen(tp->twp, pnum);
			splitscreen::RestoreViewPort();
		}
	}
	else
	{
		// todo: get real player id when online
		DisplayMultiResultScreeen(tp->twp, 0);
	}
}

static void __cdecl CalcTotalScoreM_dest(task* tp)
{
	PadReadOn();
	PadReadOnP(-1);

	HideLives = 0;
	HideTimerAndRings = 0;
	HideHud = 0;

	njReleaseTexture(&MILESRACE_TEXLIST);
	njReleaseTexture(&ava_dlg_e_TEXLIST);
	njReleaseTexture(&adv_window_TEXLIST);
	njReleaseTexture(&ava_csr_TEXLIST);

	msgc_continue.Free();
}

static void __cdecl CalcTotalScoreM(task* tp)
{
	auto twp = tp->twp;

	HideLives = -1;
	HideTimerAndRings = -1;

	switch (twp->mode)
	{
	case RESULT_IN:
		twp->value.f += 0.1f;
		if (twp->value.f > 2.0f)
		{
			twp->value.f = 2.0f;
			twp->mode = RESULT_STAT;
		}
		break;
	case RESULT_STAT:
		if (++twp->timer.l > 180 || MenuSelectButtonsPressed())
		{
			if (CanShowDialog())
			{
				twp->value.f -= 0.2f;
				if (twp->value.f <= 0.0f)
				{
					twp->value.f = 0.0f;
					msgc_continue.Initialize(continue_texts[TextLanguage], 0, 335);
					*(task**)0x3B22E28 = SetDialogTask();
					OpenDialogCsrLet(&DialogAskContinue, 1, nullptr);
					twp->mode = RESULT_DIAL;
				}
			}
			else
			{
				twp->mode = RESULT_OUT;
			}
		}
		break;
	case RESULT_DIAL:
		switch (GetDialogStat())
		{
		case 0: // Continue
			GameState = MD_GAME_END;
			ssNextStageNumber = *(Sint16*)0x3B22DC4;
			ssNextActNumber = *(Sint16*)0x3B22DDC;

			twp->mode = RESULT_DEAD;
			break;
		case 1: // Back to menu
			twp->mode = RESULT_OUT;
			break;
		}
		break;
	case RESULT_OUT:
		SetChangeGameMode(1);
		twp->mode = RESULT_DEAD;
		break;
	}

	tp->disp(tp);
}

void LoadMultiplayerResult()
{
	auto tp = CreateElementalTask(LoadObj_Data1, LEV_5, CalcTotalScoreM);
	tp->disp = late_DisplayTotalScoreM;
	tp->dest = CalcTotalScoreM_dest;

	// Load "You Win!" and "You Lose" texts
	LoadPVM("MILESRACE", &MILESRACE_TEXLIST);

	if (CanShowDialog())
	{
		// Load "Yes" and "No" texts
		if (TextLanguage)
		{
			LoadPVM("ava_dlg_e", &ava_dlg_e_TEXLIST);
		}
		else
		{
			LoadPVM("ava_dlg", &ava_dlg_e_TEXLIST);
		}

		// Load dialog textures
		LoadPVM("adv_window", &adv_window_TEXLIST);
		LoadPVM("ava_csr", &ava_csr_TEXLIST);
	}
}

void LoadMultiplayerCoopResult()
{
	auto tp = CreateElementalTask(LoadObj_Data1, LEV_5, CalcTotalScore);
	tp->dest = CalcTotalScoreM_dest;
}
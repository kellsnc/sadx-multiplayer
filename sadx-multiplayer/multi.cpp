#include "pch.h"

extern bool MultiMenuEnabled;

NJS_TEXNAME multicharTex[7];
NJS_TEXLIST multichar_Texlist = { arrayptrandlength(multicharTex) };

NJS_TEXANIM MultiTexAnim[]{
	{ 128, 128, 0, 0, 0, 0, 255, 255, 0, 0x20 },
	{ 128, 128, 0, 0, 0, 0, 255, 255, 1, 0x20 },
	{ 128, 128, 0, 0, 0, 0, 255, 255, 2, 0x20 },
	{ 128, 128, 0, 0, 0, 0, 255, 255, 3, 0x20 },
	{ 128, 128, 0, 0, 0, 0, 255, 255, 4, 0x20 },
	{ 128, 128, 0, 0, 0, 0, 255, 255, 5, 0x20 },
	{ 128, 128, 0, 0, 0, 0, 255, 255, 6, 0x20 },
};

NJS_SPRITE MultiTexSprite = { { 0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim };

void __cdecl MultiMenuExec_Display(task* tp)
{
	if (MultiMenuEnabled == false)
	{
		trial_act_sel_disp(tp);
		return;
	}

	if (!MissedFrames && TrialActStelTp)
	{
		auto wk = (TrialActSelWk*)tp->awp;

		if (wk->BaseCol != 0)
		{

			SetMaterialAndSpriteColor_Float(1, 1, 1, 1);
			SetDefaultAlphaBlend();
			MultiTexSprite.p.x = 0;
			MultiTexSprite.p.y = 300;
			njDrawSprite2D_DrawNow(&MultiTexSprite, 0, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
			MultiTexSprite.p.x += 120;
			njDrawSprite2D_DrawNow(&MultiTexSprite, 1, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
			MultiTexSprite.p.x += 120;
			njDrawSprite2D_DrawNow(&MultiTexSprite, 2, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

			MultiTexSprite.p.x = 0;
			MultiTexSprite.p.y += 130;
			njDrawSprite2D_DrawNow(&MultiTexSprite, 3, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
			MultiTexSprite.p.x += 120;
			njDrawSprite2D_DrawNow(&MultiTexSprite, 4, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
			MultiTexSprite.p.x += 120;
			njDrawSprite2D_DrawNow(&MultiTexSprite, 5, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
			ClampGlobalColorThing_Thing();

		}
	}
}

bool MultiMenu_CheckMoveInput(int button, char pNum)
{
	if ((PressedButtons[pNum] & button) != 0)
	{

		switch (button)
		{
		case Buttons_Up:

			break;
		case Buttons_Down:

			break;
		case Buttons_Left:

			break;
		case Buttons_Right:

			break;
		default:
			return false;
		}


	}

	return false;
}


void MultiMenu_InputCheck(task* tp, TrialActSelWk* wk)
{
	if (MenuBackButtonsPressed())
	{
		CmnAdvaModeProcedure(ADVA_MODE_TITLE_MENU); // force back to lead to menu
		PlayMenuBackSound();
		PlayMenuBackSound();
		wk->Stat = ADVA_STAT_FADEOUT;
		wk->T = 0.0;
		return;
	}

	for (uint8_t i = 0; i < PLAYER_MAX; i++) {
		if (MultiMenu_CheckMoveInput(Buttons_Up, i))
			return;

		if (MultiMenu_CheckMoveInput(Buttons_Down, i))
			return;

		if (MultiMenu_CheckMoveInput(Buttons_Left, i))
			return;

		if (MultiMenu_CheckMoveInput(Buttons_Right, i))
			return;
	}

	if (MenuSelectButtonsPressed())
	{
		PlayMenuEnterSound();
	}

	return;
}

void __cdecl MultiMenuExec_Main(task* tp)
{
	if (MultiMenuEnabled == false)
	{
		MusicList[MusicIDs_Trial].Name = "Trial";
		trial_act_sel_exec(tp);
		return;
	}

	auto wk = (TrialActSelWk*)tp->awp;

	if (SeqTp->awp->work.ul[1] == ADVA_MODE_TRIALACT_SEL && wk->Stat == ADVA_STAT_REQWAIT)
	{
		MusicList[MusicIDs_Trial].Name = "btl_sel";
		PlayMenuMusicID(4);
		PlayVoice(40);
		LoadPVM("multichar", &multichar_Texlist);
		wk->Stat = ADVA_STAT_FADEIN;
		wk->T = 0.0f;
		wk->SelStg = -1;
	}

	if (SeqTp->awp->work.ul[2] == ADVA_MODE_TITLE_NEW && wk->Stat == ADVA_STAT_KEEP)
	{
		PlayMenuEnterSound();
	}

	switch (wk->Stat)
	{
	case ADVA_STAT_REQWAIT:
		wk->BaseCol = 0;
		wk->SelStg = -1;
		break;
	case ADVA_STAT_FADEIN:
		wk->T += MissedFrames_B * 0.025f;
		wk->BaseCol = GetFadeOutColFromT(wk->T);

		if (wk->T >= 1.0f)
		{
			wk->BaseCol = 0xFFFFFFFF;
			wk->Stat = ADVA_STAT_KEEP;
		}

		break;
	case ADVA_STAT_KEEP:
		SeqTp->awp->work.ul[3] = 3;
		MultiMenu_InputCheck(tp, wk);
		break;
	case ADVA_STAT_FADEOUT:
		wk->T += MissedFrames_B * 0.1f;
		wk->BaseCol = GetFadeInColFromT(wk->T);

		if (wk->T >= 1.0f)
		{
			wk->Stat = ADVA_STAT_REQWAIT;
			SeqTp->awp[1].work.ub[15] = 1;
			njReleaseTexture(&multichar_Texlist);
			MenuLaunchNext();
			MultiMenuEnabled = false;

			if (wk->SelStg != -1)
			{
				SeqTp->awp[1].work.sl[1] = 100;
			}
		}

		break;
	default:
		return;
	}

	tp->disp(tp);
}

void __cdecl LoadMultiMenuExec(ModeSelPrmType* prmp)
{
	auto tp = TrialActStelTp = CreateElementalTask(0, LEV_4, MultiMenuExec_Main);

	auto wk = (TrialActSelWk*)AllocateArray(1, 208);
	wk->NextMode = prmp->NextMode;
	wk->PrevMode = prmp->PrevMode;

	tp->awp = (anywk*)wk;
	tp->disp = MultiMenuExec_Display;

	ChgSubModeToStay_0(prmp, tp);
}

void init_MultiMenu()
{
	WriteJump(LevelSelect_Load, LoadMultiMenuExec);
}
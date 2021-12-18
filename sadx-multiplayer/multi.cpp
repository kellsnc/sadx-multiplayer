#include "pch.h"
#include "multi.h"

extern bool MultiMenuEnabled;

uint8_t cursor = 0;

NJS_TEXNAME multicharTex[10];
NJS_TEXLIST multichar_Texlist = { arrayptrandlength(multicharTex) };

enum CharacterMenu {
	sonicIcon,
	eggIcon,
	tailsIcon,
	knuxIcon,
	tikalIcon,
	amyIcon,
	e102Icon,
	bigIcon,
	msIcon,
	cursorIcon
};

NJS_TEXANIM MultiTexAnim[]{
	{ 64, 64, 0, 0, 0, 0, 255, 255, sonicIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, eggIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, tailsIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, knuxIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, tikalIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, amyIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, bigIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, e102Icon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, bigIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, msIcon, 0x20 },
	{ 64, 64, 0, 0, 0, 0, 255, 255, cursorIcon, 0x20 },
};

NJS_POINT2 cursorPosArray[8]{
	{ 200, 150 },
	{ 260, 150 },
	{ 320, 150 },
	{ 380, 150 },
	{ 200, 220 },
	{ 260, 220 },
	{ 320, 220 },
	{ 380, 220 },

};

NJS_SPRITE MultiTexSprite = { { 0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim };
NJS_SPRITE MultiCursorSprite = { { 0, 0, 0 }, 1.0f, 1.0f, 0, &multichar_Texlist, MultiTexAnim };

void DrawCursor() {

	MultiCursorSprite.p.x = cursorPosArray[cursor].x;
	MultiCursorSprite.p.y = cursorPosArray[cursor].y;

	njDrawSprite2D_DrawNow(&MultiCursorSprite, cursorIcon +1, -499, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
}

void __cdecl MultiMenuExec_Display(task* tp)
{
	if (MultiMenuEnabled == false)
	{
		trial_act_sel_disp(tp);
		return;
	}

	char posX = 60;
	int startposX = 200;
	char range = 4;

	if (!MissedFrames && TrialActStelTp)
	{
		auto wk = (TrialActSelWk*)tp->awp;

		if (wk->BaseCol != 0)
		{
			gHelperFunctions->PushScaleUI(uiscale::Align_Center, false, 1.0f, 1.0f);
			SetMaterialAndSpriteColor_Float(1, 1, 1, 1);
			SetDefaultAlphaBlend();
			MultiTexSprite.p.x = startposX;
			MultiTexSprite.p.y = 150;

	
			DrawCursor();

			for (uint8_t i = 0; i < 8; i++) {

				if (i == range)
				{
					MultiTexSprite.p.x = startposX;
					MultiTexSprite.p.y += 70;
				}

				njDrawSprite2D_DrawNow(&MultiTexSprite, i, -500, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
				MultiTexSprite.p.x += posX;
			}


			DrawSADXText("MULTIPLAYER", 0, 24, 120, 40);
			ClampGlobalColorThing_Thing();
			gHelperFunctions->PopScaleUI();
		}


		float a1 = wk->BaseZ - 2.0;
		DrawTiledBG_AVA_BACK(a1);
	}
}

bool MultiMenu_CheckMoveInput(int button, char pNum)
{
	if ((PressedButtons[pNum] & button) != 0)
	{

		switch (button)
		{
		case Buttons_Up:
			cursor -= 4;
			break;
		case Buttons_Down:
			cursor += 4;
			break;
		case Buttons_Left:
			cursor--;
			break;
		case Buttons_Right:
			cursor++;
			break;
		default:
			return false;
		}

		if (cursor < sonicIcon)
			cursor = bigIcon;

		if (cursor > msIcon)
			cursor = sonicIcon;

		PlayMenuBipSound();

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
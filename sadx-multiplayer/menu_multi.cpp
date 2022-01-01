#include "pch.h"
#include "menu_multi.h"
#include "multihud.h"

enum AVA_MULTI_TEX
{
	AVAMULTITEX_SONIC,
	AVAMULTITEX_EGGMAN,
	AVAMULTITEX_TAILS,
	AVAMULTITEX_KNUCKLES,
	AVAMULTITEX_TIKAL,
	AVAMULTITEX_AMY,
	AVAMULTITEX_E102,
	AVAMULTITEX_BIG,
	AVAMULTITEX_METAL,
	AVAMULTITEX_STG1,
	AVAMULTITEX_STG2,
	AVAMULTITEX_STG3,
	AVAMULTITEX_STG4,
	AVAMULTITEX_STG5,
	AVAMULTITEX_STG6,
	AVAMULTITEX_STG7,
	AVAMULTITEX_STG8,
	AVAMULTITEX_STG9,
	AVAMULTITEX_STG10,
	AVAMULTITEX_BACK,
	AVAMULTITEX_CONFIRM,
	AVAMULTITEX_SELECT,
	AVAMULTITEX_TITLE,
	AVAMULTITEX_YES,
	AVAMULTITEX_NO,
	AVAMULTITEX_CURSOR1,
	AVAMULTITEX_CURSOR2,
	AVAMULTITEX_CURSOR3,
	AVAMULTITEX_CURSOR4,
	AVAMULTITEX_CSR1,
	AVAMULTITEX_CSR2,
	AVAMULTITEX_CSR3,
	AVAMULTITEX_CSR4,
};

enum AVA_MULTI_ANM
{
	AVAMULTIANM_CHARA,
	AVAMULTIANM_CURSOR = 8,
	AVAMULTIANM_BACK,
	AVAMULTIANM_CONFIRM,
	AVAMULTIANM_SELECT,
	AVAMULTIANM_STG
};

enum MD_MULTI
{
	MD_MULTI_INITCHARSEL,
	MD_MULTI_CHARSEL,
	MD_MULTI_INITSTGSEL,
	MD_MULTI_STGSEL,
	MD_MULTI_INITSTGASK,
	MD_MULTI_STGASK,
};

NJS_TEXNAME AVA_MULTI_TEXNAME[33];
NJS_TEXLIST AVA_MULTI_TEXLIST = { arrayptrandlength(AVA_MULTI_TEXNAME) };

NJS_TEXANIM AVA_MULTI_TEXANIM[] {
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_SONIC, 0x20},
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_EGGMAN, 0x20},
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_TAILS, 0x20},
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_KNUCKLES, 0x20},
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_TIKAL, 0x20},
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_AMY, 0x20},
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_E102, 0x20},
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_BIG, 0x20},
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_CURSOR1, 0x20},
	{ 110, 32, 55, 16, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_BACK, 0x20},
	{ 120, 32, 55, 16, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_CONFIRM, 0x20},
	{ 120, 32, 55, 16, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_SELECT, 0x20},
	{ 256, 128, 128, 64, 0, 0, 255, 255, AVAMULTITEX_STG1, 0x20},
};

NJS_SPRITE AVA_MULTI_SPRITE = { { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM };

NJS_ARGB CursorColors[8] = {
	{ 1.0f, 1.0f, 1.0f, 1.0f },
	{ 1.0f, 0.145f, 0.501f, 0.894f }, //light blue
	{ 1.0f, 0.423f, 0.894f, 0.047f }, //light green
	{ 1.0f, 0.894f, 0.701f, 0.047f }, //yellow
	{ 1.0f, 0.976f, 0.525f, 0.862f }, //pink 
	{ 1.0f, 0.584f, 0.074f, 0.560f }, //purple
	{ 1.0f, 0.070f, 0.047f, 0.894f }, //dark blue
	{ 1.0f, 0.521f, 0.0f, 0.0f}, //dark red
};

NJS_POINT2 IconPosMenuMultiCharSel[] {
	 { -120.0f,	-50.0f },
	 { -40.0f,	-50.0f },
	 { 40.0f,	-50.0f },
	 { 120.0f,	-50.0f },
	 { -120.0f,	50.0f },
	 { -40.0f,	50.0f },
	 { 40.0f,	50.0f },
	 { 120.0f,	50.0f },
};

PanelPrmType PanelPrmMenuMultiStgSel[] {
	 { -150.0f,	-120.0f, AVAMULTITEX_STG1  },
	 { 0.0f,	-120.0f, AVAMULTITEX_STG2  },
	 { 150.0f,	-120.0f, AVAMULTITEX_STG3  },
	 { -150.0f,	-40.0f,  AVAMULTITEX_STG4  },
	 { 0.0f,	-40.0f,  AVAMULTITEX_STG5  },
	 { 150.0f,	-40.0f,  AVAMULTITEX_STG6  },
	 { -150.0f,	40.0f,   AVAMULTITEX_STG7  },
	 { 0.0f,	40.0f,   AVAMULTITEX_STG8  },
	 { 150.0f,	40.0f,   AVAMULTITEX_STG9  },
	 { 0.0f,	120.0f,  AVAMULTITEX_STG10 },
};

PanelPrmType PanelPrmMenuMultiStgConfirm[] {
	 { -114.0f, 25.0f, AVAMULTITEX_YES  },
	 { 114.0f, 25.0f,  AVAMULTITEX_NO }
};

void multi_menu_confirmdialog_proc(DDlgType* ddltype);

const DialogPrmType MultiMenuStageSelDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSel, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 280.0f, 20.0f, 500.0f, 340.0f, 2.0f, 1.1f, 10, 10};
const DialogPrmType MultiMenuStageConfirmDialog = { DLG_PNLSTYLE_MARU, multi_menu_confirmdialog_proc, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgConfirm, (DlgSndPrmType*)0x7DFE08, 0x97008740, 0x97008740, 320.0f, 369.0f, 10.0f, 568.0f, 140.0f, 1.625f, 0.8f, 2, 1 };

int sonic_level_link[] = {
	LevelAndActIDs_EmeraldCoast1,
	LevelAndActIDs_WindyValley1,
	LevelAndActIDs_Casinopolis2,
	LevelAndActIDs_IceCap1,
	LevelAndActIDs_TwinklePark1,
	LevelAndActIDs_SpeedHighway1,
	LevelAndActIDs_RedMountain1,
	LevelAndActIDs_SkyDeck1,
	LevelAndActIDs_LostWorld1,
	LevelAndActIDs_FinalEgg1,
	LevelAndActIDs_HotShelter1
};

const char* stg_confirm_texts[] {
	"Do you want to play this stage?"
	"Do you want to play this stage?",
	"Voulez-vous jouer à ce niveau ?",
	"Do you want to play this stage?",
	"Do you want to play this stage?"
};

const char* press_start_texts[] {
	"Press start to join",
	"Press start to join",
	"Appuyez sur entrer pour joindre",
	"Press start to join",
	"Press start to join",
};

extern bool MultiMenuEnabled;
float alpha = 1.0f;
int selected_characters[PLAYER_MAX];
bool player_ready[PLAYER_MAX];
int stgactreq;
int stgacttexid;
int pcount;
MD_MULTI prevsubmode;

void menu_multi_reset()
{
	for (auto& item : selected_characters)
	{
		item = -1;
	}
}

void menu_multi_charsel_unready()
{
	for (auto& item : player_ready)
	{
		item = false;
	}
}

void menu_multi_change(TrialActSelWk* wk, MD_MULTI id)
{
	CloseDialog();
	wk->SubMode = (TrialActSbMdEnum)id;
}

void multi_menu_request_stg(TrialActSelWk* wk, int levelact, int id)
{
	stgacttexid = ((DialogPrmType*)DialogTp->awp->work.ptr[0])->PnlPrmPtr[id].PvrIdx;
	prevsubmode = (MD_MULTI)(wk->SubMode - 1);
	menu_multi_change(wk, MD_MULTI_INITSTGASK);
	stgactreq = levelact;
}

void multi_menu_confirmdialog_proc(DDlgType* ddltype)
{
	DrawSADXText(stg_confirm_texts[TextLanguage], 315);
	
	AVA_MULTI_SPRITE.p.x = 320;
	AVA_MULTI_SPRITE.p.y = 200;
	AVA_MULTI_SPRITE.tanim[AVAMULTIANM_STG].texid = stgacttexid;
	njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_STG, -100, 0);
}

void multi_menu_stg_confirm(TrialActSelWk* wk)
{
	auto stat = GetDialogStat();

	if (stat != -1)
	{
		menu_multi_change(wk, prevsubmode);

		if (stat == 0) // chose yes
		{
			// Get splitscreen layout (TODO: create an "SetMultiplayerMode" function)
			for (int i = PLAYER_MAX - 1; i >= 0; --i)
			{
				if (selected_characters[i] != -1)
				{
					player_count = i;
					break;
				}
			}

			int level = ConvertLevelActsID_ToLevel(stgactreq);
			int act = ConvertLevelActsID_ToAct(stgactreq);

			LastLevel = CurrentLevel;
			LastAct = CurrentAct;
			CurrentCharacter = selected_characters[0];
			CurrentLevel = level;
			CurrentAct = act;
			SeqTp->awp[1].work.ul[0] = 100;
			AvaStgActT stgact = { level, act };
			AvaCmnPrm = { (uint8_t)level, (uint8_t)act };
			AdvertiseWork.Stage = level;
			AdvertiseWork.Act = act;
			wk->SelStg = level;
			wk->Stat = ADVA_STAT_FADEOUT;
			wk->T = 0.0f;
		}
	}
}

void menu_multi_charsel(TrialActSelWk* wk)
{
	bool done = true;
	pcount = 0;

	// Manage input
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto& sel = selected_characters[i];
		auto press = PressedButtons[i];

		if (sel < 0) // If player is not connected
		{
			if (press & Buttons_Start)
			{
				PlayMenuEnterSound();
				sel = 0; // player connected
			}

			continue;
		}

		pcount++;

		if (player_ready[i] == false) // Character selection
		{
			done = false;

			if (press & Buttons_Right)
			{
				sel = (sel - (sel % 4)) + ((sel + 1) % 4);
				PlayMenuBipSound();
			}
			else if (press & Buttons_Left)
			{
				sel = sel - 1 < (sel - (sel % 4)) ? (sel - (sel % 4)) + 4 - 1 : sel - 1;
				PlayMenuBipSound();
			}
			else if (press & (Buttons_Up | Buttons_Down))
			{
				sel = sel > 3 ? sel - 4 : sel + 4;
				PlayMenuBipSound();
			}
			else if (press & Buttons_Start)
			{
				player_ready[i] = true;
				PlayMenuEnterSound();
			}
		}
		else // player is ready
		{
			if (press & Buttons_B) // unready
			{
				player_ready[i] = false;
				PlayMenuBackSound();
			}
		}
	}

	// If everyone is ready and at least two players are there
	if (pcount > 1 && player_ready[0] == true && done == true)
	{
		menu_multi_change(wk, MD_MULTI_INITSTGSEL);
	}
	else if (MenuBackButtonsPressed() && player_ready[0] == false)
	{
		CmnAdvaModeProcedure(ADVA_MODE_TITLE_MENU);
		wk->Stat = ADVA_STAT_FADEOUT;
	}
}

void menu_multi_stgsel(TrialActSelWk* wk)
{
	auto stat = GetDialogStat();

	if (stat == 10) // go back request
	{
		menu_multi_change(wk, MD_MULTI_INITCHARSEL);
	}
	else if (stat != -1) // launch game request
	{
		multi_menu_request_stg(wk, sonic_level_link[stat], stat);
	}
}

void menu_multi_subexec(TrialActSelWk* wk)
{
	switch (wk->SubMode)
	{
	case MD_MULTI_INITCHARSEL: // Open character select
		menu_multi_change(wk, MD_MULTI_CHARSEL);
		menu_multi_charsel_unready();
		break;
	case MD_MULTI_CHARSEL:
		menu_multi_charsel(wk);
		break;
	case MD_MULTI_INITSTGSEL: // Open stage select (only Sonic for now)
		ava_csr_TEXLIST.textures[0] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CURSOR1];
		ava_csr_TEXLIST.textures[1] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CURSOR2];
		ava_csr_TEXLIST.textures[2] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CURSOR3];
		ava_csr_TEXLIST.textures[3] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CURSOR4];

		menu_multi_change(wk, MD_MULTI_STGSEL);
		OpenDialog(&MultiMenuStageSelDialog);
		break;
	case MD_MULTI_STGSEL:
		menu_multi_stgsel(wk);
		break;
	case MD_MULTI_INITSTGASK: // Open prompt to ask level confirmation
		ava_csr_TEXLIST.textures[0] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CSR1];
		ava_csr_TEXLIST.textures[1] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CSR2];
		ava_csr_TEXLIST.textures[2] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CSR3];
		ava_csr_TEXLIST.textures[3] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CSR4];

		menu_multi_change(wk, MD_MULTI_STGASK);
		OpenDialog(&MultiMenuStageConfirmDialog);
		break;
	case MD_MULTI_STGASK:
		multi_menu_stg_confirm(wk);
		break;
	}
}

void multi_menu_disp_controls(TrialActSelWk* wk)
{
	// Do not show on level select screen to save space:
	
	float color = min(1.0f, alpha);

	SetMaterial(color, color, color, color);

	gHelperFunctions->PushScaleUI((uiscale::Align)(Align_Bottom | Align_Center_Horizontal), false, 1.0f, 1.0f);

	AVA_MULTI_SPRITE.p.y = 448.0f;

	AVA_MULTI_SPRITE.p.x = 165.0f;
	njDrawSprite2D_DrawNow(&AVA_MULTI_SPRITE , AVAMULTIANM_SELECT, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	AVA_MULTI_SPRITE.p.x = 330.0f;
	njDrawSprite2D_DrawNow(&AVA_MULTI_SPRITE , AVAMULTIANM_CONFIRM, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	AVA_MULTI_SPRITE.p.x = 495.0f;
	njDrawSprite2D_DrawNow(&AVA_MULTI_SPRITE , AVAMULTIANM_BACK, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	gHelperFunctions->PopScaleUI();
}

void multi_menu_disp_charsel(TrialActSelWk* wk)
{
	if (wk->SubMode <= MD_MULTI_CHARSEL)
	{
		if (alpha < 1.0f) alpha += 0.05f;
	}
	else
	{
		if (alpha > 0.0f) alpha -= 0.05f;
	}

	if (alpha <= 0.0f)
	{
		return;
	}

	alpha = min(1.0f, alpha);

	// Draw window
	ghSetPvrTexVertexColor(0x78002E67u, 0x78117BFFu, 0x78002E67u, 0x78117BFFu);
	DrawShadowWindow(120.0f, 110.0f, wk->BaseZ - 6.0f, 400.0f, 300.0f);

	SetMaterial(alpha, alpha, alpha, alpha);

	if (pcount > 0)
	{
		// Draw character icons
		for (int i = 0; i < 8; ++i)
		{
			AVA_MULTI_SPRITE.p.x = 320.0f + IconPosMenuMultiCharSel[i].x;
			AVA_MULTI_SPRITE.p.y = 260.0f + IconPosMenuMultiCharSel[i].y;
			AVA_MULTI_SPRITE.p.z = wk->BaseZ - 8;
			njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_CHARA + i, wk->BaseZ - 8, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

			// Draw cursor
			for (int p = PLAYER_MAX - 1; p >= 0; --p)
			{
				if (i == selected_characters[p])
				{
					CursorColors[p].a = 0.75f + 0.25 * njSin(FrameCounter * 1000);
					___njSetConstantMaterial(&CursorColors[p]);
					njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_CURSOR, wk->BaseZ + 100, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
					SetMaterial(alpha, alpha, alpha, alpha);
				}
			}
		}
	}
	else
	{
		DrawWaitingForPlayer(140, 230);
		DrawSADXText(press_start_texts[TextLanguage], 280);
	}
	
	// Draw controls
	multi_menu_disp_controls(wk);
}

void __cdecl MultiMenuExec_Display(task* tp)
{
	if (MultiMenuEnabled == false)
	{
		trial_act_sel_disp(tp);
		return;
	}

	auto wk = (TrialActSelWk*)tp->awp;
	
	if (wk->Stat != ADVA_STAT_REQWAIT)
	{
		gHelperFunctions->PushScaleUI(uiscale::Align_Center, true, 1.0f, 1.0f);

		ghDefaultBlendingMode();
		ghSetPvrTexBaseColor(wk->BaseCol);
		DrawSkyBg(wk->BaseZ - 2.0f);

		// Draw header
		ghSetPvrTexMaterial(0xFFFFFFFF);
		DrawTitleBack(0.0f, 40.0f, wk->BaseZ - 8, 565.0, 42.0);
		njSetTexture(&AVA_MULTI_TEXLIST);
		ghDrawPvrTexture(AVAMULTITEX_TITLE, 40.0f, 47.0f, wk->BaseZ - 18);

		multi_menu_disp_charsel(wk);

		ResetMaterial();
		gHelperFunctions->PopScaleUI();
	}
}

void __cdecl MultiMenuExec_Main(task* tp)
{
	if (MultiMenuEnabled == false)
	{
		trial_act_sel_exec(tp);
		return;
	}

	auto wk = (TrialActSelWk*)tp->awp;

	// Check if our menu is ready
	if (SeqTp->awp->work.ul[1] == ADVA_MODE_TRIALACT_SEL && wk->Stat == ADVA_STAT_REQWAIT)
	{
		menu_multi_reset();
		PlayMenuMusicID(MusicIDs_JingleE);
		LoadPVM("AVA_MULTI", &AVA_MULTI_TEXLIST);
		LoadPVM("CON_MULTI", &CON_MULTI_TEXLIST);
		wk->Stat = ADVA_STAT_FADEIN;
		alpha = 1.0f;
		wk->T = 0.0f;
		wk->SelStg = -1;
	}

	// Check if our menu has to change
	if (SeqTp->awp->work.ul[2] == ADVA_MODE_TITLE_NEW && wk->Stat == ADVA_STAT_KEEP)
	{
		PlayMenuEnterSound();
		wk->Stat = ADVA_STAT_FADEOUT;
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
		menu_multi_subexec(wk); // main menu logic
		break;
	case ADVA_STAT_FADEOUT:
		wk->T += MissedFrames_B * 0.1f;
		wk->BaseCol = GetFadeInColFromT(wk->T);

		if (wk->T >= 1.0f)
		{
			wk->Stat = ADVA_STAT_REQWAIT;

			SeqTp->awp[1].work.ub[15] = 1;
			MenuLaunchNext();

			MultiMenuEnabled = false;
			njReleaseTexture(&AVA_MULTI_TEXLIST);
			njReleaseTexture(&CON_MULTI_TEXLIST);

			// Force stage mode:
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

	wk->BaseZ = -10000.0f;

	ChgSubModeToStay_0(prmp, tp);
}

void init_MultiMenu()
{
	WriteJump(LevelSelect_Load, LoadMultiMenuExec);
}
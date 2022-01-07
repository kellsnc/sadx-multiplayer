#include "pch.h"
#include <utility>
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
	AVAMULTITEX_BACK,
	AVAMULTITEX_CONFIRM,
	AVAMULTITEX_SELECT,
	AVAMULTITEX_TITLE,
	AVAMULTITEX_ACT1,
	AVAMULTITEX_ACT2,
	AVAMULTITEX_ACT3,
	AVAMULTITEX_YES,
	AVAMULTITEX_NO,
	AVAMULTITEX_CURSORBG,
	AVAMULTITEX_CURSOR1,
	AVAMULTITEX_CURSOR2,
	AVAMULTITEX_CSR1,
	AVAMULTITEX_CSR2,
	AVAMULTITEX_CSR3,
	AVAMULTITEX_CSR4,
	AVAMULTITEX_MD_SPD,
	AVAMULTITEX_MD_EME,
	AVAMULTITEX_MD_TC,
	AVAMULTITEX_MD_FISH,
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
	AVAMULTITEX_STG11,
	AVAMULTITEX_STGTC1,
	AVAMULTITEX_STGTC2,
	AVAMULTITEX_STGTC3,
	AVAMULTITEX_STGTC4,
	AVAMULTITEX_STGTC5,
	AVAMULTITEX_STGTC6,
	AVAMULTITEX_STGBIG1,
	AVAMULTITEX_STGBIG2,
	AVAMULTITEX_STGBIG3,
	AVAMULTITEX_STGBIG4,
	AVAMULTITEX_STGEME1,
	AVAMULTITEX_STGEME2,
	AVAMULTITEX_STGEME3,
	AVAMULTITEX_STGEME4,
	AVAMULTITEX_STGEME5,
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
	MD_MULTI_INITMODESEL,
	MD_MULTI_MODESEL,
	MD_MULTI_INITSTGSEL_EME,
	MD_MULTI_STGSEL_EME,
	MD_MULTI_INITSTGSEL_TC,
	MD_MULTI_STGSEL_TC,
	MD_MULTI_INITSTGSEL_FISH,
	MD_MULTI_STGSEL_FISH,
	MD_MULTI_INITSTGSEL_SNC,
	MD_MULTI_STGSEL_SNC,
	MD_MULTI_INITSTGASK,
	MD_MULTI_STGASK,
};

struct MultiMenuWK
{
	AdvaStatEnum Stat;
	AdvaModeEnum PrevMode;
	AdvaModeEnum NextMode;
	float BaseZ;
	float T;
	unsigned int BaseCol;
	MD_MULTI SubMode;
	int SelStg;
	float alphaMainMenu;
	float alphaControls;
	int stgreq;
	int actreq;
};

NJS_TEXNAME AVA_MULTI_TEXNAME[44];
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
	{ 64, 64, 32, 32, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_CURSOR2, 0x20},
	{ 120, 32, 0, 0, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_BACK, 0x20},
	{ 120, 32, 0, 0, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_CONFIRM, 0x20},
	{ 120, 32, 0, 0, 0, 0, 0x0FF, 0x0FF, AVAMULTITEX_SELECT, 0x20},
	{ 256, 128, 128, 64, 0, 0, 255, 255, AVAMULTITEX_STG1, 0x20},
};

NJS_SPRITE AVA_MULTI_SPRITE = { { 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 0, &AVA_MULTI_TEXLIST, AVA_MULTI_TEXANIM };

NJS_ARGB CursorColors[8] = {
	{ 1.0f, 0.145f, 0.501f, 0.894f }, // light blue
	{ 1.0f, 0.97f, 0.07f, 0.07f },    // light red
	{ 1.0f, 0.423f, 0.894f, 0.047f }, //light green
	{ 1.0f, 0.894f, 0.701f, 0.047f }, //yellow
	{ 1.0f, 0.976f, 0.525f, 0.862f }, //pink 
	{ 1.0f, 0.584f, 0.074f, 0.560f }, //purple
	{ 1.0f, 0.070f, 0.047f, 0.894f }, //dark blue
	{ 1.0f, 0.521f, 0.0f, 0.0f},      //dark red
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

PanelPrmType PanelPrmMenuMultiModSel[] {
	 { -180.0f,	8.0f, AVAMULTITEX_MD_SPD   },
	 { -60.0f,	8.0f, AVAMULTITEX_MD_EME   },
	 { 60.0f,	8.0f, AVAMULTITEX_MD_TC    },
	 { 180.0f,	8.0f,  AVAMULTITEX_MD_FISH },
};

PanelPrmType PanelPrmMenuMultiStgSelEme[]{
	 { -150.0f,	-40.0f, AVAMULTITEX_STGEME1 },
	 { 0.0f,	-40.0f, AVAMULTITEX_STGEME2 },
	 { 150.0f,	-40.0f, AVAMULTITEX_STGEME3 },
	 { -75.0f,	40.0f,  AVAMULTITEX_STGEME4 },
	 { 75.0f,	40.0f,  AVAMULTITEX_STGEME5 }
};

PanelPrmType PanelPrmMenuMultiStgSelBig[]{
	 { -100.0f,	-65.0f, AVAMULTITEX_STGBIG1 },
	 { 100.0f,	-65.0f, AVAMULTITEX_STGBIG2 },
	 { -100.0f,	65.0f,  AVAMULTITEX_STGBIG3 },
	 { 100.0f,	65.0f,  AVAMULTITEX_STGBIG4 }
};

PanelPrmType PanelPrmMenuMultiStgSelSonic[]{
	 { -150.0f,	-120.0f, AVAMULTITEX_STG1  },
	 { 0.0f,	-120.0f, AVAMULTITEX_STG2  },
	 { 150.0f,	-120.0f, AVAMULTITEX_STG3  },
	 { -150.0f,	-40.0f,  AVAMULTITEX_STG4  },
	 { 0.0f,	-40.0f,  AVAMULTITEX_STG5  },
	 { 150.0f,	-40.0f,  AVAMULTITEX_STG6  },
	 { -150.0f,	40.0f,   AVAMULTITEX_STG7  },
	 { 0.0f,	40.0f,   AVAMULTITEX_STG8  },
	 { 150.0f,	40.0f,   AVAMULTITEX_STG9  },
	 { -75.0f,	120.0f,  AVAMULTITEX_STG10 },
	 { 75.0f,	120.0f,  AVAMULTITEX_STG11 },
};

PanelPrmType PanelPrmMenuMultiStgSelTwinkle[]{
	 { -150.0f,	-40.0f, AVAMULTITEX_STGTC1 },
	 { 0.0f,	-40.0f, AVAMULTITEX_STGTC2 },
	 { 150.0f,	-40.0f, AVAMULTITEX_STGTC3 },
	 { -150.0f,	40.0f,  AVAMULTITEX_STGTC4 },
	 { 0.0f,	40.0f,  AVAMULTITEX_STGTC5 },
	 { 150.0f,	40.0f,  AVAMULTITEX_STGTC6 }
};

PanelPrmType PanelPrmMenuMultiStgConfirm[4] {};

void multi_menu_confirmdialog_proc(DDlgType* ddltype);

const DialogPrmType MultiMenuModeSelDialog = { DLG_PNLSTYLE_MARU2, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiModSel, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 20.0f, 500.0f, 180.0f, 1.2f, 1.2f, LengthOfArray(PanelPrmMenuMultiModSel), 4};
const DialogPrmType MultiMenuStageSelBigDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelBig, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 260.0f, 20.0f, 500.0f, 290.0f, 3.0f, 1.7f, LengthOfArray(PanelPrmMenuMultiStgSelBig), 4};
const DialogPrmType MultiMenuStageSelSonicDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelSonic, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 280.0f, 20.0f, 500.0f, 340.0f, 2.1f, 1.2f, LengthOfArray(PanelPrmMenuMultiStgSelSonic), 11};
const DialogPrmType MultiMenuStageSelTwinkleDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelTwinkle, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 20.0f, 500.0f, 200.0f, 2.1f, 1.2f, LengthOfArray(PanelPrmMenuMultiStgSelTwinkle), 6};
const DialogPrmType MultiMenuStageSelEmeDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelEme, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 20.0f, 500.0f, 200.0f, 2.1f, 1.2f, LengthOfArray(PanelPrmMenuMultiStgSelEme), 5};
DialogPrmType MultiMenuStageConfirmDialog = { DLG_PNLSTYLE_MARU, multi_menu_confirmdialog_proc, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgConfirm, (DlgSndPrmType*)0x7DFE08, 0x97008740, 0x97008740, 320.0f, 369.0f, 10.0f, 568.0f, 140.0f, 1.625f, 0.8f, 4, 3 };

std::pair<int, int> sonic_level_link[] = {
	{ LevelAndActIDs_EmeraldCoast1, 2 },
	{ LevelAndActIDs_WindyValley1, 3 },
	{ LevelAndActIDs_Casinopolis2, 1 },
	{ LevelAndActIDs_IceCap2, 2 },
	{ LevelAndActIDs_TwinklePark1, 3 },
	{ LevelAndActIDs_SpeedHighway1, 3 },
	{ LevelAndActIDs_RedMountain1, 2 },
	{ LevelAndActIDs_SkyDeck1, 3 },
	{ LevelAndActIDs_LostWorld1, 2 },
	{ LevelAndActIDs_FinalEgg1, 3 },
	{ LevelAndActIDs_HotShelter1, 2 }
};

int twinkle_level_link[]{
	LevelAndActIDs_TwinkleCircuit1,
	LevelAndActIDs_TwinkleCircuit2,
	LevelAndActIDs_TwinkleCircuit3,
	LevelAndActIDs_TwinkleCircuit4,
	LevelAndActIDs_TwinkleCircuit5,
	LevelAndActIDs_TwinkleCircuit6
};

int big_level_link[]{
	LevelAndActIDs_TwinklePark2,
	LevelAndActIDs_IceCap2,
	LevelAndActIDs_EmeraldCoast3,
	LevelAndActIDs_HotShelter1
};

int eme_level_link[]{
	LevelAndActIDs_SpeedHighway3,
	LevelAndActIDs_Casinopolis1,
	LevelAndActIDs_RedMountain3,
	LevelAndActIDs_LostWorld2,
	LevelAndActIDs_SkyDeck3
};

int charsel_voicelist[] {
	1098,
	569,
	528,
	392,
	921,
	510,
	394,
	1346,
	2047
};

AvaTexLdEnum AvaTexLdListForMulti[]{
	TENUM_AVA_BACK, TENUM_ADV_WINDOW, TENUM_NMAX_SADXPC
};

const char* stg_confirm_texts[] {
	"Do you want to play this stage?"
	"Do you want to play this stage?",
	"Voulez-vous jouer à ce niveau ?",
	"¿Quieres jugar este nivel?",
	"Do you want to play this stage?"
};

const char* press_start_texts[] {
	"Press start to join",
	"Press start to join",
	"Appuyez sur entrer pour joindre",
	"Presiona start para unirte",
	"Press start to join"
};

int stgacttexid = 0;
int selected_characters[PLAYER_MAX];
bool player_ready[PLAYER_MAX];
int pcount;
bool enabled_characters[8];
MD_MULTI saved_mode;

void menu_multi_reset()
{
	pcount = 0;

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

int menu_multi_getplayerno(int num)
{
	if (num == 6)
	{
		num = Characters_Tikal;
	}
	else if (num == 7)
	{
		num = Characters_Eggman;
	}
	else if (num >= 3)
	{
		num += 2;
	}
	else if (num >= 1)
	{
		num += 1;
	}

	return num;
}

void menu_multi_change(MultiMenuWK* wk, MD_MULTI id)
{
	CloseDialog();
	wk->SubMode = id;
}

void menu_multi_launch_level(MultiMenuWK* wk, int act)
{
	// Enable multiplayer mode
	multiplayer::Enable(pcount);

	for (int i = 0; i < pcount; ++i)
	{
		SetCurrentCharacter(i, menu_multi_getplayerno(selected_characters[i]));
	}

	// Force trial return to this menu instead of charsel
	WriteData((int*)0x7EEB58, (int)ADVA_MODE_MULTI);

	auto level = wk->stgreq;

	LastLevel = CurrentLevel;
	LastAct = CurrentAct;
	CurrentCharacter = menu_multi_getplayerno(selected_characters[0]);
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

void multi_menu_request_stg(MultiMenuWK* wk, int level, int actcnt, int item)
{
	auto act = ConvertLevelActsID_ToAct(level);

	MultiMenuStageConfirmDialog.CsrMax = actcnt + 1;
	MultiMenuStageConfirmDialog.CsrCancel = actcnt;

	if (actcnt == 1)
	{
		PanelPrmMenuMultiStgConfirm[0] = { -114.0f, 25.0f, AVAMULTITEX_YES };
		PanelPrmMenuMultiStgConfirm[1] = { 114.0f, 25.0f, AVAMULTITEX_NO };
	}
	else if (actcnt == 2)
	{
		PanelPrmMenuMultiStgConfirm[0] = { -144.0f, 25.0f, (uint8_t)(AVAMULTITEX_ACT1 + act) };
		PanelPrmMenuMultiStgConfirm[1] = { 0.0f, 25.0f, (uint8_t)(AVAMULTITEX_ACT2 + act) };
		PanelPrmMenuMultiStgConfirm[2] = { 144.0f, 25.0f, AVAMULTITEX_NO };
	}
	else if (actcnt == 3)
	{
		PanelPrmMenuMultiStgConfirm[0] = { -177.0f, 25.0f, AVAMULTITEX_ACT1 };
		PanelPrmMenuMultiStgConfirm[1] = { -60.0f, 25.0f, AVAMULTITEX_ACT2 };
		PanelPrmMenuMultiStgConfirm[2] = { 60.0f, 25.0f, AVAMULTITEX_ACT3 };
		PanelPrmMenuMultiStgConfirm[3] = { 177.0f, 25.0f, AVAMULTITEX_NO };
	}

	stgacttexid = ((DialogPrmType*)DialogTp->awp->work.ptr[0])->PnlPrmPtr[item].PvrIdx;
	menu_multi_change(wk, MD_MULTI_INITSTGASK);
	wk->stgreq = ConvertLevelActsID_ToLevel(level);
	wk->actreq = ConvertLevelActsID_ToAct(level);
}

void multi_menu_confirmdialog_proc(DDlgType* ddltype)
{
	DrawSADXText(stg_confirm_texts[TextLanguage], 315);
	
	AVA_MULTI_SPRITE.p.x = 320;
	AVA_MULTI_SPRITE.p.y = 200;
	AVA_MULTI_SPRITE.tanim[AVAMULTIANM_STG].texid = stgacttexid;
	njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_STG, -100, NJD_SPRITE_ALPHA);
}

void multi_menu_stg_confirm(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	if (stat != -1)
	{
		menu_multi_change(wk, saved_mode);

		if (stat != MultiMenuStageConfirmDialog.CsrCancel)
		{
			int act = PanelPrmMenuMultiStgConfirm[stat].PvrIdx;

			if (act == AVAMULTITEX_YES)
			{
				act = wk->actreq;
			}
			else
			{
				act = wk->actreq + act - AVAMULTITEX_ACT1;
			}

			menu_multi_launch_level(wk, act);
		}
	}
}

void menu_multi_stgsel_snc(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	if (stat == MultiMenuStageSelSonicDialog.CsrCancel) // go back request
	{
		menu_multi_change(wk, MD_MULTI_INITMODESEL);
	}
	else if (stat != -1) // launch game request
	{
		multi_menu_request_stg(wk, sonic_level_link[stat].first, sonic_level_link[stat].second, stat);
	}
}

void menu_multi_stgsel_twinkle(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	if (stat == MultiMenuStageSelTwinkleDialog.CsrCancel) // go back request
	{
		menu_multi_change(wk, MD_MULTI_INITMODESEL);
	}
	else if (stat != -1) // launch game request
	{
		multi_menu_request_stg(wk, twinkle_level_link[stat], 1, stat);
	}
}

void menu_multi_stgsel_big(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	if (stat == MultiMenuStageSelBigDialog.CsrCancel) // go back request
	{
		menu_multi_change(wk, MD_MULTI_INITMODESEL);
	}
	else if (stat != -1) // launch game request
	{
		multi_menu_request_stg(wk, big_level_link[stat], 1, stat);
	}
}

void menu_multi_stgsel_eme(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	if (stat == MultiMenuStageSelEmeDialog.CsrCancel) // go back request
	{
		menu_multi_change(wk, MD_MULTI_INITMODESEL);
	}
	else if (stat != -1) // launch game request
	{
		multi_menu_request_stg(wk, eme_level_link[stat], 1, stat);
	}
}

void menu_multi_modesel(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	switch (stat)
	{
	case 0:
		menu_multi_change(wk, MD_MULTI_INITSTGSEL_SNC);
		break;
	case 1:
		menu_multi_change(wk, MD_MULTI_INITSTGSEL_EME);
		break;
	case 2:
		menu_multi_change(wk, MD_MULTI_INITSTGSEL_TC);
		break;
	case 3:
		menu_multi_change(wk, MD_MULTI_INITSTGSEL_FISH);
		break;
	case 4:
		menu_multi_change(wk, MD_MULTI_INITCHARSEL);
		break;
	}
}

void menu_multi_charsel(MultiMenuWK* wk)
{
	bool done = true;
	pcount = 0;

	// Return to main menu
	if (MenuBackButtonsPressed() && player_ready[0] == false)
	{
		CmnAdvaModeProcedure(ADVA_MODE_TITLE_MENU);
		wk->T = 0.0f;
		wk->Stat = ADVA_STAT_FADEOUT;
	}

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
			bool oneshot = false;

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

			while (enabled_characters[sel] == false)
			{
				if (press & Buttons_Left)
				{
					sel = abs(sel % 8 - 1);
				}
				else
				{
					sel = abs(sel % 8 + 1);
				}
			}

			if (press & Buttons_Start)
			{
				player_ready[i] = true;
				PlayVoice(charsel_voicelist[sel]);
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

	// If everyone is ready and at least two players are there (including player 1)
	if (pcount > 1 && player_ready[0] == true && done == true)
	{
		menu_multi_change(wk, MD_MULTI_INITMODESEL);
	}
}

void menu_multi_setrndcursor()
{
	ava_csr_TEXLIST.textures[0] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CSR1];
	ava_csr_TEXLIST.textures[1] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CSR2];
	ava_csr_TEXLIST.textures[2] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CSR3];
	ava_csr_TEXLIST.textures[3] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CSR4];
}

void menu_multi_setsqrcursor()
{
	ava_csr_TEXLIST.textures[0] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CURSORBG];
	ava_csr_TEXLIST.textures[1] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CURSORBG];
	ava_csr_TEXLIST.textures[2] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CURSORBG];
	ava_csr_TEXLIST.textures[3] = AVA_MULTI_TEXLIST.textures[AVAMULTITEX_CURSOR1];
}

void menu_multi_getcharaenable()
{
	for (int i = 0; i < LengthOfArray(enabled_characters); ++i)
	{
		if (i < 6)
		{
			enabled_characters[i] = GetCharacterUnlockedFlag(i) == TRUE;
		}
		else if (i == Characters_Eggman)
		{
			enabled_characters[i] = false;
		}
		else if (i == Characters_Tikal)
		{
			enabled_characters[i] = false;
		}
	}
}

void menu_multi_subexec(MultiMenuWK* wk)
{
	switch (wk->SubMode)
	{
	case MD_MULTI_INITCHARSEL: // Open character select
		menu_multi_change(wk, MD_MULTI_CHARSEL);
		menu_multi_charsel_unready();
		menu_multi_getcharaenable();
		saved_mode = MD_MULTI_INITCHARSEL;
		break;
	case MD_MULTI_CHARSEL:
		menu_multi_charsel(wk);
		break;
	case MD_MULTI_INITMODESEL:
		menu_multi_setrndcursor();
		menu_multi_change(wk, MD_MULTI_MODESEL);
		saved_mode = MD_MULTI_INITMODESEL;
		OpenDialog(&MultiMenuModeSelDialog);
		break;
	case MD_MULTI_MODESEL:
		menu_multi_modesel(wk);
		break;
	case MD_MULTI_INITSTGSEL_EME:
		menu_multi_setsqrcursor();
		menu_multi_change(wk, MD_MULTI_STGSEL_EME);
		saved_mode = MD_MULTI_INITSTGSEL_EME;
		OpenDialog(&MultiMenuStageSelEmeDialog);
		break;
	case MD_MULTI_STGSEL_EME:
		menu_multi_stgsel_eme(wk);
		break;
	case MD_MULTI_INITSTGSEL_TC:
		menu_multi_setsqrcursor();
		menu_multi_change(wk, MD_MULTI_STGSEL_TC);
		saved_mode = MD_MULTI_INITSTGSEL_TC;
		OpenDialog(&MultiMenuStageSelTwinkleDialog);
		break;
	case MD_MULTI_STGSEL_TC:
		menu_multi_stgsel_twinkle(wk);
		break;
	case MD_MULTI_INITSTGSEL_FISH:
		menu_multi_setsqrcursor();
		menu_multi_change(wk, MD_MULTI_STGSEL_FISH);
		saved_mode = MD_MULTI_INITSTGSEL_FISH;
		OpenDialog(&MultiMenuStageSelBigDialog);
		break;
	case MD_MULTI_STGSEL_FISH:
		menu_multi_stgsel_big(wk);
		break;
	case MD_MULTI_INITSTGSEL_SNC: // Open stage select (only Sonic for now)
		menu_multi_setsqrcursor();
		menu_multi_change(wk, MD_MULTI_STGSEL_SNC);
		saved_mode = MD_MULTI_INITSTGSEL_SNC;
		OpenDialog(&MultiMenuStageSelSonicDialog);
		break;
	case MD_MULTI_STGSEL_SNC:
		menu_multi_stgsel_snc(wk);
		break;
	case MD_MULTI_INITSTGASK: // Open prompt to ask level confirmation
		menu_multi_setrndcursor();
		menu_multi_change(wk, MD_MULTI_STGASK);
		OpenDialog(&MultiMenuStageConfirmDialog);
		break;
	case MD_MULTI_STGASK:
		multi_menu_stg_confirm(wk);
		break;
	}
}

void multi_menu_disp_controls(MultiMenuWK* wk)
{
	if (wk->SubMode < MD_MULTI_STGSEL_SNC && wk->Stat != ADVA_STAT_FADEOUT)
	{
		if (wk->alphaControls < 1.0f) wk->alphaControls += 0.05f;
	}
	else
	{
		if (wk->alphaControls > 0.0f) wk->alphaControls -= 0.05f;
	}

	if (wk->alphaControls <= 0.0f)
	{
		return;
	}

	wk->alphaControls = min(1.0f, wk->alphaControls);

	SetMaterial(wk->alphaControls, 1.0f, 1.0f, 1.0f);

	gHelperFunctions->PushScaleUI((uiscale::Align)(Align_Bottom | Align_Center_Horizontal), false, 1.0f, 1.0f);

	AVA_MULTI_SPRITE.p.y = 432.0f;

	AVA_MULTI_SPRITE.p.x = 135.0f;
	njDrawSprite2D_DrawNow(&AVA_MULTI_SPRITE , AVAMULTIANM_SELECT, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	AVA_MULTI_SPRITE.p.x += 130.0f;
	njDrawSprite2D_DrawNow(&AVA_MULTI_SPRITE , AVAMULTIANM_CONFIRM, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	AVA_MULTI_SPRITE.p.x += 150.0f;
	njDrawSprite2D_DrawNow(&AVA_MULTI_SPRITE , AVAMULTIANM_BACK, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	gHelperFunctions->PopScaleUI();
}

void multi_menu_disp_charsel(MultiMenuWK* wk)
{
	if (wk->SubMode <= MD_MULTI_CHARSEL && wk->Stat != ADVA_STAT_FADEOUT)
	{
		if (wk->alphaMainMenu < 1.0f) wk->alphaMainMenu += 0.05f;
	}
	else
	{
		if (wk->alphaMainMenu > 0.0f) wk->alphaMainMenu -= 0.05f;
	}

	if (wk->alphaMainMenu <= 0.0f)
	{
		return;
	}

	wk->alphaMainMenu = min(1.0f, wk->alphaMainMenu);

	// Draw window
	ghSetPvrTexVertexColor(0x78002E67u, 0x78117BFFu, 0x78002E67u, 0x78117BFFu);
	DrawShadowWindow(120.0f, 110.0f, wk->BaseZ - 6.0f, 400.0f, 300.0f);

	SetMaterial(wk->alphaMainMenu, 1.0f, 1.0f, 1.0f);

	if (pcount > 0)
	{
		// Draw character icons
		for (int i = 0; i < 8; ++i)
		{
			AVA_MULTI_SPRITE.p.x = 320.0f + IconPosMenuMultiCharSel[i].x;
			AVA_MULTI_SPRITE.p.y = 260.0f + IconPosMenuMultiCharSel[i].y;
			AVA_MULTI_SPRITE.p.z = wk->BaseZ - 8;

			if (enabled_characters[i] == true)
			{
				njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_CHARA + i, wk->BaseZ - 8, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
			}
			else
			{
				SetMaterial(wk->alphaMainMenu, 0.5f, 0.5f, 0.5f);
				njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_CHARA + i, wk->BaseZ - 8, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
				SetMaterial(wk->alphaMainMenu, 1.0f, 1.0f, 1.0f);
			}
			
			// Draw cursor
			for (int p = PLAYER_MAX - 1; p >= 0; --p)
			{
				if (i == selected_characters[p])
				{
					if (wk->Stat != ADVA_STAT_FADEOUT)
					{
						CursorColors[p].a = 0.75f + 0.25 * njSin(FrameCounter * 1000);
					}
					else
					{
						CursorColors[p].a = wk->alphaMainMenu;
					}

					___njSetConstantMaterial(&CursorColors[p]);
					njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_CURSOR, wk->BaseZ + 100,  NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
					SetMaterial(wk->alphaMainMenu, 1.0f, 1.0f, 1.0f);
				}
			}
		}
	}
	else
	{
		DrawWaitingForPlayer(140, 230);
		DrawSADXText(press_start_texts[TextLanguage], 280);
	}
}

void __cdecl MultiMenuExec_Display(task* tp)
{
	auto wk = (MultiMenuWK*)tp->awp;
	
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

		// Draw charsel background and items
		multi_menu_disp_charsel(wk);

		// Draw controls
		multi_menu_disp_controls(wk);

		ResetMaterial();
		gHelperFunctions->PopScaleUI();
	}
}

void __cdecl MultiMenuExec_Main(task* tp)
{
	auto wk = (MultiMenuWK*)tp->awp;

	// Check if our menu is ready
	if (SeqTp->awp->work.ul[1] == ADVA_MODE_MULTI && wk->Stat == ADVA_STAT_REQWAIT)
	{
		AvaLoadTexForEachMode(ADVA_MODE_MULTI);

		// Initialize menu or reset previous state
		if (saved_mode <= MD_MULTI_MODESEL)
		{
			menu_multi_reset();
			menu_multi_charsel_unready();
			menu_multi_change(wk, MD_MULTI_INITCHARSEL);
			wk->Stat = ADVA_STAT_FADEIN;
			wk->T = 0.0f;
		}
		else
		{
			menu_multi_change(wk, saved_mode);
			wk->Stat = ADVA_STAT_KEEP;
			wk->T = 1.0f;
			wk->BaseCol = 0xFFFFFFFF;
		}

		PlayMenuMusicID(MusicIDs_JingleE);
		LoadPVM("AVA_MULTI", &AVA_MULTI_TEXLIST);
		LoadPVM("CON_MULTI", &CON_MULTI_TEXLIST);
		wk->alphaMainMenu = 1.0f;
		wk->alphaControls = 1.0f;
		wk->SelStg = -1;
	}

	// Check if our menu has to change
	if (SeqTp->awp->work.ul[2] == ADVA_MODE_MULTI && wk->Stat == ADVA_STAT_KEEP)
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
		
		// Fade out shouldn't happen when going back to main menu
		if (wk->SelStg >= 0)
		{
			wk->BaseCol = GetFadeInColFromT(wk->T);
		}

		if (wk->T >= 1.0f)
		{
			wk->Stat = ADVA_STAT_REQWAIT;

			SeqTp->awp[1].work.ub[15] = 1;

			njReleaseTexture(&AVA_MULTI_TEXLIST);
			njReleaseTexture(&CON_MULTI_TEXLIST);

			AvaReleaseTexForEachMode();

			// Force stage mode:
			if (wk->SelStg >= 0)
			{
				SeqTp->awp[1].work.sl[1] = 100;
			}
		}

		break;
	default:
		return;
	}
}

void __cdecl LoadMultiMenuExec(ModeSelPrmType* prmp)
{
	auto tp = CreateElementalTask(0, LEV_4, MultiMenuExec_Main);

	auto wk = (MultiMenuWK*)AllocateArray(1, sizeof(MultiMenuWK));
	wk->NextMode = prmp->NextMode;
	wk->PrevMode = prmp->PrevMode;

	tp->awp = (anywk*)wk;
	tp->disp = MultiMenuExec_Display;

	wk->BaseZ = -10000.0f;
}

void InitMultiMenu()
{
	CreateModeFncPtrs[ADVA_MODE_EXPLAIN] = LoadMultiMenuExec; // Replace unused menu
	AvaTexLdLists[ADVA_MODE_EXPLAIN] = AvaTexLdListForMulti;
}
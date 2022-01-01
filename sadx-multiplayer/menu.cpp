#include "pch.h"
#include "menu.h"
#include "menu_multi.h"

bool MultiMenuEnabled = false;

// Menu layout with minigames
PanelPrmType PanelPrmTitleMenu1[]
{
	 { 4.0f, -120.0f, 1 },
	 { 4.0f, -80.0f, 5 },
	 { 4.0f, -40.0f, 11 },
	 { 4.0f, 0.0f, 2 },
	 { 4.0f, 40.0f, 8 },
	 { 4.0f, 80.0f, 4 },
	 { 4.0f, 120.0f, 9 }
};

// Menu layout without minigames
PanelPrmType PanelPrmTitleMenu2[]
{
	 { 4.0f, -100.0f, 1 },
	 { 4.0f, -60.0f, 5 },
	 { 4.0f, -20.0f, 11 },
	 { 4.0f, 20.0f, 2 },
	 { 4.0f, 60.0f, 4 },
	 { 4.0f, 100.0f, 9 },
};

// Menu layout without minigames
PanelPrmType PanelPrmTitleMenuMulti[]
{
	 { 4.0f, -30.0f, 12 },
	 { 4.0f, 30.0f, 13 },
};

const DialogPrmType MainMenuMultiDialog = { DLG_PNLSTYLE_SIKAKU2, nullptr, &ava_title_e_TEXLIST, PanelPrmTitleMenuMulti, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 316.0, 236.0, 20.0, 232.0, 160.0, 0.71899998, 0.5, 2, 2 };

Trampoline* MainMenuExecSub_t = nullptr;

void MainMenuExecSub_r(TitleMenuWk* wkp)
{
	if (wkp->SubMode == 1)
	{
		int stat = GetDialogStat();
		int v8 = 0;
		char v13[4];

		// adjust menu id if the mini game button is not there
		if (stat >= 4 && !IsMiniGameMenuEnabled())
		{
			stat += 1;
		}

		wkp->SelMenu = (TitleMenuEnum)stat;
		
		AdvertiseWork.MainMenuSelectedMode = GblMenuTbl[stat >= 2 ? stat - 1 : stat];

		switch (stat)
		{
		case 0: // Adventure
		case 1: // Trial
			TARGET_DYNAMIC(MainMenuExecSub)(wkp); // first two items doesn't need adjusting so original is fine
			break;
		case 2: // Multiplayer (custom)
			OpenDialog(&MainMenuMultiDialog);
			wkp->SubMode = (TitleMenuSbMdEnum)7;
			break;
		case 3: // Mission
			CmnAdvaModeProcedure(ADVA_MODE_CHAR_SEL);
			wkp->SubMode = TITLEMENU_SMD_NWAIT;
			break;
		case 4: // MiniGame
			CmnAdvaModeProcedure(ADVA_MODE_PUTI);
			wkp->SubMode = TITLEMENU_SMD_NWAIT;
			break;
		case 5: // Option
			CmnAdvaModeProcedure(ADVA_MODE_OPTION_SEL);
			wkp->SubMode = TITLEMENU_SMD_NWAIT;
			break;
		case 6: // Exit
			IsExiting = TRUE;
			break;
		case 7: // Back to title screen
			CmnAdvaModeProcedure(wkp->PrevMode);
			SaveFile.LastCharacter = 0;
			j_LoadSave();
			memcpy((void*)0x3B29D70, &SaveFile, sizeof(SaveFile));
			wkp->SubMode = TITLEMENU_SMD_NWAIT;
			break;
		}

		return;
	}
	else if (wkp->SubMode == 7)
	{
		switch (GetDialogStat())
		{
		case 0:
		case 1:
			CmnAdvaModeProcedure(ADVA_MODE_TRIALACT_SEL); // go directly to level select
			AdvertiseWork.flags[1] = 0;
			wkp->SubMode = TITLEMENU_SMD_NWAIT;
			MultiMenuEnabled = true; // replace trial with multi mode
			break;
		case 2:
			wkp->SubMode = TITLEMENU_SMD_TO_MAINMENU;
			break;
		}
	}

	TARGET_DYNAMIC(MainMenuExecSub)(wkp);
}

void __cdecl InitMenu(const HelperFunctions& helperFunctions)
{
	MainMenuExecSub_t = new Trampoline(0x50B630, 0x50B638, MainMenuExecSub_r);

	DialogPrm[2].PnlPrmPtr = PanelPrmTitleMenu1;
	DialogPrm[2].CsrMax = 7;
	DialogPrm[2].CsrCancel = 7;
	DialogPrm[2].SzY = 320.0f;

	DialogPrm[3].PnlPrmPtr = PanelPrmTitleMenu2;
	DialogPrm[3].CsrMax = 6;
	DialogPrm[3].CsrCancel = 6;
	DialogPrm[3].SzY = 280.0f;

	MultiMenuInit();
}
#include "pch.h"
#include "menu.h"
#include "menu_multi.h"
#include "menu_adventure.h"
#include "splitscreen.h"
#include "netplay.h"

// Menu layout with minigames
PanelPrmType PanelPrmTitleMenu1[]
{
	 { 4.0f, -120.0f, 1 },
	 { 4.0f, -80.0f, 5 },
	 { 4.0f, -40.0f, 2 },
	 { 4.0f, 0.0f, 11 },
	 { 4.0f, 40.0f, 8 },
	 { 4.0f, 80.0f, 4 },
	 { 4.0f, 120.0f, 9 }
};

// Menu layout without minigames
PanelPrmType PanelPrmTitleMenu2[]
{
	 { 4.0f, -100.0f, 1 },
	 { 4.0f, -60.0f, 5 },
	 { 4.0f, -20.0f, 2 },
	 { 4.0f, 20.0f, 11 },
	 { 4.0f, 60.0f, 4 },
	 { 4.0f, 100.0f, 9 },
};

// Menu layout without minigames
PanelPrmType PanelPrmTitleMenuMulti[]
{
	 { 4.0f, -30.0f, 12 },
	 { 4.0f, 30.0f, 13 },
};

const DialogPrmType MainMenuMultiDialog = { DLG_PNLSTYLE_SIKAKU2, nullptr, &ava_title_e_TEXLIST, PanelPrmTitleMenuMulti, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 316.0f, 236.0f, 20.0f, 232.0f, 160.0f, 0.719f, 0.5, 2, 2 };

FastFunctionHook<void, TitleMenuWk*> title_menu_sub_exec_t(0x50B630);
FastFunctionHook<void, task*> dialog_disp_t(0x432480);
int selected_multi_mode = 0;

// Make sure ingame dialogs draw widescreen
static void __cdecl dialog_disp_r(task* tp)
{
	if (SplitScreen::IsActive())
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		dialog_disp_t.Original(tp);
		SplitScreen::RestoreViewPort();
	}
	else
	{
		dialog_disp_t.Original(tp);
	}
}

static bool AvaGetMultiEnable()
{
	return AvaGetTrialEnable();
}

static void OpenMultiModeDialog()
{
#ifdef MULTI_NETPLAY
	OpenDialogCsrLet(&MainMenuMultiDialog, selected_multi_mode, NULL);
#else
	char csr[2] = { 1, -1 };
	OpenDialogCsrLet(&MainMenuMultiDialog, selected_multi_mode, csr);
#endif
}

void title_menu_sub_exec_r(TitleMenuWk* wkp)
{
	// Make sure netplay got disconnected
	if (netplay.IsConnected())
	{
		netplay.Exit();
	}

	// Make sure multiplayer got disabled
	if (multiplayer::IsEnabled())
	{
		multiplayer::Disable();
	}

	if (wkp->SubMode == TITLEMENU_SMD_STAY || wkp->SubMode == TITLEMENU_SMD_TO_MAINMENU)
	{
		char csrp[9]{}; // disable items
		int cnt = 0; // amount of disabled items

		if (!AvaGetTrialEnable())
		{
			csrp[cnt++] = 1;
		}

		if (!AvaGetMissionEnable())
		{
			csrp[cnt++] = 2;
		}

		if (!AvaGetMultiEnable())
		{
			csrp[cnt++] = 3;
		}

		csrp[cnt] = -1;

		if (IsMiniGameMenuEnabled())
		{
			AdvaOpenDialogQuick(DIA_TYPE_MAINMENU, wkp->SelMenu, csrp);
		}
		else
		{
			AdvaOpenDialogQuick(DIA_TYPE_MAINMENU_NPUTI, wkp->SelMenu, csrp);
		}

		wkp->SubMode = TITLEMENU_SMD_DECIDE;
	}
	else if (wkp->SubMode == TITLEMENU_SMD_DECIDE)
	{
		TldFlg = TRUE;

		int stat = GetDialogStat();

		// adjust menu id if the mini game button is not there
		if (stat >= 4 && !IsMiniGameMenuEnabled())
		{
			stat += 1;
		}

		wkp->SelMenu = (TitleMenuEnum)stat;

		AdvertiseWork.MainMenuSelectedMode = GblMenuTbl[stat >= 3 ? stat - 1 : stat];

		switch (stat)
		{
		case 0: // Adventure
		case 1: // Trial
		case 2: // Mission
			title_menu_sub_exec_t.Original(wkp); // first three items do not need adjusting so original is fine
			break;
		case 3: // Multiplayer (custom)
			OpenMultiModeDialog();
			wkp->SubMode = (TitleMenuSbMdEnum)7;
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
	}
	else if (wkp->SubMode == 7)
	{
		switch (GetDialogStat())
		{
		case 0:
			selected_multi_mode = 0;
			CmnAdvaModeProcedure((AdvaModeEnum)ADVA_MODE_MULTI);
			wkp->SubMode = TITLEMENU_SMD_NWAIT;
			break;
		case 1:
			selected_multi_mode = 1;
			CmnAdvaModeProcedure((AdvaModeEnum)ADVA_MODE_MULTI);
			wkp->SubMode = TITLEMENU_SMD_NWAIT;
			break;
		case 2:
			wkp->SubMode = TITLEMENU_SMD_TO_MAINMENU;
			break;
		}
	}
	else
	{
		title_menu_sub_exec_t.Original(wkp);
	}
}

void __cdecl InitMenu()
{
	title_menu_sub_exec_t.Hook(title_menu_sub_exec_r);
	dialog_disp_t.Hook(dialog_disp_r);

	DialogPrm[2].PnlPrmPtr = PanelPrmTitleMenu1;
	DialogPrm[2].CsrMax = 7;
	DialogPrm[2].CsrCancel = 7;
	DialogPrm[2].SzY = 320.0f;

	DialogPrm[3].PnlPrmPtr = PanelPrmTitleMenu2;
	DialogPrm[3].CsrMax = 6;
	DialogPrm[3].CsrCancel = 6;
	DialogPrm[3].SzY = 280.0f;

	InitMultiMenu();
	InitAdventureMenu();
}
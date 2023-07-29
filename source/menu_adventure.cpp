#include "pch.h"
#include "SADXModLoader.h"
#include "multiplayer.h"
#include "players.h"

#define CHSEL_SMD_COOP CHSEL_SMD_MAX

UsercallFuncVoid(char_sel_sub_exec, (CharSelWk* wkp), (wkp), 0x511570, rESI);

// Game, instruction, cancel (Unfinished stories)
PanelPrmType PanelPrmCharsel1_m[]
{
	{ -198.0f, 0.0f, 2 },
	{ -66.0f, 0.0f, 14 }, // Coop
	{ 66.0f, 0.0f, 5 },
	{ 198.0f, 0.0f, 0 }
};

// Game, instruction, ending, cancel (Finished stories)
PanelPrmType PanelPrmCharsel2_m[]
{
	{ -230.0f, 0.0f, 2 },
	{ -115.0f, 0.0f, 14 }, // Coop
	{ 0.0f, 0.0f, 5 },
	{ 115.0f, 0.0f, 1 },
	{ 230.0f, 0.0f, 0 }
};

// Game, cancel (Super Sonic story unfinished)
PanelPrmType PanelPrmCharsel3_m[]
{
	{ -155.0f, 0.0f, 2 },
	{ 0.0f, 0.0f, 14 },
	{ 155.0f, 0.0f, 0 }
};

// Game, ending, cancel (Super Sonic story finished)
PanelPrmType PanelPrmCharsel4_m[]
{
	{ -198.0f, 0.0f, 2 },
	{ -66.0f, 0.0f, 14 },
	{ 66.0f, 0.0f, 1 },
	{ 198.0f, 0.0f, 0 }
};

void __cdecl char_sel_sub_exec_r(CharSelWk* wkp)
{
	auto stat = GetDialogStat();

	switch (wkp->SubMode)
	{
	case CHSEL_SMD_DLG1:
		if (stat < 0)
			break;

		switch (stat)
		{
		case 0: // 0
			wkp->DlgStat = ADVDLG_KAISHI;
			wkp->SubMode = CHSEL_SMD_DECIDE2;
			break;
		case 1: // new slot
			wkp->SubMode = CHSEL_SMD_COOP;
			break;
		case 2: // 1
			wkp->DlgStat = ADVDLG_SETSUMEI;
			wkp->SubMode = CHSEL_SMD_DECIDE2;
			break;
		case 3: // 2
			wkp->DlgStat = ADVDLG_CANCEL;
			wkp->SubMode = CHSEL_SMD_DECIDE2;
			break;
		default:
			wkp->SubMode = CHSEL_SMD_DECIDE2;
			break;
		}

		break;
	case CHSEL_SMD_DLG2:
		if (stat < 0)
			break;

		switch (stat)
		{
		case 0: // 0
			wkp->DlgStat = ADVDLG_KAISHI;
			wkp->SubMode = CHSEL_SMD_DECIDE2;
			break;
		case 1: // new slot
			wkp->SubMode = CHSEL_SMD_COOP;
			break;
		case 2: // 1
			wkp->DlgStat = ADVDLG_SETSUMEI;
			wkp->SubMode = CHSEL_SMD_DECIDE2;
			break;
		case 3: // 2
			wkp->DlgStat = ADVDLG_ENDING;
			wkp->SubMode = CHSEL_SMD_DECIDE2;
			break;
		case 4: // 3
			wkp->DlgStat = ADVDLG_CANCEL;
			wkp->SubMode = CHSEL_SMD_DECIDE2;
			break;
		default:
			wkp->SubMode = CHSEL_SMD_DECIDE2;
			break;
		}

		break;
	case CHSEL_SMD_COOP:
		multiplayer::Enable(2, multiplayer::mode::coop);
		SetCurrentCharacter(1, Characters_Tails);
		wkp->DlgStat = ADVDLG_KAISHI;
		wkp->SubMode = CHSEL_SMD_DECIDE2;
		break;
	default:
		char_sel_sub_exec.Original(wkp);
		return;
	}

	calcvsyncsyoriochi();
	loop_count = 0;
	adva_loop_count = gu32loop_count;
}

void InitAdventureMenu()
{
#ifdef MULTI_TEST
	DialogPrm[DIA_TYPE_CHARSEL1].PnlPrmPtr = PanelPrmCharsel1_m;
	DialogPrm[DIA_TYPE_CHARSEL1].CsrMax = LengthOfArray(PanelPrmCharsel1_m);
	DialogPrm[DIA_TYPE_CHARSEL1].CsrCancel = DialogPrm[DIA_TYPE_CHARSEL1].CsrMax - 1;
	DialogPrm[DIA_TYPE_CHARSEL2].PnlPrmPtr = PanelPrmCharsel2_m;
	DialogPrm[DIA_TYPE_CHARSEL2].CsrMax = LengthOfArray(PanelPrmCharsel2_m);
	DialogPrm[DIA_TYPE_CHARSEL2].CsrCancel = DialogPrm[DIA_TYPE_CHARSEL2].CsrMax - 1;
	DialogPrm[DIA_TYPE_CHARSEL2].SzX = 580.0f;
	DialogPrm[DIA_TYPE_CHARSEL2].CntrX = 320.0f;
	//DialogPrm[DIA_TYPE_CHARSEL3].PnlPrmPtr = PanelPrmCharsel3_m;
	//DialogPrm[DIA_TYPE_CHARSEL3].CsrMax = LengthOfArray(PanelPrmCharsel3_m);
	//DialogPrm[DIA_TYPE_CHARSEL3].CsrCancel = DialogPrm[DIA_TYPE_CHARSEL3].CsrMax - 1;
	//DialogPrm[DIA_TYPE_CHARSEL4].PnlPrmPtr = PanelPrmCharsel4_m;
	//DialogPrm[DIA_TYPE_CHARSEL4].CsrMax = LengthOfArray(PanelPrmCharsel4_m);
	//DialogPrm[DIA_TYPE_CHARSEL4].CsrCancel = DialogPrm[DIA_TYPE_CHARSEL4].CsrMax - 1;

	char_sel_sub_exec.Hook(char_sel_sub_exec_r);
#endif
}
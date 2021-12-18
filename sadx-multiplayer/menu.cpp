#include "pch.h"
#include "menu.h"
#include "multi.h"

#define ReplacePVM(C, D) helperFunctions.ReplaceFile("system\\" C ".pvm", "system\\" D ".pvmx")

int CurrentMenu = 0;
bool MultiMenuEnabled = false;

PanelPrmType PanelPrmTitleMenu_r[]
{
	{ 4.0f, -90.0f, 0 },
	{ 4.0f, -30.0f, 3 },
	{ 4.0f, 30.0f, 7 },
	{ 4.0f, 90.0f, 11 }
};

Trampoline* MainMenuExecSub_t;

void MainMenuExecSub_r(TitleMenuWk* wkp)
{
	if (wkp->SubMode == TITLEMENU_SMD_STAY_S)
	{
		auto state = GetDialogStat();

		if (GetDialogStat() == 3)
		{
			CmnAdvaModeProcedure(ADVA_MODE_TRIALACT_SEL); // go directly to level select
			AdvertiseWork.flags[1] = 0;
			wkp->SubMode = TITLEMENU_SMD_NWAIT;
			MultiMenuEnabled = true; // replace trial with multi mode
			return;
		}
		else
		{
			MultiMenuEnabled = false;
		}
	}

	TARGET_DYNAMIC(MainMenuExecSub)(wkp);
}

void __cdecl Menu_Init(const HelperFunctions& helperFunctions)
{
	ResizeTextureList(&ava_title_e_TEXLIST, 12);
	MainMenuExecSub_t = new Trampoline(0x50B630, 0x50B638, MainMenuExecSub_r);
	DialogPrm[4].PnlPrmPtr = PanelPrmTitleMenu_r;
	DialogPrm[4].CsrMax = 4;
	DialogPrm[4].CsrCancel = 4;
	DialogPrm[4].SzY = 260.0f;
	ReplacePVM("AVA_TITLE_E", "AVA_TITLE_E_MULTI");
	init_MultiMenu();
}
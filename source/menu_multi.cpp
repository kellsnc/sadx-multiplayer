#include "pch.h"
#include <utility>
#include "SADXModLoader.h"
#include "menu.h"
#include "network.h"
#include "config.h"
#include "input.h"
#include "menu_multi.h"

enum AVA_MULTI_TEX
{
	AVAMULTITEX_SONIC,
	AVAMULTITEX_EGGMAN,
	AVAMULTITEX_TAILS,
	AVAMULTITEX_KNUCKLES,
	AVAMULTITEX_TIKAL,
	AVAMULTITEX_AMY,
	AVAMULTITEX_BIG,
	AVAMULTITEX_E102,
	AVAMULTITEX_METAL,
	AVAMULTITEX_BACK,
	AVAMULTITEX_CONFIRM,
	AVAMULTITEX_SELECT,
	AVAMULTITEX_ICON,
	AVAMULTITEX_TITLE,
	AVAMULTITEX_ACT1,
	AVAMULTITEX_ACT2,
	AVAMULTITEX_ACT3,
	AVAMULTITEX_YES,
	AVAMULTITEX_NO,
	AVAMULTITEX_CONTINUE,
	AVAMULTITEX_CONNECT,
	AVAMULTITEX_HOST,
	AVAMULTITEX_COOP,
	AVAMULTITEX_BATTLE,
	AVAMULTITEX_CURSORBG,
	AVAMULTITEX_CURSOR1,
	AVAMULTITEX_CURSOR2,
	AVAMULTITEX_CSR1,
	AVAMULTITEX_CSR2,
	AVAMULTITEX_CSR3,
	AVAMULTITEX_CSR4,
	AVAMULTITEX_MD_SPD,
	AVAMULTITEX_MD_FLY,
	AVAMULTITEX_MD_EME,
	AVAMULTITEX_MD_EGROB,
	AVAMULTITEX_MD_FISH,
	AVAMULTITEX_MD_SHOOT,
	AVAMULTITEX_MD_TC,
	AVAMULTITEX_MD_VS,
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
	AVAMULTITEX_STGBOARD,
	AVAMULTITEX_STG0202,
	AVAMULTITEX_STG0302A,
	AVAMULTITEX_STG1000A,
	AVAMULTITEX_STG1002E,
	AVAMULTITEX_STG0501E,
	AVAMULTITEX_STG1202E,
	AVAMULTITEX_STG1500,
	AVAMULTITEX_STG1600,
	AVAMULTITEX_STG1700,
	AVAMULTITEX_STG1800,
	AVAMULTITEX_STG2000,
	AVAMULTITEX_STG2100,
	AVAMULTITEX_STG2400,
	AVAMULTITEX_STG2500
};

enum AVA_MULTI_ANM
{
	AVAMULTIANM_CHARA,
	AVAMULTIANM_CURSOR = 8,
	AVAMULTIANM_BACK,
	AVAMULTIANM_CONFIRM,
	AVAMULTIANM_SELECT,
	AVAMULTIANM_ICON,
	AVAMULTIANM_STG
};

enum MD_MULTI
{
	MD_MULTI_CONNECT,
	MD_MULTI_LOCALCON,
	MD_MULTI_ONLINECON,
	MD_MULTI_MODESEL,
	MD_MULTI_COOPSEL,
	MD_MULTI_BATTLESEL,
	MD_MULTI_CHARSEL,
	MD_MULTI_STGSEL_SPD,
	MD_MULTI_STGSEL_FLY,
	MD_MULTI_STGSEL_EME,
	MD_MULTI_STGSEL_EGROB,
	MD_MULTI_STGSEL_FISH,
	MD_MULTI_STGSEL_SHOOT,
	MD_MULTI_STGSEL_TC,
	MD_MULTI_STGSEL_VS,
	MD_MULTI_STGASK
};

enum CharSelChara
{
	CharSelChara_Sonic,
	CharSelChara_Tails,
	CharSelChara_Knuckles,
	CharSelChara_Amy,
	CharSelChara_Big,
	CharSelChara_Gamma,
	CharSelChara_Tikal,
	CharSelChara_Eggman,
};

enum class ConnectMenuSelection
{
	AddressBar,
	Client,
	Host
};

enum class ConnectMenuMode
{
	Select,
	Hub,
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
	float alphaCharSel;
	float alphaConnect;
	float alphaControls;
	int stgreq;
	int actreq;
};

NJS_TEXNAME AVA_MULTI_TEXNAME[78]{};
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
	{ 32, 32, 16, 16, 0, 0, 255, 255, AVAMULTITEX_ICON, 0x20},
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

PanelPrmType PanelPrmMenuMultiModeSel[] {
	 { -100.0f,	0.0f, AVAMULTITEX_COOP   },
	 { 100.0f,	0.0f, AVAMULTITEX_BATTLE }
};

PanelPrmType PanelPrmMenuMultiCoopSel[]{
	 { -180.0f,	-52.0f, AVAMULTITEX_MD_SPD },
	 { -60.0f,	-52.0f, AVAMULTITEX_MD_FLY },
	 { 60.0f,	-52.0f, AVAMULTITEX_MD_EME },
	 { 180.0f,	-52.0f, AVAMULTITEX_MD_EGROB },
	 { -180.0f,	68.0f, AVAMULTITEX_MD_FISH },
	 { -60.0f,	68.0f, AVAMULTITEX_MD_SHOOT }
};

PanelPrmType PanelPrmMenuMultiBattleSel[] {
	 { -180.0f,	-52.0f, AVAMULTITEX_MD_SPD },
	 { -60.0f,	-52.0f, AVAMULTITEX_MD_FLY },
	 { 60.0f,	-52.0f, AVAMULTITEX_MD_EME },
	 { 180.0f,	-52.0f, AVAMULTITEX_MD_EGROB },
	 { -180.0f,	68.0f, AVAMULTITEX_MD_FISH },
	 { -60.0f,	68.0f, AVAMULTITEX_MD_SHOOT },
	 { 60.0f,	68.0f, AVAMULTITEX_MD_TC },
	 { 180.0f,	68.0f, AVAMULTITEX_MD_VS }
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
	 { 75.0f,	120.0f,  AVAMULTITEX_STGBOARD },
};

PanelPrmType PanelPrmMenuMultiStgSelFly[]{
	 { -150.0f,	-40.0f, AVAMULTITEX_STG2 },
	 { 0.0f,	-40.0f, AVAMULTITEX_STG3 },
	 { 150.0f,	-40.0f, AVAMULTITEX_STG4 },
	 { -75.0f,	40.0f,  AVAMULTITEX_STG8 },
	 { 75.0f,	40.0f,  AVAMULTITEX_STG6 }
};

PanelPrmType PanelPrmMenuMultiStgSelEme[]{
	 { -150.0f,	-40.0f, AVAMULTITEX_STGEME1 },
	 { 0.0f,	-40.0f, AVAMULTITEX_STGEME2 },
	 { 150.0f,	-40.0f, AVAMULTITEX_STGEME3 },
	 { -75.0f,	40.0f,  AVAMULTITEX_STGEME4 },
	 { 75.0f,	40.0f,  AVAMULTITEX_STGEME5 }
};

PanelPrmType PanelPrmMenuMultiStgSelEgRob[]{
	 { -100.0f,	-65.0f, AVAMULTITEX_STG0302A },
	 { 100.0f,	-65.0f, AVAMULTITEX_STGBIG4 },
	 { 0.0f,    65.0f,  AVAMULTITEX_STG1000A }
};

PanelPrmType PanelPrmMenuMultiStgSelBig[]{
	 { -100.0f, -65.0f, AVAMULTITEX_STGBIG1 },
	 { 100.0f,  -65.0f, AVAMULTITEX_STGBIG2 },
	 { -100.0f, 65.0f,  AVAMULTITEX_STGBIG3 },
	 { 100.0f,  65.0f,  AVAMULTITEX_STGBIG4 }
};

PanelPrmType PanelPrmMenuMultiStgSelShoot[]{
	 { -150.0f,	-40.0f, AVAMULTITEX_STG1002E },
	 { 0.0f,	-40.0f, AVAMULTITEX_STG1 },
	 { 150.0f,	-40.0f, AVAMULTITEX_STG2 },
	 { -75.0f,	40.0f,  AVAMULTITEX_STG0501E },
	 { 75.0f,	40.0f,  AVAMULTITEX_STG1202E }
};

PanelPrmType PanelPrmMenuMultiStgSelTwinkle[]{
	 { -150.0f,	-40.0f, AVAMULTITEX_STGTC1 },
	 { 0.0f,	-40.0f, AVAMULTITEX_STGTC2 },
	 { 150.0f,	-40.0f, AVAMULTITEX_STGTC3 },
	 { -150.0f,	40.0f,  AVAMULTITEX_STGTC4 },
	 { 0.0f,	40.0f,  AVAMULTITEX_STGTC5 },
	 { 150.0f,	40.0f,  AVAMULTITEX_STGTC6 }
};

PanelPrmType PanelPrmMenuMultiStgSelVs[]{
	 { -150.0f,	-80.0f, AVAMULTITEX_STG1500 },
	 { 0.0f,	-80.0f, AVAMULTITEX_STG1600 },
	 { 150.0f,	-80.0f, AVAMULTITEX_STG1700 },
	 { -150.0f,	0.0f,   AVAMULTITEX_STG1800 },
	 { 0.0f,	0.0f,   AVAMULTITEX_STG2000 },
	 { 150.0f,	0.0f,   AVAMULTITEX_STG2100 },
	 { -75.0f,	80.0f,  AVAMULTITEX_STG2400 },
	 { 75.0f,	80.0f,  AVAMULTITEX_STG2500 }
};

PanelPrmType PanelPrmMenuMultiStgConfirm[4] {};

void menu_multi_confirmdialog_proc(DDlgType* ddltype);

const DialogPrmType MultiMenuModeSelDialog = { DLG_PNLSTYLE_MARU2, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiModeSel, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 10.0f, 568.0f, 120.0f, 1.625f, 0.8f, LengthOfArray<char>(PanelPrmMenuMultiModeSel), 2i8 };
const DialogPrmType MultiMenuCoopSelDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiCoopSel, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 20.0f, 500.0f, 260.0f, 1.2f, 1.2f, LengthOfArray<char>(PanelPrmMenuMultiCoopSel), 6i8};
const DialogPrmType MultiMenuBattleSelDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiBattleSel, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 20.0f, 500.0f, 260.0f, 1.2f, 1.2f, LengthOfArray<char>(PanelPrmMenuMultiBattleSel), 8i8};
const DialogPrmType MultiMenuStageSelSonicDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelSonic, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 280.0f, 20.0f, 500.0f, 340.0f, 2.1f, 1.2f, LengthOfArray<char>(PanelPrmMenuMultiStgSelSonic), 11i8 };
const DialogPrmType MultiMenuStageSelFlyDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelFly, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 20.0f, 500.0f, 200.0f, 2.1f, 1.2f, LengthOfArray<char>(PanelPrmMenuMultiStgSelFly), 5i8 };
const DialogPrmType MultiMenuStageSelEmeDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelEme, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 20.0f, 500.0f, 200.0f, 2.1f, 1.2f, LengthOfArray<char>(PanelPrmMenuMultiStgSelEme), 5i8 };
const DialogPrmType MultiMenuStageSelEgRobDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelEgRob, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 260.0f, 20.0f, 500.0f, 290.0f, 3.0f, 1.7f, LengthOfArray<char>(PanelPrmMenuMultiStgSelEgRob), 3i8};
const DialogPrmType MultiMenuStageSelBigDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelBig, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 260.0f, 20.0f, 500.0f, 290.0f, 3.0f, 1.7f, LengthOfArray<char>(PanelPrmMenuMultiStgSelBig), 4i8};
const DialogPrmType MultiMenuStageSelShootDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelShoot, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 20.0f, 500.0f, 200.0f, 2.1f, 1.2f, LengthOfArray<char>(PanelPrmMenuMultiStgSelShoot), 5i8 };
const DialogPrmType MultiMenuStageSelTwinkleDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelTwinkle, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 250.0f, 20.0f, 500.0f, 200.0f, 2.1f, 1.2f, LengthOfArray<char>(PanelPrmMenuMultiStgSelTwinkle), 6i8};
const DialogPrmType MultiMenuStageSelVsDialog = { DLG_PNLSTYLE_MARU4, nullptr, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgSelVs, (DlgSndPrmType*)0x7DFE08, 0x7812B4FF, 0x7812B4FF, 320.0f, 260.0f, 20.0f, 500.0f, 290.0f, 2.1f, 1.2f, LengthOfArray<char>(PanelPrmMenuMultiStgSelVs), 8i8 };
DialogPrmType MultiMenuStageConfirmDialog = { DLG_PNLSTYLE_MARU, menu_multi_confirmdialog_proc, &AVA_MULTI_TEXLIST, PanelPrmMenuMultiStgConfirm, (DlgSndPrmType*)0x7DFE08, 0x97008740, 0x97008740, 320.0f, 369.0f, 10.0f, 568.0f, 140.0f, 1.625f, 0.8f, 4i8, 3i8 };

std::pair<int, int> spd_level_link_btl[] = {
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
	{ LevelAndActIDs_SandHill, 1 }
};

int spd_level_link[]{
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
	LevelAndActIDs_HotShelter1,
	LevelAndActIDs_SandHill
};

int fly_level_link[]{
	LevelAndActIDs_WindyValley3,
	LevelAndActIDs_Casinopolis2,
	LevelAndActIDs_IceCap3,
	LevelAndActIDs_SkyDeck1,
	LevelAndActIDs_SpeedHighway1
};

int eme_level_link[]{
	LevelAndActIDs_SpeedHighway3,
	LevelAndActIDs_Casinopolis1,
	LevelAndActIDs_RedMountain3,
	LevelAndActIDs_LostWorld2,
	LevelAndActIDs_SkyDeck3
};

int egrob_level_link[]{
	LevelAndActIDs_TwinklePark2,
	LevelAndActIDs_HotShelter1,
	LevelAndActIDs_FinalEgg2
};

int fish_level_link[]{
	LevelAndActIDs_TwinklePark2,
	LevelAndActIDs_IceCap2,
	LevelAndActIDs_EmeraldCoast3,
	LevelAndActIDs_HotShelter1
};

int shoot_level_link[]{
	LevelAndActIDs_FinalEgg3,
	LevelAndActIDs_EmeraldCoast1,
	LevelAndActIDs_WindyValley1,
	LevelAndActIDs_RedMountain2,
	LevelAndActIDs_HotShelter3
};

int twinkle_level_link[]{
	LevelAndActIDs_TwinkleCircuit1,
	LevelAndActIDs_TwinkleCircuit2,
	LevelAndActIDs_TwinkleCircuit3,
	LevelAndActIDs_TwinkleCircuit4,
	LevelAndActIDs_TwinkleCircuit5,
	LevelAndActIDs_TwinkleCircuit6
};

int charsel_voicelist[] {
	1098,
	569,
	528,
	392,
	510,
	921,
	394,
	1346,
	2047
};

int vs_level_link[]{
	LevelAndActIDs_Chaos0,
	LevelAndActIDs_Chaos2,
	LevelAndActIDs_Chaos4,
	LevelAndActIDs_Chaos6Knuckles,
	LevelAndActIDs_EggHornet,
	LevelAndActIDs_EggWalker,
	LevelAndActIDs_E101,
	LevelAndActIDs_E101R
};

const AvaTexLdEnum AvaTexLdListForMulti[]{
	TENUM_AVA_BACK, TENUM_ADV_WINDOW, TENUM_AVA_SQUARE, TENUM_NMAX_SADXPC
};

const char* stg_confirm_texts[] {
	"Do you want to play this stage?"
	"Do you want to play this stage?",
	"Voulez-vous jouer � ce niveau ?",
	"�Quieres jugar este nivel?",
	"Do you want to play this stage?"
};

const char* press_start_texts[] {
	"Press start to join",
	"Press start to join",
	"Appuyez sur entrer pour joindre",
	"Presiona start para unirte",
	"Press start to join"
};

static constexpr size_t address_limit = 32;
task* MultiMenuTp = nullptr;
int stgacttexid = 0;
int selected_characters[PLAYER_MAX];
bool chara_ready[PLAYER_MAX];
bool player_ready[PLAYER_MAX];
int pcount;
bool enabled_characters[8];
MD_MULTI saved_mode;
MD_MULTI next_mode;
int gNextDialogStat;
ConnectMenuSelection gConnectSelection;
ConnectMenuMode gConnectMenuMode;
multiplayer::mode gNextMultiMode;
std::string address_text;

static MSGC msgc_address;
static MSGC msgc_connect;

static void FreeAddressText()
{
	MSG_Close(&msgc_address);
}

static void LoadAddressText()
{
	MSG_Open(&msgc_address, 0, 230, 0, 0, 0xD0000020);
	MSG_Cls(&msgc_address);
	MSG_Puts_(&msgc_address, address_text.c_str());
	MSG_LoadTexture(&msgc_address);
}

void menu_multi_charsel_unready()
{
	for (auto& i : selected_characters)
	{
		i = -1;
	}

	for (auto& i : chara_ready)
	{
		i = false;
	}
}

void menu_multi_unready()
{
	pcount = 0;

	for (auto& item : player_ready)
	{
		item = false;
	}
}

void menu_multi_setallcharacters(int c)
{
	for (auto& i : selected_characters)
	{
		i = c;
	}
}

int menu_multi_getplayerno(int num)
{
	switch (num)
	{
	case CharSelChara_Sonic:
	default:
		return Characters_Sonic;
	case CharSelChara_Tails:
		return Characters_Tails;
	case CharSelChara_Knuckles:
		return Characters_Knuckles;
	case CharSelChara_Amy:
		return Characters_Amy;
	case CharSelChara_Big:
		return Characters_Big;
	case CharSelChara_Gamma:
		return Characters_Gamma;
	case CharSelChara_Tikal:
		return Characters_Tikal;
	case CharSelChara_Eggman:
		return Characters_Eggman;
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
	for (size_t i = 0ui32; i < LengthOfArray(enabled_characters); ++i)
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

void menu_multi_change(MultiMenuWK* wk, MD_MULTI id)
{
	int nextdial = wk->SubMode < id ? 0 : gNextDialogStat;

	CloseDialog();

	auto previous = wk->SubMode;

	wk->SubMode = id;

	switch (id)
	{
	case MD_MULTI_CONNECT:
		if (selected_multi_mode != 1)
		{
			wk->SubMode = MD_MULTI_LOCALCON;
		}
		else
		{
			wk->SubMode = MD_MULTI_ONLINECON;

			if (previous != MD_MULTI_MODESEL)
			{
				gConnectSelection = ConnectMenuSelection::Client;
				gConnectMenuMode = ConnectMenuMode::Select;
				if (address_text.empty()) address_text = config::network::default_ip + ":" + std::to_string(config::network::default_port);
				if (address_text.size() > address_limit)
				{
					address_text.resize(address_limit);
				}
				LoadAddressText();
			}
		}

		menu_multi_unready();
		saved_mode = wk->SubMode;
		break;
	case MD_MULTI_MODESEL:
		menu_multi_setrndcursor();
		saved_mode = MD_MULTI_MODESEL;
		OpenDialogCsrLet(&MultiMenuModeSelDialog, nextdial, 0);
		break;
	case MD_MULTI_COOPSEL:
		menu_multi_setrndcursor();
		saved_mode = MD_MULTI_COOPSEL;
		OpenDialogCsrLet(&MultiMenuCoopSelDialog, nextdial, 0);
		break;
	case MD_MULTI_BATTLESEL:
		menu_multi_setrndcursor();
		saved_mode = MD_MULTI_BATTLESEL;
		OpenDialogCsrLet(&MultiMenuBattleSelDialog, nextdial, 0);
		break;
	case MD_MULTI_CHARSEL: // Open character select
		menu_multi_getcharaenable();
		menu_multi_charsel_unready();

		if (gNextMultiMode == multiplayer::mode::coop)
		{
			chara_ready[0] = true;
		}
		break;
	case MD_MULTI_STGSEL_SPD:
		menu_multi_setsqrcursor();
		saved_mode = MD_MULTI_STGSEL_SPD;
		OpenDialogCsrLet(&MultiMenuStageSelSonicDialog, nextdial, 0);
		break;
	case MD_MULTI_STGSEL_FLY:
		menu_multi_setsqrcursor();
		saved_mode = MD_MULTI_STGSEL_FLY;
		OpenDialogCsrLet(&MultiMenuStageSelFlyDialog, nextdial, 0);
		break;
	case MD_MULTI_STGSEL_EME:
		menu_multi_setsqrcursor();
		saved_mode = MD_MULTI_STGSEL_EME;
		OpenDialogCsrLet(&MultiMenuStageSelEmeDialog, nextdial, 0);
		break;
	case MD_MULTI_STGSEL_EGROB:
		menu_multi_setsqrcursor();
		saved_mode = MD_MULTI_STGSEL_EGROB;
		OpenDialogCsrLet(&MultiMenuStageSelEgRobDialog, nextdial, 0);
		break;
	case MD_MULTI_STGSEL_FISH:
		menu_multi_setsqrcursor();
		saved_mode = MD_MULTI_STGSEL_FISH;
		OpenDialogCsrLet(&MultiMenuStageSelBigDialog, nextdial, 0);
		break;
	case MD_MULTI_STGSEL_SHOOT:
		menu_multi_setsqrcursor();
		saved_mode = MD_MULTI_STGSEL_SHOOT;
		OpenDialogCsrLet(&MultiMenuStageSelShootDialog, nextdial, 0);
		break;
	case MD_MULTI_STGSEL_TC:
		menu_multi_setsqrcursor();
		saved_mode = MD_MULTI_STGSEL_TC;
		OpenDialogCsrLet(&MultiMenuStageSelTwinkleDialog, nextdial, 0);
		break;
	case MD_MULTI_STGSEL_VS:
		menu_multi_setsqrcursor();
		saved_mode = MD_MULTI_STGSEL_VS;
		OpenDialogCsrLet(&MultiMenuStageSelVsDialog, nextdial, 0);
		break;
	case MD_MULTI_STGASK: // Open prompt to ask level confirmation
		menu_multi_setrndcursor();
		OpenDialog(&MultiMenuStageConfirmDialog);
		break;
	}
}

void menu_multi_launch_level(MultiMenuWK* wk, int act)
{
	CloseDialog();

	// Enable multiplayer mode
	multiplayer::Enable(pcount, gNextMultiMode);

	for (int i = 0; i < pcount; ++i)
	{
		SetCurrentCharacter(i, (Characters)menu_multi_getplayerno(selected_characters[i]));
	}

	ResetNumPlayerM();

	// Force trial return to this menu instead of charsel
	WriteData((int*)0x7EEB58, (int)ADVA_MODE_MULTI);

	auto level = wk->stgreq;

	LastLevel = CurrentLevel;
	LastAct = CurrentAct;
	CurrentCharacter = menu_multi_getplayerno(selected_characters[0]);
	CurrentLevel = level;
	CurrentAct = act;
	SeqTp->awp[1].work.ul[0] = 100;
	AvaStgActT stgact = { static_cast<uint8_t>(level), static_cast<uint8_t>(act) };
	AvaCmnPrm = { static_cast<uint8_t>(level), static_cast<uint8_t>(act) };
	AdvertiseWork.Stage = level;
	AdvertiseWork.Act = act;
	wk->SelStg = level;
	wk->Stat = ADVA_STAT_FADEOUT;
	wk->T = 0.0f;
}

void menu_multi_request_stg(MultiMenuWK* wk, int level, int actcnt, int item)
{
	auto act = toactnum(level);

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
	menu_multi_change(wk, MD_MULTI_STGASK);
	wk->stgreq = tolevelnum(level);
	wk->actreq = toactnum(level);
}

void menu_multi_tomodesel(MultiMenuWK* wk)
{
	if (gNextMultiMode == multiplayer::mode::battle)
	{
		menu_multi_change(wk, MD_MULTI_BATTLESEL);
	}
	else
	{
		menu_multi_change(wk, MD_MULTI_COOPSEL);
	}
}

void menu_multi_confirmdialog_proc(DDlgType* ddltype)
{
	DrawSADXText(stg_confirm_texts[TextLanguage], 315);
	
	AVA_MULTI_SPRITE.p.x = 320;
	AVA_MULTI_SPRITE.p.y = 200;
	AVA_MULTI_SPRITE.tanim[AVAMULTIANM_STG].texid = stgacttexid;
	njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_STG, -100, NJD_SPRITE_ALPHA);
}

void menu_multi_stg_confirm(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	if (stat != -1)
	{
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
		else
		{
			menu_multi_change(wk, saved_mode);
		}
	}
}

void menu_multi_stgsel(MultiMenuWK* wk, const DialogPrmType* dialog, int* list, int back)
{
	auto stat = GetDialogStat();

	if (stat == dialog->CsrCancel) // go back request
	{
		gNextDialogStat = back;
		menu_multi_change(wk, MD_MULTI_BATTLESEL);
	}
	else if (stat != -1) // launch game request
	{
		gNextDialogStat = stat;
		menu_multi_request_stg(wk, list[stat], 1, stat);
	}
}

void menu_multi_charsel(MultiMenuWK* wk)
{
	bool done = true;

	// Return to main menu
	if (MenuBackButtonsPressedM())
	{
		if (gNextMultiMode == multiplayer::mode::coop || chara_ready[0] == false)
		{
			menu_multi_change(wk, saved_mode);
			return;
		}
	}

	// Manage input
	for (int i = gNextMultiMode == multiplayer::mode::coop ? 1 : 0; i < PLAYER_MAX; ++i)
	{
		auto& sel = selected_characters[i];
		auto press = GetPressedButtons(i);

		if (player_ready[i] == false) // If player is not connected
		{
			continue;
		}

		if (chara_ready[i] == false) // Character selection
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
				chara_ready[i] = true;
				PlayVoice(charsel_voicelist[menu_multi_getplayerno(sel)]);
				PlayMenuEnterSound();
			}
		}
		else // player is ready
		{
			if (press & Buttons_B) // unready
			{
				done = false;
				sel = -1;
				PlayMenuBackSound();
			}
		}
	}

	// If everyone has selected
	if (done == true)
	{
		menu_multi_change(wk, next_mode);
	}
}

void menu_multi_coopsel(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	if (stat >= 0)
	{
		if (stat == MultiMenuCoopSelDialog.CsrCancel)
		{
			gNextDialogStat = 0;
			menu_multi_change(wk, MD_MULTI_MODESEL);
		}
		else
		{
			switch (stat)
			{
			case 0:
				next_mode = MD_MULTI_STGSEL_SPD;
				break;
			case 1:
				next_mode = MD_MULTI_STGSEL_FLY;
				break;
			case 2:
				next_mode = MD_MULTI_STGSEL_EME;
				break;
			case 3:
				next_mode = MD_MULTI_STGSEL_EGROB;
				break;
			case 4:
				next_mode = MD_MULTI_STGSEL_FISH;
				break;
			case 5:
				next_mode = MD_MULTI_STGSEL_SHOOT;
				break;
			}

			menu_multi_change(wk, MD_MULTI_CHARSEL);
			selected_characters[0] = stat;
			gNextDialogStat = stat;
		}
	}
}

void menu_multi_battlesel(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	switch (stat)
	{
	case 0:
		menu_multi_change(wk, MD_MULTI_STGSEL_SPD);
		menu_multi_setallcharacters(CharSelChara_Sonic);
		break;
	case 1:
		menu_multi_change(wk, MD_MULTI_STGSEL_FLY);
		menu_multi_setallcharacters(CharSelChara_Tails);
		break;
	case 2:
		menu_multi_change(wk, MD_MULTI_STGSEL_EME);
		menu_multi_setallcharacters(CharSelChara_Knuckles);
		break;
	case 3:
		menu_multi_change(wk, MD_MULTI_STGSEL_EGROB);
		menu_multi_setallcharacters(CharSelChara_Amy);
		break;
	case 4:
		menu_multi_change(wk, MD_MULTI_STGSEL_FISH);
		menu_multi_setallcharacters(CharSelChara_Big);
		break;
	case 5:
		menu_multi_change(wk, MD_MULTI_STGSEL_SHOOT);
		menu_multi_setallcharacters(CharSelChara_Gamma);
		break;
	case 6:
		gNextDialogStat = 6;
		next_mode = MD_MULTI_STGSEL_TC;
		menu_multi_change(wk, MD_MULTI_CHARSEL);
		break;
	case 7:
		gNextDialogStat = 7;
		next_mode = MD_MULTI_STGSEL_VS;
		menu_multi_change(wk, MD_MULTI_CHARSEL);
		break;
	case 8:
		gNextDialogStat = 1;
		menu_multi_change(wk, MD_MULTI_MODESEL);
		break;
	}
}

void menu_multi_modesel(MultiMenuWK* wk)
{
	auto stat = GetDialogStat();

	switch (stat)
	{
	case 0:
		gNextMultiMode = multiplayer::mode::coop;
		menu_multi_change(wk, MD_MULTI_COOPSEL);
		break;
	case 1:
		gNextMultiMode = multiplayer::mode::battle;
		menu_multi_change(wk, MD_MULTI_BATTLESEL);
		break;
	case 2:
		menu_multi_change(wk, MD_MULTI_CONNECT);
		break;
	}
}

void menu_multi_online_serverselect(MultiMenuWK* wk)
{
	if (gConnectSelection == ConnectMenuSelection::AddressBar)
	{
		if (!address_text.empty() && address_text.size() < address_limit)
		{
			bool modified = false;

			if (KeyboardKeyPressed(42))
			{
				address_text.pop_back();
				modified = true;
			}

			for (int i = 0; i < 9; ++i)
			{
				if (KeyboardKeyPressed(30 + i))
				{
					address_text += (char)(49 + i);
					modified = true;
				}
			}

			if (KeyboardKeyPressed(39))
			{
				address_text += '0';
				modified = true;
			}

			for (int i = 0; i < 26; ++i)
			{
				if (KeyboardKeyPressed(4 + i))
				{
					address_text += (char)(97 + i);
					modified = true;
				}
			}

			if (KeyboardKeyPressed(51))
			{
				address_text += ':';
				modified = true;
			}

			if (KeyboardKeyPressed(55))
			{
				address_text += '.';
				modified = true;
			}

			if (KeyboardKeyPressed(45))
			{
				address_text += '-';
				modified = true;
			}

			if (KeyboardKeyPressed(56))
			{
				address_text += '/';
				modified = true;
			}

			if (modified)
			{
				FreeAddressText();
				LoadAddressText();
			}
			else
			{
				if (MenuSelectButtonsPressedM(0))
				{
					CmnAdvaModeProcedure(ADVA_MODE_TITLE_MENU);
					wk->T = 0.0f;
					wk->Stat = ADVA_STAT_FADEOUT;
				}
			}
		}
	}
	else
	{
		// Return to main menu
		if (MenuBackButtonsPressedM(0))
		{
			CmnAdvaModeProcedure(ADVA_MODE_TITLE_MENU);
			wk->T = 0.0f;
			wk->Stat = ADVA_STAT_FADEOUT;
		}

		if (MenuSelectButtonsPressedM(0))
		{
			MSG_Close(&msgc_connect);
			MSG_Open(&msgc_connect, 0, 200, 0, 0, 0xD0000020);
			MSG_Cls(&msgc_connect);
			MSG_Puts_(&msgc_connect, gConnectSelection == ConnectMenuSelection::Host ? "Waiting for players..." : "Connecting to server...");
			MSG_LoadTexture(&msgc_connect);

			PlayMenuEnterSound();
			gConnectMenuMode = ConnectMenuMode::Hub;
		}
	}

	auto button = GetPressedButtons(0);

	if (button & Buttons_Down)
	{
		gConnectSelection = ConnectMenuSelection::Client;
		PlayMenuBipSound();
	}
	else if (button & Buttons_Up)
	{
		gConnectSelection = ConnectMenuSelection::AddressBar;
		PlayMenuBipSound();
	}
	else if (button & Buttons_Left)
	{
		if (gConnectSelection == ConnectMenuSelection::Host)
		{
			gConnectSelection = ConnectMenuSelection::Client;
			PlayMenuBipSound();
		}
	}
	else if (button & Buttons_Right)
	{
		if (gConnectSelection == ConnectMenuSelection::Client)
		{
			gConnectSelection = ConnectMenuSelection::Host;
			PlayMenuBipSound();
		}
	}
}

void menu_multi_online_hub(MultiMenuWK* wk)
{
	if (!network.IsConnected())
	{
		if (GameTimer % 60 == 0)
		{
			size_t s = address_text.find_last_of(':');
			int port;
			std::string ip;

			if (s != std::string::npos)
			{
				ip = address_text.substr(0, s);
				port = std::stoi(address_text.substr(s + 1));
			}
			else
			{
				port = 80;
			}

			network.Create(gConnectSelection == ConnectMenuSelection::Host ? Network::Type::Server : Network::Type::Client, ip.c_str(), port);
		}
	}

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		player_ready[i] = network.IsPlayerConnected(i);
	}

	// Return to main menu
	if (MenuBackButtonsPressedM(0))
	{
		network.Exit();
		gConnectMenuMode = ConnectMenuMode::Select;
		LoadAddressText();
	}

	// If at least one player is there
	if (MenuSelectButtonsPressedM() && network.IsConnected() && network.GetPlayerCount() > 1)
	{
		pcount = network.GetPlayerCount();
		PlayMenuEnterSound();
		menu_multi_change(wk, MD_MULTI_MODESEL);
	}
}

void menu_multi_onlinecon(MultiMenuWK* wk)
{
	if (gConnectMenuMode == ConnectMenuMode::Select)
	{
		menu_multi_online_serverselect(wk);
	}
	else
	{
		menu_multi_online_hub(wk);
	}
}

void menu_multi_localcon(MultiMenuWK* wk)
{
	pcount = 0;

	// Return to main menu
	if (MenuBackButtonsPressedM(0) && player_ready[0] == false)
	{
		CmnAdvaModeProcedure(ADVA_MODE_TITLE_MENU);
		wk->T = 0.0f;
		wk->Stat = ADVA_STAT_FADEOUT;
	}

	// Manage input
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto press = GetPressedButtons(i);

		if (player_ready[i] == false) // Character selection
		{
			if (press & Buttons_Start)
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
			else
			{
				pcount++;
			}
		}
	}

	// If everyone is ready and at least two players are there (including player 1)
	if (MenuSelectButtonsPressedM() && player_ready[0] == true && pcount > 1)
	{
		PlayMenuEnterSound();
		menu_multi_change(wk, MD_MULTI_MODESEL);
	}
}

void menu_multi_subexec(MultiMenuWK* wk)
{
	switch (wk->SubMode)
	{
	case MD_MULTI_LOCALCON:
		menu_multi_localcon(wk);
		break;
	case MD_MULTI_ONLINECON:
		menu_multi_onlinecon(wk);
		break;
	case MD_MULTI_CHARSEL:
		menu_multi_charsel(wk);
		break;
	case MD_MULTI_MODESEL:
		menu_multi_modesel(wk);
		break;
	case MD_MULTI_COOPSEL:
		menu_multi_coopsel(wk);
		break;
	case MD_MULTI_BATTLESEL:
		menu_multi_battlesel(wk);
		break;
	case MD_MULTI_STGSEL_SPD:
		menu_multi_stgsel(wk, &MultiMenuStageSelSonicDialog, spd_level_link, 0);
		break;
	case MD_MULTI_STGSEL_FLY:
		menu_multi_stgsel(wk, &MultiMenuStageSelFlyDialog, fly_level_link, 1);
		break;
	case MD_MULTI_STGSEL_EME:
		menu_multi_stgsel(wk, &MultiMenuStageSelEmeDialog, eme_level_link, 2);
		break;
	case MD_MULTI_STGSEL_EGROB:
		menu_multi_stgsel(wk, &MultiMenuStageSelEgRobDialog, egrob_level_link, 3);
		break;
	case MD_MULTI_STGSEL_FISH:
		menu_multi_stgsel(wk, &MultiMenuStageSelBigDialog, fish_level_link, 4);
		break;
	case MD_MULTI_STGSEL_SHOOT:
		menu_multi_stgsel(wk, &MultiMenuStageSelShootDialog, shoot_level_link, 5);
		break;
	case MD_MULTI_STGSEL_TC:
		menu_multi_stgsel(wk, &MultiMenuStageSelTwinkleDialog, twinkle_level_link, 6);
		break;
	case MD_MULTI_STGSEL_VS:
		menu_multi_stgsel(wk, &MultiMenuStageSelVsDialog, vs_level_link, 7);
		break;
	case MD_MULTI_STGASK:
		menu_multi_stg_confirm(wk);
		break;
	}
}

void menu_multi_disp_controls(MultiMenuWK* wk)
{
	if (wk->SubMode != MD_MULTI_STGSEL_SPD && wk->SubMode >= MD_MULTI_ONLINECON && wk->SubMode != MD_MULTI_STGASK && wk->Stat != ADVA_STAT_FADEOUT)
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

	gHelperFunctions->PushScaleUI((uiscale::Align)(uiscale::Align::Align_Bottom | uiscale::Align::Align_Center_Horizontal), false, 1.0f, 1.0f);

	AVA_MULTI_SPRITE.p.y = 432.0f;

	AVA_MULTI_SPRITE.p.x = 135.0f;
	njDrawSprite2D_DrawNow(&AVA_MULTI_SPRITE , AVAMULTIANM_SELECT, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	AVA_MULTI_SPRITE.p.x += 130.0f;
	njDrawSprite2D_DrawNow(&AVA_MULTI_SPRITE , AVAMULTIANM_CONFIRM, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	AVA_MULTI_SPRITE.p.x += 150.0f;
	njDrawSprite2D_DrawNow(&AVA_MULTI_SPRITE , AVAMULTIANM_BACK, -64, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);

	gHelperFunctions->PopScaleUI();
}

void menu_multi_disp_charsel(MultiMenuWK* wk)
{
	if (wk->SubMode == MD_MULTI_CHARSEL && wk->Stat != ADVA_STAT_FADEOUT)
	{
		if (wk->alphaCharSel < 1.0f) wk->alphaCharSel += 0.05f;
	}
	else
	{
		if (wk->alphaCharSel > 0.0f) wk->alphaCharSel -= 0.1f;
	}

	if (wk->alphaCharSel <= 0.0f)
	{
		return;
	}

	wk->alphaCharSel = min(1.0f, wk->alphaCharSel);

	// Draw window
	ghSetPvrTexVertexColor(0x78002E67u, 0x78117BFFu, 0x78002E67u, 0x78117BFFu);
	DrawShadowWindow(120.0f, 110.0f, wk->BaseZ - 6.0f, 400.0f, 300.0f);

	SetMaterial(wk->alphaCharSel, 1.0f, 1.0f, 1.0f);

	// Draw character icons
	for (int i = 0; i < 8; ++i)
	{
		AVA_MULTI_SPRITE.p.x = 320.0f + IconPosMenuMultiCharSel[i].x;
		AVA_MULTI_SPRITE.p.y = 260.0f + IconPosMenuMultiCharSel[i].y;
		AVA_MULTI_SPRITE.p.z = wk->BaseZ - 8.0f;

		if (enabled_characters[i] == true)
		{
			njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_CHARA + i, wk->BaseZ - 8, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
		}
		else
		{
			SetMaterial(wk->alphaCharSel, 0.5f, 0.5f, 0.5f);
			njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_CHARA + i, wk->BaseZ - 8, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
			SetMaterial(wk->alphaCharSel, 1.0f, 1.0f, 1.0f);
		}

		// Draw cursor
		for (int p = PLAYER_MAX - 1; p >= 0; --p)
		{
			if (i == selected_characters[p])
			{
				if (wk->Stat != ADVA_STAT_FADEOUT && chara_ready[i] == false)
				{
					CursorColors[p].a = 0.75f + 0.25f * njSin(FrameCounter * 1000);
				}
				else
				{
					CursorColors[p].a = wk->alphaCharSel;
				}

				___njSetConstantMaterial(&CursorColors[p]);
				njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_CURSOR, wk->BaseZ + 100, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
				SetMaterial(wk->alphaCharSel, 1.0f, 1.0f, 1.0f);
			}
		}
	}
}

void menu_multi_disp_localcon(MultiMenuWK* wk)
{
	// Draw window
	ghSetPvrTexVertexColor(0x78002E67u, 0x78117BFFu, 0x78002E67u, 0x78117BFFu);
	DrawShadowWindow(120.0f, 180.0f, wk->BaseZ - 6.0f, 400.0f, 180.0f);

	SetMaterial(wk->alphaConnect, 1.0f, 1.0f, 1.0f);

	DrawSADXText(press_start_texts[TextLanguage], 210i16);

	AVA_MULTI_SPRITE.p.x = 220.0f;
	AVA_MULTI_SPRITE.p.y = 270.0f;

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		AVA_MULTI_SPRITE.p.x += 40.0f;

		if (player_ready[p])
		{
			if (wk->Stat != ADVA_STAT_FADEOUT)
			{
				CursorColors[p].a = 0.75f + 0.25f * njSin(FrameCounter * 1000);
			}
			else
			{
				CursorColors[p].a = wk->alphaConnect;
			}

			___njSetConstantMaterial(&CursorColors[p]);
		}
		else
		{
			SetMaterial(min(0.75f, wk->alphaConnect), 0.75f, 0.75f, 0.75f);
		}

		njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_ICON, wk->BaseZ + 100.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
		SetMaterial(wk->alphaConnect, 1.0f, 1.0f, 1.0f);
	}

	if (player_ready[0] == true && (player_ready[1] == true || player_ready[2] == true || player_ready[3] == true))
	{
		DrawSquareC(0xFF12B4FFu, 320.0f, 320.0f, wk->BaseZ + 100.0f, 0.715f, 0.551f);
		DrawDlgCsrSqr(0xFFu, 320.0f, 320.0f, wk->BaseZ + 200.0f, 0.715f, 0.551f);
		ghSetPvrTexMaterial(0xFFFFFFFFu);
	}
	else
	{
		DrawSquareC(0x9012B4FFu, 320.0f, 320.0f, wk->BaseZ + 100.0f, 0.695f, 0.531f);
		ghSetPvrTexMaterial(0x90FFFFFFu);
	}

	njSetTexture(&AVA_MULTI_TEXLIST);
	ghDrawPvrTexture(AVAMULTITEX_CONTINUE, 255.0f, 305.0f, wk->BaseZ + 300);
}

void menu_multi_disp_onlinecon(MultiMenuWK* wk)
{
	if (wk->alphaConnect <= 0.0f)
	{
		return;
	}

	ghSetPvrTexVertexColor(0x78002E67u, 0x78117BFFu, 0x78002E67u, 0x78117BFFu);
	DrawShadowWindow(110.0f, 180.0f, wk->BaseZ - 6.0f, 420.0f, 180.0f);

	SetMaterial(wk->alphaConnect, 1.0f, 1.0f, 1.0f);

	if (gConnectMenuMode == ConnectMenuMode::Select)
	{
		DrawSquareC(0xFF303030u, 320.0f, 240.0f, wk->BaseZ + 100.0f, 1.480f, 0.551f);
		if (gConnectSelection == ConnectMenuSelection::AddressBar) DrawDlgCsrSqr(0xFFu, 320.0f, 240.0f, wk->BaseZ + 200.0f, 1.480f, 0.551f);

		MSG_Disp(&msgc_address);

		const float buttons_pos[] = { 220.0f, 420.0f };
		const float text_pos[] = { 155.0f, 355.0f };
		const int texs[] = { AVAMULTITEX_CONNECT, AVAMULTITEX_HOST };
		for (int i = 0; i < 2; ++i)
		{
			DrawSquareC(0xFF12B4FFu, buttons_pos[i], 300.0f, wk->BaseZ + 100.0f, 0.715f, 0.551f);

			if ((int)gConnectSelection - 1 == i)
			{
				DrawDlgCsrSqr(0xFFu, buttons_pos[i], 300.0f, wk->BaseZ + 200.0f, 0.715f, 0.551f);
				ghSetPvrTexMaterial(0xFFFFFFFFu);
			}
			else
			{
				ghSetPvrTexMaterial(0x90FFFFFFu);
			}

			njSetTexture(&AVA_MULTI_TEXLIST);
			ghDrawPvrTexture(texs[i], text_pos[i], 285.0f, wk->BaseZ + 300.0f);
		}
	}
	else
	{
		MSG_Disp(&msgc_connect);

		AVA_MULTI_SPRITE.p.x = 220.0f;
		AVA_MULTI_SPRITE.p.y = 260.0f;

		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			AVA_MULTI_SPRITE.p.x += 40.0f;

			if (player_ready[p])
			{
				if (wk->Stat != ADVA_STAT_FADEOUT)
				{
					CursorColors[p].a = 0.75f + 0.25f * njSin(FrameCounter * 1000);
				}
				else
				{
					CursorColors[p].a = wk->alphaConnect;
				}

				___njSetConstantMaterial(&CursorColors[p]);
			}
			else
			{
				SetMaterial(min(0.75f, wk->alphaConnect), 0.75f, 0.75f, 0.75f);
			}

			njDrawSprite2D_ForcePriority(&AVA_MULTI_SPRITE, AVAMULTIANM_ICON, wk->BaseZ + 100.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
			SetMaterial(wk->alphaConnect, 1.0f, 1.0f, 1.0f);
		}

		if (player_ready[1] == true)
		{
			DrawSquareC(0xFF12B4FFu, 320.0f, 320.0f, wk->BaseZ + 100.0f, 0.715f, 0.551f);
			DrawDlgCsrSqr(0xFFu, 320.0f, 320.0f, wk->BaseZ + 200.0f, 0.715f, 0.551f);
			ghSetPvrTexMaterial(0xFFFFFFFFu);
		}
		else
		{
			DrawSquareC(0x9012B4FFu, 320.0f, 320.0f, wk->BaseZ + 100.0f, 0.695f, 0.531f);
			ghSetPvrTexMaterial(0x90FFFFFFu);
		}

		njSetTexture(&AVA_MULTI_TEXLIST);
		ghDrawPvrTexture(AVAMULTITEX_CONTINUE, 255.0f, 305.0f, wk->BaseZ + 300);
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
		ghSetPvrTexMaterial(0xFFFFFFFFu);
		DrawTitleBack(0.0f, 40.0f, wk->BaseZ - 8.0f, 565.0f, 42.0f);
		njSetTexture(&AVA_MULTI_TEXLIST);
		ghDrawPvrTexture(AVAMULTITEX_TITLE, 40.0f, 47.0f, wk->BaseZ - 18.0f);

		// Menu alpha
		if ((wk->SubMode == MD_MULTI_LOCALCON || wk->SubMode == MD_MULTI_ONLINECON) && wk->Stat != ADVA_STAT_FADEOUT)
		{
			if (wk->alphaConnect < 1.0f) wk->alphaConnect += 0.05f;
		}
		else
		{
			if (wk->alphaConnect > 0.0f) wk->alphaConnect -= 0.1f;
		}

		wk->alphaConnect = min(1.0f, wk->alphaConnect);

		if (wk->alphaConnect > 0)
		{
			if (selected_multi_mode != 1)
			{
				// Draw local player connection
				menu_multi_disp_localcon(wk);
			}
			else
			{
				// Draw distant player connection
				menu_multi_disp_onlinecon(wk);
			}
		}
		
		// Draw charsel background and items
		menu_multi_disp_charsel(wk);

		// Draw controls
		menu_multi_disp_controls(wk);

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
		if (saved_mode <= MD_MULTI_BATTLESEL)
		{
			menu_multi_charsel_unready();
			menu_multi_unready();
			menu_multi_change(wk, MD_MULTI_CONNECT);
			wk->Stat = ADVA_STAT_FADEIN;
			wk->T = 0.0f;
		}
		else
		{
			gNextDialogStat = 0;
			menu_multi_change(wk, saved_mode);
			wk->Stat = ADVA_STAT_KEEP;
			wk->T = 1.0f;
			wk->BaseCol = 0xFFFFFFFF;
		}

		PlayMenuMusicID(MusicIDs_JingleE);
		LoadPVM("AVA_MULTI", &AVA_MULTI_TEXLIST);
		wk->alphaConnect = wk->alphaCharSel = wk->alphaControls = 0.0f;
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

			SeqTp->awp[1].work.ub[15] = 1i8;

			njReleaseTexture(&AVA_MULTI_TEXLIST);

			AvaReleaseTexForEachMode();

			FreeAddressText();
			MSG_Close(&msgc_connect);

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

void __cdecl LoadMultiMenu(ModeSelPrmType* prmp)
{
	if (MultiMenuTp)
	{
		return;
	}

	auto tp = MultiMenuTp = CreateElementalTask(0, LEV_4, MultiMenuExec_Main);

	auto wk = (MultiMenuWK*)AllocateArray(1, sizeof(MultiMenuWK));
	wk->NextMode = prmp->NextMode;
	wk->PrevMode = prmp->PrevMode;

	tp->awp = (anywk*)wk;
	tp->disp = MultiMenuExec_Display;

	wk->BaseZ = -10000.0f;
}

void __cdecl FreeMultiMenu()
{
	if (MultiMenuTp)
	{
		FreeTask(MultiMenuTp);
		MultiMenuTp = nullptr;
	}
}

void InitMultiMenu()
{
	CreateModeFncPtrs[ADVA_MODE_EXPLAIN] = LoadMultiMenu; // Replace unused menu
	FreeModeFncPtrs[ADVA_MODE_EXPLAIN] = FreeMultiMenu;
	AvaTexLdLists[ADVA_MODE_EXPLAIN] = (int*)AvaTexLdListForMulti;
}
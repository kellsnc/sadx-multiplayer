#pragma once

#define TASKWK_PLAYERID(a) a->counter.b[0]
#define TASKWK_CHARID(a) a->counter.b[1]

struct SONIC_INPUT
{
	int angle;
	float stroke;
};

struct zxsdwstr
{
	NJS_POINT3 pos;
	xssunit lower;
	xssunit upper;
};

VoidFunc(DisplayTask, 0x40B540);
TaskFunc(Camera, 0x438090);
DataPointer(taskwk*, camera_twp, 0x3B2CBB0);
FunctionPointer(void, ds_DrawBoxFill2D, (float x, float y, float x2, float y2, float pri, int argb), 0x4071C0);
FunctionPointer(BOOL, GetZxShadowOnFDPolygon, (zxsdwstr* carry, NJS_OBJECT* object), 0x456510);
FunctionPointer(void, SetChangeGameMode, (__int16 mode), 0x413C90);
FunctionPointer(void, njDrawQuadTextureEx, (NJS_QUAD_TEXTURE_EX* quad), 0x77DE10);
VoidFunc(TempEraseSound, 0x424830);
DataPointer(int, loop_count, 0x3B1117C);
DataPointer(Uint8, ucInputStatus, 0x909FB0);
DataArray(Uint8, ucInputStatusForEachPlayer, 0x909FB4, 4);
DataArray(SONIC_INPUT, input_data, 0x3B0E368, 8);
DataArray(SONIC_INPUT, input_dataG, 0x3B0E7A0, 8);
VoidFunc(FreeQueueSound, 0x424460);
DataArray(PDS_PERIPHERAL*, per, 0x3B0E77C, 8);
DataArray(PDS_PERIPHERAL, perG, 0x3B0E7F0, 8);
CamFunc(CameraStay, 0x464DF0);
ObjectFunc(sub_425B30, 0x425B30);
ObjectFunc(sub_425BB0, 0x425BB0);
FunctionPointer(char, GetDialogStat, (), 0x432550);
VoidFunc(PlayMenuBipSound, 0x6FC8A0);
VoidFunc(PlayMenuEnterSound, 0x505810);
VoidFunc(PlayMenuBackSound, 0x505830);
FunctionPointer(void, PlayMenuMusicID, (int id), 0x505990);
FunctionPointer(void, CmnAdvaModeProcedure, (int id), 0x505B40);
FunctionPointer(int, GetFadeOutColFromT, (float t), 0x506E10);
FunctionPointer(int, GetFadeInColFromT, (float t), 0x506E40);


struct __declspec(align(4)) PanelPrmType
{
	float OfsX;
	float OfsY;
	unsigned __int8 PvrIdx;
};


enum AdvaStatEnum : __int32
{
	ADVA_STAT_REQWAIT = 0x0,
	ADVA_STAT_FADEIN = 0x1,
	ADVA_STAT_KEEP = 0x2,
	ADVA_STAT_FADEOUT = 0x3,
	ADVA_STAT_MAX = 0x4,
};

enum TrialActSbMdEnum : __int32
{
	TRIALACTSEL_SMD_STAY = 0x0,
	TRIALACTSEL_SMD_DECIDE = 0x1,
	TRIALACTSEL_SMD_NWAIT = 0x2,
	TRIALACTSEL_SMD_MAX = 0x3,
};

enum AvaStgEnum : __int32
{
	AVASTG_MR = 0x0,
	AVASTG_SS = 0x1,
	AVASTG_EC = 0x2,
	AVASTG_BEACH = 0x3,
	AVASTG_WINDY = 0x4,
	AVASTG_CASINO = 0x5,
	AVASTG_SNOW = 0x6,
	AVASTG_TWINKLEPARK = 0x7,
	AVASTG_HIGHWAY = 0x8,
	AVASTG_MOUNTAIN = 0x9,
	AVASTG_SKYDECK = 0xA,
	AVASTG_SHELTER = 0xB,
	AVASTG_RUIN = 0xC,
	AVASTG_FINAL = 0xD,
	AVASTG_SHOOTING1 = 0xE,
	AVASTG_SHOOTING2 = 0xF,
	AVASTG_SANDBOARD = 0x10,
	AVASTG_EC_C = 0x11,
	AVASTG_MG_CART = 0x12,
	AVASTG_CHAOS0 = 0x13,
	AVASTG_CHAOS2 = 0x14,
	AVASTG_CHAOS4 = 0x15,
	AVASTG_CHAOS6 = 0x16,
	AVASTG_EGGMOBILE1 = 0x17,
	AVASTG_EGGMOBILE2 = 0x18,
	AVASTG_EGGMOBILE3 = 0x19,
	AVASTG_VS_SONIC = 0x1A,
	AVASTG_VS_KNUCKLES = 0x1B,
	AVASTG_VS_MILES = 0x1C,
	AVASTG_VS_E102 = 0x1D,
	AVASTG_VS_ZERO = 0x1E,
	AVASTG_VS_E102B = 0x1F,
	AVASTG_VS_BOSS = 0x20,
	AVASTG_MAX = 0x21,
};

struct AvaStgActT
{
	unsigned __int8 Stg;
	unsigned __int8 Act;
};

struct __declspec(align(4)) AvaStgActPrmT
{
	AvaStgActT* StgActPtr;
	unsigned __int8 Num;
};

enum TitleMenuSbMdEnum : __int32
{
	TITLEMENU_SMD_STAY = 0x0,
	TITLEMENU_SMD_DECIDE = 0x1,
	TITLEMENU_SMD_STAY_S = 0x2,
	TITLEMENU_SMD_TO_DEMO = 0x3,
	TITLEMENU_SMD_TO_DEMO2 = 0x4,
	TITLEMENU_SMD_TO_MAINMENU = 0x5,
	TITLEMENU_SMD_NWAIT = 0x6,
	TITLEMENU_SMD_MAX = 0x7,
};

enum TitleMenuEnum : __int32
{
	OPTION_MENU_HOW2PLAY = 0x0,
	OPTION_MENU_CONTROLS = 0x1,
	OPTION_MENU_OPTIONS = 0x2,
	OPTION_MENU_CREDITS = 0x3,
	OPTION_MENU_BACK = 0x4,
	OPTION_MENU_MAX = 0x5,
};

struct __declspec(align(2)) TrialActSelWk
{
	AdvaStatEnum Stat;
	AdvaModeEnum PrevMode;
	AdvaModeEnum NextMode;
	float BaseZ;
	float T;
	unsigned int BaseCol;
	TrialActSbMdEnum SubMode;
	AvaStgActT NowStgAct[10];
	AvaStgActPrmT NowStgActPrm;
	AvaStgEnum Stg[33];
	unsigned __int8 StgEnaMax;
	unsigned int CsrCol;
	float CsrScale;
	int CsrAniAng;
	unsigned __int8 CsrAlpha;
	unsigned __int8 DispStg;
	/*unsigned*/ __int8 SelStg;
};


enum ChSbMdEnum : __int32
{
	CHSEL_SMD_STAY = 0x0,
	CHSEL_SMD_MOVING_L = 0x1,
	CHSEL_SMD_MOVING_R = 0x2,
	CHSEL_SMD_DECIDE = 0x3,
	CHSEL_SMD_DLG1 = 0x4,
	CHSEL_SMD_DLG2 = 0x5,
	CHSEL_SMD_DLG3 = 0x6,
	CHSEL_SMD_DLG4 = 0x7,
	CHSEL_SMD_DECIDE2 = 0x8,
	CHSEL_SMD_DECIDE2X = 0x9,
	CHSEL_SMD_NWAIT = 0xA,
	CHSEL_SMD_MAX = 0xB,
};

enum AdvDlgSelEnum : __int32
{
	ADVDLG_KAISHI = 0x0,
	ADVDLG_SETSUMEI = 0x1,
	ADVDLG_ENDING = 0x2,
	ADVDLG_CANCEL = 0x3,
	ADVDLG_MAX = 0x4,
};

struct ModeSelPrmType
{
	AdvaModeEnum PrevMode;
	AdvaModeEnum NextMode;
	AdvaModeEnum NextModeSub;
};

struct __declspec(align(4)) CharSelWk
{
	AdvaStatEnum Stat;
	AdvaModeEnum PrevMode;
	AdvaModeEnum NextMode;
	float BaseZ;
	float T;
	unsigned int BaseCol;
	int CsrAniAng;
	ChSbMdEnum SubMode;
	AvaCharEnum SelCh;
	unsigned __int8 SelFile;
	float AngT;
	__int16 Cnt;
	AdvDlgSelEnum DlgStat;
	unsigned __int8 WakeFlg;
};

struct __declspec(align(2)) TitleMenuWk
{
	AdvaStatEnum Stat;
	AdvaModeEnum PrevMode;
	AdvaModeEnum NextMode;
	AdvaModeEnum NextModeSub;
	float BaseZ;
	float T;
	unsigned int BaseCol;
	TitleMenuSbMdEnum SubMode;
	TitleMenuEnum SelMenu;
	__int16 Cnt;
	unsigned __int8 WakeFlg;
};


enum DlgPnlStyleEnum : __int32
{
	DLG_PNLSTYLE_MARU = 0x0,
	DLG_PNLSTYLE_SIKAKU = 0x1,
	DLG_PNLSTYLE_MARU4 = 0x2,
	DLG_PNLSTYLE_MARUSO = 0x3,
	DLG_PNLSTYLE_SIKAKU2 = 0x4,
	DLG_PNLSTYLE_MARU2 = 0x5,
	DLG_PNLSTYLE_MAX = 0x6,
};

struct DlgSndPrmType
{
	int OpenSnd;
	int CloseSnd;
	int MoveSnd;
	int DecideSnd;
	int CancelSnd;
};

struct __declspec(align(4)) DialogPrmType
{
	DlgPnlStyleEnum DlgStyle;
	void(__cdecl* EachDrawFnc)(int*);
	NJS_TEXLIST* PnlTlsPtr;
	PanelPrmType* PnlPrmPtr;
	DlgSndPrmType* DlgSndPrmPtr;
	unsigned int DlgColU;
	unsigned int DlgColD;
	float CntrX;
	float CntrY;
	float BaseZ;
	float SzX;
	float SzY;
	float MagX;
	float MagY;
	char CsrMax;
	char CsrCancel;
};

DataPointer(AvaStgActT, AvaCmnPrm, 0x3C5FED0);
DataPointer(ADVERTISE_WORK, AdvertiseWork, 0x3B2A2FA);
DataArray(DialogPrmType, DialogPrm, 0x7EE328, 22); // Menu dialogs
TaskFunc(title_new_exec, 0x5101A0);
TaskFunc(trial_act_sel_exec, 0x50B220);
TaskFunc(trial_act_sel_disp, 0x50B410);

DataPointer(int, unk_3C49C23, 0x3C49C23);
DataPointer(task*, SeqTp, 0x3C5E8D0);
DataPointer(task*, TrialActStelTp, 0x3C5FEE0);
DataArray(PVMEntry*, MenuTexlists, 0x10D7CB0, 5);
DataArray(void*, CreateModeFncPtrs, 0x10D7B4C, 14);
DataPointer(task*, TitleNewTp, 0x3C5FF00);
DataPointer(BOOL, Menu_CanSelect, 0x3C5E8E0);
VoidFunc(MenuLaunchNext, 0x506040);

static const void* const ChgSubModePtr = (void*)0x50B1C0;
static inline void ChgSubModeToStay_0(ModeSelPrmType* a1, task* a2)
{
	__asm
	{
		mov ecx, [a2]
		mov eax, [a1]
		call ChgSubModePtr
	}
}

struct MSGC
{
	unsigned __int8 kind;
	__int16 x;
	__int16 y;
	unsigned __int16 width;
	unsigned __int16 height;
	unsigned __int16 buf_width;
	unsigned __int16 buf_height;
	unsigned __int16 buf_width2;
	unsigned __int16 buf_height2;
	__int16 lx;
	__int16 ly;
	__int16 sx;
	__int16 sy;
	unsigned __int16 color;
	NJS_COLOR fc;
	NJS_COLOR bc;
	float scale;
	void* bitmap;
	unsigned int globalindex;
	NJS_TEXLIST texlist;
	NJS_TEXNAME texname;
	int msgc_flag;
};

DataPointer(MSGC, stru_3ABDC18, 0x3ABDC18);
VoidFunc(sub_40BC80, 0x40BC80);
FunctionPointer(void, SetMessageSettings, (MSGC* a1, __int16 x, __int16 y, int width, int height, int globalindex), 0x40E430);
FunctionPointer(void, MSG_LoadTexture2, (MSGC* a1), 0x40D2A0);
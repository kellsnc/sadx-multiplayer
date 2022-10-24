#pragma once

#define TASKWK_PLAYERID(a) a->counter.b[0] // Get player id from player's taskwk
#define TASKWK_CHARID(a) a->counter.b[1] // Get character id from player's taskwk

static constexpr int ADVA_MODE_MULTI = 0x8;

enum AvaTexLdEnum : __int32
{
	TENUM_AVA_BACK = 0x0,
	TENUM_ADV_WINDOW = 0x1,
	TENUM_AVA_CHSEL = 0x2,
	TENUM_AVA_SUUJI = 0x3,
	TENUM_AVA_CSR = 0x4,
	TENUM_AVA_DLG = 0x5,
	TENUM_AVA_FSDLG = 0x6,
	TENUM_AVA_FILESEL = 0x7,
	TENUM_AVA_FSCMN = 0x8,
	TENUM_AVA_SNDTEST = 0x9,
	TENUM_AVA_SQUARE = 0xA,
	TENUM_AVA_STNAM = 0xB,
	TENUM_AVA_TRIALACTSEL = 0xC,
	TENUM_AVA_VMSSEL = 0xD,
	TENUM_B_CHNAM = 0xE,
	TENUM_M_CHNAM = 0xF,
	TENUM_TX_CHNAM = 0x10,
	TENUM_AVA_GTITLE0 = 0x11,
	TENUM_AVA_TITLECMN = 0x12,
	TENUM_AVA_TITLE = 0x13,
	TENUM_AVA_TITLE_BACK = 0x14,
	TENUM_AVA_OPTION = 0x15,
	TENUM_AVA_EMBLEM = 0x16,
	TENUM_AVA_NEW16NO = 0x17,
	TENUM_AVA_SAN = 0x18,
	TENUM_AVA_EMBLEMVIEW = 0x19,
	TENUM_AVA_INETDEMO = 0x1A,
	TENUM_TVSETTING = 0x1B,
	TENUM_METAL = 0x1C,
	TENUM_AVA_TITLECMN_SMALL = 0x1D,
	TENUM_AVA_TITLE_DEMO = 0x1E,
	TENUM_AVA_NEW_MENU = 0x1F,
	TENUM_AVA_HELP_OPTIONS = 0x20,
	TENUM_AVA_HOW2PLAY = 0x21,
	TENUM_AVA_LB = 0x22,
	TENUM_AVA_NEW_DC = 0x23,
	TENUM_NMAX = 0x24,
	TENUM_END = 0x24,

	TENUM_NMAX_SADXPC = 30 // custom, enum is different
};

enum TGT_MODE : __int32
{
	MODE_WAIT_PLAYER = 0x0,
	MODE_JUMP = 0x2,
	MODE_HOVER = 0x3,
	MODE_FALL = 0x4,
	MODE_DAMAGE = 0x5,
	MODE_ATTACK = 0x6,
	MODE_DEATH = 0x7,
	MODE_MAX = 0x8,
};

enum : __int32
{
	ID_NO_DEFINE = 0x0,
	ID_P_PLAYABLE = 0x1,
	ID_P_NOPLAYABLE = 0x2,
	ID_P_BOSS = 0x3,
	ID_O_MOVE_S = 0x4,
	ID_O_MOVE_M = 0x5,
	ID_O_MOVE_L = 0x6,
	ID_O_SWING = 0x7,
	ID_O_JUMPPANEL = 0x8,
	ID_O_SNOWJUMP = 0x9,
	ID_E_CART = 0xA,
	ID_I_RING = 0xB,
	ID_C_OTTOTTO = 0xC,
	ID_C_BIGWJUMP = 0xD,
	ID_END = 0xE,
};

struct Mass
{
	NJS_POINT3 trans[12];
	NJS_POINT3 velocity[12];
	NJS_POINT3 acceleration[12];
};

struct String
{
	Mass mass;
	unsigned int flag;
};

struct erctrlstr
{
	int command;
	NJS_POINT3 pos0;
	NJS_POINT3 pos1;
};

struct OCMDATA
{
	taskwk* twp;
	int(__cdecl* breakfunc)(NJS_POINT3*, NJS_POINT3*, taskwk*);
	char num;
	char flag;
	char type;
	char sflag;
};

struct CAM_ANYPARAM
{
	NJS_POINT3 camAnyParamPos;
	NJS_POINT3 camAnyParamTgt;
	Angle3 camAnyParamAng;
	int camAnyTmpSint32[2];
	float camAnyTmpFloat[2];
};

struct NES_LIB_PARAM
{
	const char* Company;
	const char* AppName;
	const char* DVDRoot;
	Bool Windowed;
	Sint32 GraphMode;
	Sint32 Frame;
	Sint32 Count;
	Float AspectX;
	Float AspectY;
	Uint32 PolyBuff_QUAD;
	Uint32 PolyBuff_CS;
	Uint32 PolyBuff_CS_N;
	Uint32 PolyBuff_CS_UVN;
	Uint32 PolyBuff_CS_UVN_N;
	Uint32 PolyBuff_CS_D8;
	Uint32 PolyBuff_CS_N_D8;
	Uint32 PolyBuff_CS_UVN_D8;
	Uint32 PolyBuff_CS_UVN_N_D8;
	Sint32 ZSearchMode;
	Uint32 Width;
	Uint32 Height;
};

struct strCamCartData
{
	NJS_POINT3 transCam;
	NJS_POINT3 transTgt;
};

struct gachamotionwk
{
	NJS_POINT3 spd;
	NJS_POINT3 acc;
	Angle3 ang_aim;
	Angle3 ang_spd;
	float rad;
	float height;
	float weight;
	float frame;
	float framespd;
	float thrust;
	int etimer;
	int playerang;
	int playerangdiff;
	float playerdist;
	int prio;
};

struct PATHCAMERAWORK
{
	int modeflag;
	float fForwardPathDist;
	float fBackPathDist;
	float fForwardPathMul;
	float fBackPathMul;
	float fSonicSize;
	float fCamRotMul;
	float fCamMovMul;
	__int16 nChangeFrame;
	unsigned __int8 ucAdjustType;
	unsigned __int8 ucAdjustTypeAfter;
	float fPathCameraRangeIn;
	float fPathCameraRangeOut;
	int nTopPathGap;
	int nBottomPathGap;
};

struct PATHCAMERA1WORK
{
	int modeflag;
	pathtag* pPathTag;
	float fForwardPathDist;
	float fBackPathDist;
	float fForwardPathMul;
	float fBackPathMul;
	float fMaxSonicDist;
	float fMinSonicDist;
	float fSonicSize;
	float fCameraSize;
	float fMaxCameraAcc;
	float fCameraAccMul;
	float fPathCameraRangeOut;
	int nPlayer;
	int nTopPathGap;
	int nBottomPathGap;
	int nChangeFrame;
	NJS_POINT3 vecCamOfs;
	NJS_POINT3 vecPathOfs;
	Angle3 angCamRot;
	int angMaxCamSpd;
	float angCamSpdMul;
};

struct beamhitstr
{
	float reach;
	NJS_POINT3 pos;
	NJS_POINT3 vec;
	float dist;
	NJS_POINT3 hitpos;
	xssunit hitinfo;
};

struct CLEAR_TIME
{
	char m;
	char s;
	char f;
};

struct BEST3_WEIGHT
{
	__int16 weight[3];
};

struct BEST3_SCORE
{
	int score[3];
};

struct BEST3_CART
{
	CLEAR_TIME time[3];
	CLEAR_TIME lap[2];
};

struct BEST3_TIME
{
	CLEAR_TIME time[3];
};

struct SEQ_DATA
{
	char time;
	char s8pad;
	__int16 seqno;
	__int16 sec;
	__int16 nextsec;
	unsigned __int16 stage;
	unsigned __int16 destination;
};

struct BIGGEST_WEIGHT
{
	__int16 weight;
	__int16 type;
};

struct SAVE_DATA
{
	int code;
	int totalTime;
	int bestScore[32];
	CLEAR_TIME bestTime[28];
	BEST3_WEIGHT bestWeight[4];
	BIGGEST_WEIGHT biggestWeight[4];
	__int16 bestRing[32];
	BEST3_SCORE miniShooting[2];
	BEST3_SCORE miniShooting2[2];
	BEST3_SCORE miniSnowBoard[2];
	BEST3_SCORE miniSandBoard[2];
	BEST3_SCORE miniMole;
	BEST3_CART miniCart[6];
	BEST3_TIME bossGame[6];
	char emblem[17];
	char option;
	char charenge[7];
	unsigned __int8 lastPlayer;
	unsigned __int8 vibration;
	char dummy;
	unsigned __int16 lastStage;
	unsigned __int16 lastAction;
	char seqVars[64];
	char evExecFlag[64];
	char scPlayableFlag[8];
	SEQ_DATA seqTable[8];
	char flgCompletedActionStage[8][43];
	unsigned __int8 MissionData[60];
	unsigned int u32TotalRing;
	int M_bestScore[10];
	CLEAR_TIME M_bestTime[10];
	__int16 M_bestRing[10];
	char Align[2];
	BEST3_SCORE M_miniSnowBoard;
	BEST3_SCORE M_miniSandBoard;
	BEST3_CART M_miniCart;
	BEST3_TIME M_bossGame;
	int M_emblem;
};

struct colaround
{
	taskwk* twp;
	float dist;
};

DataArray(SAVE_DATA, SaveData, 0x3B2B3A8, 3);
FunctionPointer(void, njDrawQuadTextureEx, (NJS_QUAD_TEXTURE_EX* quad), 0x77DE10);
VoidFunc(TempEraseSound, 0x424830);
VoidFunc(FreeQueueSound, 0x424460);
ObjectFunc(sub_425B30, 0x425B30);
ObjectFunc(sub_425BB0, 0x425BB0);
DataPointer(int, Cart_demo_flag, 0x3C4ACA8); // Status of
DataArray(NJS_TEXANIM, cartsprite_score, 0x3C5D558, 13); // simply "score" in symbols
VoidFunc(InitActionScore, 0x427EF0);
DataPointer(BOOL, boolOneShot, 0x3C52464);
FunctionPointer(bool, CheckEditMode, (), 0x4258F0);
FunctionPointer(void, CameraFilter, (task* tp), 0x436CD0);
FunctionPointer(void, DrawCharacterShadow, (taskwk* twp, shadowwk* swp), 0x49F0B0);
VoidFunc(MakeLandCollLandEntryALL, 0x43B580);
DataPointer(int, RandomRingNum, 0x3C5AB30);
DataPointer(NJS_POINT3, explosion_pos, 0x3C5AB24);
DataPointer(Float, explosion_r, 0x3C5A9D0);
FunctionPointer(float, DrawShadow_, (Angle3* ang, NJS_POINT3* pos, float scl), 0x49EF30);
FunctionPointer(int, GetGlobalTime, (), 0x4261B0);
VoidFunc(setRainEffect, 0x546320);
VoidFunc(C4SuimenYurashiSet, 0x5535D0);
VoidFunc(C4LeafSetOld, 0x554480);
VoidFunc(InitIndirectEffect3D_Bossegm1, 0x5723E0);
DataArray(_OBJ_LANDENTRY, ri_landentry_buf, 0x3B2E518, 128);
DataPointer(int, ri_landentry_nmb, 0x3B36D3C);
VoidFunc(dsEditLightInit, 0x40AEE0);
FunctionPointer(void, DrawLineV, (NJS_VECTOR*, NJS_VECTOR*), 0x412990);
FunctionPointer(void, Set3DPositionPCM, (int, float, float, float), 0x4102C0);
TaskFunc(relbox_switch_draw, 0x46A8D0);
DataPointer(int, MRaceResult, 0x3C53A94);
DataPointer(NJS_SPRITE, Spr_MRaceDisp, 0x3C53A98);
FunctionPointer(task*, SetDialogTask, (), 0x432C60);
DataPointer(pathtag, pathtag_hw1_heli0807, 0x26A72D4);
DataPointer(NJS_OBJECT, object_turnasi_oya_koa, 0x267D3B4);
DataPointer(NJS_OBJECT, object_turnasi_oya_kob, 0x267C7AC);
DataPointer(BOOL, tornade_flag, 0x3C5D670);
FunctionPointer(BOOL, NearTornade, (NJS_POINT3* pos, Float* dist), 0x4BA860);
DataPointer(NJS_OBJECT, object_goaleme_blue_blue, 0xC3FDA0);
DataPointer(NJS_OBJECT, object_goaleme_white_white, 0xC3F050);
DataPointer(NJS_OBJECT, object_goaleme_green_green, 0xC3E300);
FunctionPointer(int, CheckWhichJumpPanelKicked, (), 0x4B84D0);
DataPointer(unsigned __int8, NadareStart, 0x3C5E200);
DataPointer(uint8_t, ccsi_flag, 0x3C4ABB4);
DataArray(CUSTOM_OBJ, fan_model_big, 0x88C03C, 2);
DataPointer(NJS_OBJECT, object_b_cart_cart_cart, 0x38A9130);
DataArray(CUSTOM_OBJ, fan_model_e102, 0x88C014, 2);
DataPointer(NJS_OBJECT, object_e_cart_cart_cart, 0x38AB250);
DataArray(CUSTOM_OBJ, fan_model, 0x88C02C, 2);
DataPointer(NJS_OBJECT, object_sarucart_sarucart_sarucart, 0x38BAAA4);
DataPointer(NJS_OBJECT, object_sarucart_saru_body_saru_body, 0x38B8780);
DataArray(NJS_MODEL_SADX*, cart_model, 0x38F4FC8, 32);
DataPointer(NJS_OBJECT*, e102_arm, 0x3C53F80);
DataArray(NJS_COLOR, e102laser_color, 0x91C648, 2);
DataArray(NJS_POINT3, e102laser_pos, 0x3C53F68, 2);
DataPointer(NJS_POINT3COL, e102laser_p3c, 0x3C53B60);
DataArray(int, GblMenuTbl, 0x7EF8E8, 6);
DataPointer(int, unk_3C49C23, 0x3C49C23);
VoidFunc(PlayMenuBipSound, 0x6FC8A0);
VoidFunc(PlayMenuEnterSound, 0x505810);
VoidFunc(PlayMenuBackSound, 0x505830);
FunctionPointer(BOOL, IsMiniGameMenuEnabled, (), 0x506460);
FunctionPointer(void, PlayMenuMusicID, (MusicIDs id), 0x505900);
FunctionPointer(void, PlayMenuMusic, (int id), 0x505990);
DataPointer(MSGC, jimakumsgc, 0x3ABDC18);
DataPointer(void*, jimakubuf, 0x3ABDF84);
FunctionPointer(int, GetSwitchOnOff, (unsigned int ID), 0x4CB4F0);
TaskFunc(ObjShelterFade, 0x5ABD00);
DataPointer(NJS_SPRITE, sprite_fade, 0x19C7C24);
TaskFunc(ObjShelterTunnelscroll, 0x5AC4A0);
TaskFunc(ObjShelterNo2cargo, 0x5ABED0);
FunctionPointer(void, SetSwitchOnOff, (unsigned int ID, int OnOff), 0x4CB4C0);
VoidFunc(CreateTunnelcol, 0x5AC2C0);
TaskFunc(EffectSpark, 0x4CE830);
TaskFunc(PathKassha, 0x603640);
TaskFunc(KasshaDisplayer, 0x603590);
DataPointer(int, ke_ongame_flg, 0x3C52AD8);
DataPointer(int, found_feme_nmb, 0x3C52C04);
FunctionPointer(BOOL, isMissionClearDisp, (), 0x414FE0);
TaskFunc(KnucklesLaterSE, 0x474F50);
TaskFunc(FragmEmeraldDigDisplay, 0x4A31D0);
FunctionPointer(int, Knuckles_Status, (int plnmb), 0x475600);
FunctionPointer(void, CreateKiranR, (NJS_POINT3* pos, NJS_POINT3* velo, float scl, Angle ang), 0x4BAD80);
TaskFunc(Knuckles_KakeraGame, 0x476440);
FunctionPointer(int, GetMRaceLevel, (), 0x47C200);
DataPointer(int, MRaceLevel, 0x3C53AB8);
DataPointer(int, MRaceStageNumber, 0x3C539EC);
TaskFunc(InitMoble2PControl, 0x47D8C0);
TaskFunc(InitSonic2PControl, 0x47D820);
TaskFunc(late_DispMilesMeter2P, 0x47C260);
DataPointer(NJS_POINT3, VecTemp0, 0x3B0F140);
DataPointer(NJS_POINT3, VecTemp1, 0x3B0F12C);
DataPointer(int, slJudge, 0x3B0EF44);
DataArray(int*, MPT_MRaceEachStage, 0x91C0E0, 10);
DataPointer(taskwk*, rd_mountain_twp, 0x03C80F84);
TaskFunc(exitFishingLure, 0x46C8D0);
TaskFunc(dispFishingLure, 0x470580);
FunctionPointer(int, BGM_Replay, (), 0x4256E0);
FunctionPointer(BOOL, BigSetPosition, (NJS_POINT3* p, NJS_POINT3* v, Angle3* a, float r), 0x46F130);
TaskFunc(BigDisplayHit, 0x46EBC0);
TaskFunc(BigDisplayStatus, 0x470090);
TaskFunc(dispBigKaeru, 0x7A6BB0);
TaskFunc(BigDisplayFishWeight, 0x4701A0);
DataPointer(task*, pRd_Master, 0x3B0EFD8);
FunctionPointer(void, ERobStart, (erctrlstr* cmd), 0x4B3EB0);
TaskFunc(Eggrob_Init, 0x4D3ED0);
VoidFunc(EV_NpcMilesStandByOff, 0x42CE20);
FunctionPointer(void, SetFreeCameraMode, (int sw), 0x4348A0);
DataPointer(SaveFileData, sd, 0x3C52558);
DataPointer(float, distance, 0x3C52AC8);
DataPointer(float, distancep, 0x3C52ACC);
FunctionPointer(float, BigChkHeavyWeight, (), 0x46F7D0);
FunctionPointer(int, getLureKind, (), 0x46C870);
FunctionPointer(void, String_IniEasy, (String* ___this, const NJS_POINT3* v0, const NJS_POINT3* vN), 0x4BF860);
FunctionPointer(void, String_Exe, (String* ___this, const NJS_POINT3* v0, const NJS_POINT3* vN, int mode), 0x4BFCA0);
FunctionPointer(void, String_Dsp, (String* ___this), 0x4BFD40);
DataPointer(float, No2CargoSpd, 0x3C72E5C);
FunctionPointer(void, CreateSnow, (NJS_POINT3* pos, NJS_POINT3* velo, float scl), 0x4B9B10);
FunctionPointer(void, DrawSquareC, (unsigned int col, float x, float y, float z, float magx, float magy), 0x431CB0);
FunctionPointer(void, DrawDlgCsrSqr, (unsigned __int8 col_alpha, float x, float y, float z, float magx, float magy), 0x431BB0);
FunctionPointer(void, CreateWater, (NJS_POINT3* pos, NJS_POINT3* velo, float scl), 0x4B9540);
TaskFunc(CartGetOffPlayer, 0x798C60);
TaskFunc(EnemyAir, 0x4AA340);
TaskFunc(EffectSkyScope, 0x5F1AA0);
TaskFunc(dispSkyScope, 0x5F19F0);
DataPointer(NJS_OBJECT, object_sky_scope_a_scope_a, 0x214E4B8);
TaskFunc(ObjectSkydeck_crane_hang_Draw, 0x5F2F50);
DataPointer(Angle, cloud_roll, 0x3C7F05C);
DataPointer(Angle, cloud_pitch, 0x3C7F028);
FunctionPointer(void, SetUserGravityXZ, (int ang, int angz), 0x43B4C0);
DataPointer(NJS_OBJECT, object_cl_uki_cl_uki, 0x1A29EEC);
DataPointer(NJS_OBJECT, object_mdlelv1_cl_elvtr_cl_elvtr, 0x1A3D74C);
DataPointer(NJS_ACTION, action_fun_funflot, 0x1A3037C);
FunctionPointer(int, PCheckJump, (taskwk* twp), 0x43BF40);
DataPointer(int, bWake, 0x912DF0); // Timer toggle
DataPointer(int16_t, ssNextStageNumber, 0x3B22DF0);
DataPointer(int16_t, ssNextActNumber, 0x3B22E18);
VoidFunc(late_execCancel, 0x403E60);
VoidFunc(sub_437100, 0x437100);
DataPointer(_OBJ_CAMERAPARAM, cameraParam, 0x3B2CB7C);
DataPointer(NJS_PLANE, plCollision, 0x915094);
FunctionPointer(void, DispTask, (int level), 0x40B4F0);
FunctionPointer(Bool, ChkGameMode, (), 0x414D90);
DataPointer(Sint16, ssAct, 0x3B2CAC8);
DataPointer(NES_LIB_PARAM, nesparam, 0x892944);
DataPointer(Float, clipZ_near, 0x90082C);
DataPointer(Float, clipZ_far, 0x900830);
DataPointer(NJS_CACTION*, pCameraAction, 0x3B2C950);
DataPointer(Float, fActionFrame, 0x3B2CA6C);
DataPointer(Sint32, camera_mode, 0x3B2C6C4); // no symbol name
FastcallFunctionPointer(Float, njDistancePL2PL, (NJS_PLANE* pl1, NJS_PLANE* pl2, NJS_LINE* l), 0x7887D0);
FunctionPointer(void, ListGroundForCollision, (Float xPosition, Float yPosition, Float zPosition, Float fRadius), 0x43ACD0);
DataPointer(CCL_INFO, camera_sphere_info, 0x9BE8A8);
FunctionPointer(Bool, IsThisTaskPlayer, (task* tp), 0x441AD0);
VoidFunc(SetPlayer, 0x4157C0);
FunctionPointer(void, EV_Load2, (int num), 0x42FA30);
FunctionPointer(void, HoldOnIcicleP, (Uint8 pno, task* ttp), 0x441210);
FunctionPointer(void, ChangeModePlaceWithTrampolineP, (Uint8 pno, Sint8 time), 0x4417F0);
FunctionPointer(void, SetTrampolineVelocityP, (Uint8 pno, Sint8 time, Float x, Float y, Float z), 0x441390);
DataArray(PL_ACTION, big_action, 0x3C556A0, 105);
VoidFunc(_advertise_prolog, 0x5034A0);
DataArray(strCamCartData, camCartData, 0x91B670, 3);
VoidFunc(SetCartCameraDemo, 0x4DB4E0);
FunctionPointer(taskwk*, CCL_IsHitPlayerWithNum, (taskwk* twp, Sint32 info_num), 0x41CC60);
FunctionPointer(void, CreateHitmark, (NJS_POINT3* pos, Float scl), 0x4CAB40);
DataPointer(NJS_OBJECT, object_youUP_yogun_yogun, 0x2484D5C);
DataArray(PATHCAMERA1WORK*, pathcamera1works, 0x97EC40, 11);
CamFunc(PathCamera1, 0x4653E0);
CamFunc(PathCamera2Core, 0x465E20);
FunctionPointer(Sint32, PC1_SearchNearPath, (NJS_POINT3* posttgt, PATHCAMERA1WORK* pPathCamera1Work), 0x465190);
FunctionPointer(Bool, PC1_PathMoveScan, (int* nowframe, NJS_POINT3* posonpath, NJS_POINT3* postgt, NJS_POINT3* vecnear, PATHCAMERA1WORK* pPathCamera1Work), 0x464F70);
FunctionPointer(Bool, CL_ColPolBeamHit, (beamhitstr* bhsp), 0x4546E0);
TaskFunc(InitPathworkCamera, 0x4BBF80);
FunctionPointer(Bool, MirenSoundSetStartBgmTask, (Sint32 bgmId, Sint32 time), 0x79E180);
VoidFunc(MirenSoundFinishBgm, 0x79E1C0);
VoidFunc(SandObjSetManageTask, 0x597E90);
FunctionPointer(void, ObjCasino_ExecGColl2, (task* tp, Float _rad2, NJS_MODEL_SADX* pmodel), 0x5DD320);
TaskFunc(ObjectCasinoTutuAEnd, 0x5D43C0);
DataPointer(Float, SAL_kowarezofallspeed, 0x1E74F00);
DataPointer(Float, SAL_kowarezofraction, 0x1E74F04);
DataPointer(Float, SAL_kowarezocrashspd, 0x1E74F08);
DataPointer(Float, SAL_kowarezocrashrad, 0x1E74F0C);
DataPointer(Float, SAL_kowarezocrashradspd, 0x1E74F10);
DataPointer(Sint32, SAL_kowarezocrashtime, 0x1E74F14);
DataPointer(Sint32, SAL_kowarezocrashtimecomptime, 0x1E74F18);
DataPointer(Float, SAL_kowarezocrashrotspd, 0x1E74F1C);
DataPointer(Sint32, SAL_kowarezocrashrefspd, 0x1E74F20);
DataPointer(Sint32, SAL_kowarezocrashrefspdmul, 0x1E74F24);
DataPointer(Float, SAL_kowarezocrashrefbdspd, 0x1E74F28);
DataPointer(Float, SAL_kowarezohitsmokesclmul, 0x1E74F2C);
DataPointer(Float, SAL_kowarezocrashsmokescl, 0x1E74F30);
DataPointer(Uint16, numDisplayEntry, 0x3B32D2C);
VoidFunc(InitScore, 0x427EE0);
FunctionPointer(BOOL, GetEachCharEnableForTrial, (Sint32 chnum), 0x506700);
DataArray(Sint8[STAGE_NUMBER], flgCompletedActionStage, 0x3B18250, 8);
CamFunc(CameraEventPoint, 0x464A00);
VoidFunc(calcvsyncsyoriochi, 0x413920);
DataPointer(Uint32, gu32loop_count, 0x3B1118C);
DataPointer(Uint32, adva_loop_count, 0x3B11184);
DataArray(colaround, around_ring_list_p0, 0x3B27470, 257);
DataArray(colaround, around_enemy_list_p0, 0x3B259C0, 657);
DataArray(colaround, around_ring_list_p1, 0x3B23298, 257);
DataArray(colaround, around_enemy_list_p1, 0x3B242F8, 657);
DataPointer(Uint16, arl_num0, 0x3B27048);
DataPointer(Uint16, ael_num0, 0x3B23288);
DataPointer(Uint16, arl_num1, 0x3B27C7C);
DataPointer(Uint16, ael_num1, 0x3B240A8);
FunctionPointer(void, PPutHeldObject, (taskwk* twp, taskwk* htwp, playerwk* pwp), 0x4421F0);
FunctionPointer(Float, GetUnitVector, (NJS_POINT3* vec), 0x459AC0);
FunctionPointer(void, HangDownFromPathP, (Uint8 pno, pathtag* pp, Sint32 point), 0x440E90);
FunctionPointer(Float, orGetWaveHeight, (Float pos_x, Float pos_z), 0x4F8A00);
DataArray(Float, hasi_a_posy, 0x3C5E7F4, 2);
DataArray(Float, hasi_b_posy, 0x3C5E790, 25);
DataArray(Float, hasi_b_angz, 0x3C5E800, 25);

TaskFunc(drawEffectChaos0EffectB, 0x7ACCB0);
TaskFunc(drawEffectChaos0LightParticle, 0x7ACB30);
TaskFunc(dispEffectChaos0AttackA, 0x7AC9D0);
TaskFunc(CircleLimit, 0x7AF300);

DataPointer(char, chaos_nextmode, 0x3C5A7EC);
DataPointer(char, chaos_reqmode, 0x3C5A7ED);
DataPointer(char, chaos_oldmode, 0x3C5A7E1);
DataPointer(char, chaos_punch_num, 0x3C5A7EE);
DataPointer(char, chaos_pole_punch_num, 0x3C63D04);
FunctionPointer(void, setDrop, (taskwk* twp, int num, float size, float y_offset), 0x7AD1C0);
FunctionPointer(void, SetEffectPunchTameParticle, (NJS_POINT3* pos, int num), 0x7AD8E0);
FunctionPointer(void, setShakeHeadParam, (float param), 0x549BB0);
DataPointer(float, flt_3C63CEC, 0x3C63CEC);
FunctionPointer(int, EH_PosPlayerCheck, (), 0x573310); //inlined in symbols
FunctionPointer(void, sub_575190, (Egm1MissilesPrm* a1), 0x575190);
FunctionPointer(signed int, ChkPlayerCanHold, (task* a1, unsigned __int8 PlayerNumber), 0x4C9FE0);
TaskFunc(CalcTotalScore, 0x42BEE0);
FunctionPointer(void, SetAutoPilotForBreak, (uint8_t pNum), 0x440F70);
FunctionPointer(void, SetLookingAngleP, (uint8_t pnum, Angle3* a2), 0x441040);
FunctionPointer(void, PResetPosition, (taskwk* a1, motionwk2* a2, playerwk* a3), 0x43EE70);

FunctionPointer(void, Knux_RunsActions, (taskwk* twp, motionwk2* mwp, playerwk* pwp), 0x478020);
TaskFunc(dispKnuEffectChargeUpStay, 0x4C0FC0);
FunctionPointer(void, KnuEffectDrawTsubu, (NJS_POINT3* v, float r), 0x4C0F20);
TaskFunc(KnuEffectChargeUpStay, 0x4C0FF0);
FunctionPointer(void, KnuEffectPutChargeComp, (NJS_VECTOR* position, float alpha), 0x4C1330);
FunctionPointer(void, SetLocalPathCamera, (pathtag* path, Sint8 mode, int timer), 0x469300);
DataPointer(pathtag, pathtag_s_camera, 0x919BF4);

static const void* const pLockingOnTargetEnemy2Ptr = (void*)0x7984B0;
static inline void pLockingOnTargetEnemy2(motionwk2* mwp, taskwk* twp, playerwk* pwp)
{
	__asm
	{
		push[pwp]
		mov edi, [twp]
		mov ebx, [mwp]
		call pLockingOnTargetEnemy2Ptr
		add esp, 4
	}
}

static const void* const EffBarrierPosSetPtr = (void*)0x4B9CE0;
static inline void EffBarrierPosSet(taskwk* twp, taskwk* ptwp)
{
	__asm
	{
		mov esi, [ptwp]
		mov eax, [twp]
		call EffBarrierPosSetPtr
	}
}

static const void* const DrawActionBPtr = (void*)0x406C40;
static inline void DrawActionB(NJS_ACTION* action, float frame, int flgs, float clpScl, void* drwMdlFnc)
{
	__asm
	{
		push[drwMdlFnc]
		push[clpScl]
		push dword ptr[flgs]
		push[frame]
		mov eax, [action]
		call DrawActionBPtr
		add esp, 16
	}
}

static const void* const SpinnaDrawPtr = (void*)0x4AFB40;
static inline void SpinnaDraw(taskwk* twp, enemywk* ewp)
{
	__asm
	{
		mov esi, [ewp]
		mov ebx, [twp]
		call SpinnaDrawPtr
	}
}

static const void* const PondDrawPtr = (void*)0x7AA2F0;
static inline void PondDraw(taskwk* twp, enemywk* ewp)
{
	__asm
	{
		mov ebx, [ewp]
		mov esi, [twp]
		call PondDrawPtr
	}
}

static const void* const HeliPathPtr = (void*)0x6134A0;
static inline void HeliPath(taskwk* twp, pathtag* tag, pathinfo info)
{
	__asm
	{
		sub esp, 50h
		mov ecx, 14h
		lea esi, [info];
		mov edi, esp
			rep movsd
			mov edx, [tag]
			mov esi, [twp]
			call HeliPathPtr
			add esp, 50h
	}
}

static const void* const HeliPosCopyPlayerPtr = (void*)0x613540;
static inline void HeliPosCopyPlayer(taskwk* PlayerWork, taskwk* twp)
{
	__asm
	{
		mov ebx, [twp]
		mov esi, [PlayerWork]
		call HeliPosCopyPlayerPtr
	}
}

static const void* const SpringAnglePtr = (void*)0x566FE0;
static inline void SpringAngle(Angle* pAng, Angle* pAngSpd)
{
	__asm
	{
		mov edi, [pAng]
		mov esi, [pAngSpd]
		call SpringAnglePtr
	}
}

static const void* const springAnglePtr = (void*)0x5A36E0;
static inline void springAngle(Angle angTarget, Angle* pAng, Angle* pAngSpd, Angle angMaxSpd, float fSpringRate, float fSpringReduction)
{
	__asm
	{
		push[fSpringReduction]
		push[fSpringRate]
		push[angMaxSpd]
		mov esi, [pAngSpd]
		mov edi, [pAng]
		mov eax, [angTarget]
		call springAnglePtr
		add esp, 12
	}
}

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

static const void* const rdTwinkleInitPtr = (void*)0x61CB10;
static inline void rdTwinkleInit(task* tp)
{
	__asm
	{
		mov edi, [tp]
		call rdTwinkleInitPtr
	}
}

static const void* const checkSkySinkRaneColliPtr = (void*)0x5F5DF0;
static inline void checkSkySinkRaneColli(taskwk* twp)
{
	__asm
	{
		mov esi, [twp]
		call checkSkySinkRaneColliPtr
	}
}

static const void* const AirDrawPtr = (void*)0x4A9180;
static inline void AirDraw(taskwk* twp, enemywk* ewp)
{
	__asm
	{
		mov edi, [ewp]
		mov esi, [twp]
		call AirDrawPtr
	}
}

static const void* const MissleDrawPtr = (void*)0x4A8800;
static inline void MissleDraw(taskwk* twp, enemywk* ewp)
{
	__asm
	{
		push ewp
		mov esi, [twp]
		call MissleDrawPtr
		add esp, 4
	}
}

static const void* const checkSkyEVColliPtr = (void*)0x5F5900;
static inline void checkSkyEVColli(taskwk* twp)
{
	__asm
	{
		mov esi, [twp]
		call checkSkyEVColliPtr
	}
}

static const void* const RoboDrawPtr = (void*)0x4A4B30;
static inline void RoboDraw(taskwk* twp, enemywk* ewp)
{
	__asm
	{
		mov esi, [ewp]
		mov edi, [twp]
		call RoboDrawPtr
	}
}

static const void* const RoboHeadDrawPtr = (void*)0x4A4110;
static inline void RoboHeadDraw(taskwk* twp, enemywk* ewp)
{
	__asm
	{
		push ewp
		mov eax, [twp]
		call RoboHeadDrawPtr
		add esp, 4
	}
}

static const void* const RoboHeadUpPtr = (void*)0x4A4490;
static inline void RoboHeadUp(taskwk* twp, enemywk* ewp)
{
	__asm
	{
		mov edi, [ewp]
		mov esi, [twp]
		call RoboHeadUpPtr
	}
}

static const void* const RoboHeadCaptureBeamPtr = (void*)0x4A43A0;
static inline BOOL RoboHeadCaptureBeam(task* tp)
{
	__asm
	{
		mov eax, [tp]
		call RoboHeadCaptureBeamPtr
	}
}

static const void* const sub_46E940Ptr = (void*)0x46E940;
static inline void sub_46E940(NJS_POINT3* src, NJS_POINT3* dst)
{
	__asm
	{
		mov edi, [dst]
		mov esi, [src]
		call sub_46E940Ptr
	}
}

static const void* const CS_CheckCollisionPlayer2NoWaterPtr = (void*)0x4D4C50;
static inline Bool CS_CheckCollisionPlayer2NoWater(CCL_INFO* pinf, taskwk* ctwp)
{
	Bool result;
	__asm
	{
		mov esi, [ctwp]
		mov edi, [pinf]
		call CS_CheckCollisionPlayer2NoWaterPtr
		mov result, eax
	}
	return result;
}

static const void* const SetRouteDeletaPtr = (void*)0x573380;
static inline void SetRouteDelta_0(bossextwk* egmwk, float a2)
{
	__asm
	{
		push[a2]
		mov esi, [egmwk]
		call SetRouteDeletaPtr
		add esp, 4
	}
}

static const void* const CreateCrashSmokePtr = (void*)0x5C3360;
static inline void CreateCrashSmoke(taskwk* twp, NJS_POINT3* point, Float scl)
{
	__asm
	{
		push[scl]
		push[point]
		mov esi, [twp]
		call CreateCrashSmokePtr
		add esp, 8
	}
}

static const void* const E102CheckInputPtr = (void*)0x480870;
static inline int E102CheckInput(playerwk* pwp, taskwk* data, motionwk2* data2)
{
	int result;
	__asm
	{
		push[data2]
		mov esi, [data]
		mov edi, [pwp]
		call E102CheckInputPtr
		add esp, 4
		mov result, eax
	}
	return result;
}

static const void* const E102CheckStopPtr = (void*)0x480EE0;
static inline int E102CheckStop(taskwk* a1, playerwk* a2)
{
	int result;
	__asm
	{
		mov eax, [a2]
		mov ecx, [a1]
		call E102CheckStopPtr
		mov result, eax
	}
	return result;
}

static const void* const E102CheckJumpPtr = (void*)0x480730;
static inline int E102CheckJump(playerwk* a1, taskwk* a2)
{
	int result;
	__asm
	{
		mov ebx, [a2]
		mov eax, [a1]
		call E102CheckJumpPtr
		mov result, eax
	}
	return result;
}

static const void* const AmyCheckInputPtr = (void*)0x487810;
static inline int AmyCheckInput(playerwk* pwp, motionwk2* data2, taskwk* data)
{
	int result;
	__asm
	{
		mov esi, [data]
		mov edi, [data2]
		mov ecx, [pwp]
		call AmyCheckInputPtr
		mov result, eax
	}
	return result;
}

static const void* const AmyCheckStopPtr = (void*)0x4885B0;
static inline int AmyCheckStop(taskwk* a1, playerwk* a2)
{
	int result;
	__asm
	{
		mov ecx, [a2]
		mov eax, [a1]
		call AmyCheckStopPtr
		mov result, eax
	}
	return result;
}

static const void* const SonicGetPillarRotSpeedPtr = (void*)0x45AB70;
static inline int SonicGetPillarRotSpeed(playerwk* a1)
{
	int result;
	__asm
	{
		mov eax, [a1]
		call SonicGetPillarRotSpeedPtr
		mov result, eax
	}
	return result;
}

static const void* const AmyCheckJumpPtr = (void*)0x487640;
static inline signed int AmyCheckJump(playerwk* a1, taskwk* a2, motionwk2* a3)
{
	int result;
	__asm
	{
		mov ebx, [a3]
		mov ecx, [a2]
		mov eax, [a1]
		call AmyCheckJumpPtr
		mov result, eax
	}
	return result;
}

static const void* const BigCheckJumpPtr = (void*)0x48D2A0;
static inline signed int BigCheckJump(playerwk* a1, taskwk* a2)
{
	int result;
	__asm
	{
		mov ecx, [a2]
		mov eax, [a1]
		call BigCheckJumpPtr
		mov result, eax
	}
	return result;
}

static const void* const BigCheckInputPtr = (void*)0x48D400;
static inline signed int BigCheckInput(playerwk* a1, taskwk* a2, motionwk2* a3)
{
	int result;
	__asm
	{
		push[a3]
		mov edi, [a2]
		mov eax, [a1]
		call BigCheckInputPtr
		mov result, eax
		add esp, 4
	}
	return result;
}
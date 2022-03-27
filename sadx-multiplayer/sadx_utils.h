#pragma once

#define TASKWK_PLAYERID(a) a->counter.b[0] // Get player id from player's taskwk
#define TASKWK_CHARID(a) a->counter.b[1] // Get character id from player's taskwk
#define UNIT_RAND ((double)rand() / (double)(RAND_MAX + 1)) // Outputs a number between 0.0 and 1.0

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

struct CUSTOM_OBJ
{
	NJS_OBJECT* obj;
	void(__cdecl* exad)(NJS_OBJECT*);
};

struct KnFragmNmbStr
{
	char nmb[4];
	unsigned __int16 stgnmb;
};

struct KnFragmSetStr
{
	int id;
	int boutflag;
	int psflag;
	int cpflag;
	NJS_POINT3 pos;
	NJS_POINT3 contpos;
};

struct sMRacePath
{
	int flag;
	NJS_POINT3 pos;
};

struct sSonicCtrl
{
	NJS_POINT3 now_path_pos;
	NJS_POINT3 tgt_path_pos;
	NJS_POINT3 vec_snc_tgt;
	float dist_snc_tgt;
	int path_flag;
	int path_flag_bak;
	int last_ang;
	float pl_last_spd;
	int jump_cnt;
};

struct sParabola
{
	NJS_POINT3 pos_start;
	NJS_POINT3 pos_end;
	float gravity;
	int time;
	NJS_POINT3 speed;
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

struct _SC_NUMBERS
{
	unsigned __int8 type;
	unsigned __int8 attr;
	__int16 rot;
	unsigned int max;
	unsigned int value;
	NJS_POINT3 pos;
	float scl;
	unsigned int color;
};

struct Big_ydata
{
	int attr;
	int angx;
	int angz;
	float ypos;
};

struct Big_ypos
{
	Big_ydata top;
	Big_ydata bottom;
	Big_ydata water;
};

FunctionPointer(void, njDrawTriangle3D, (NJS_POINT3COL* p, int n, unsigned int atr), 0x77EBA0);
FunctionPointer(void, njDrawQuadTextureEx, (NJS_QUAD_TEXTURE_EX* quad), 0x77DE10);
VoidFunc(TempEraseSound, 0x424830);
VoidFunc(FreeQueueSound, 0x424460);
ObjectFunc(sub_425B30, 0x425B30);
ObjectFunc(sub_425BB0, 0x425BB0);
DataPointer(NJS_ARGB, cur_argb, 0x3AB9864);
DataPointer(int, Cart_demo_flag, 0x3C4ACA8); // Status of 
DataArray(NJS_TEXANIM, cartsprite_score, 0x3C5D558, 13); // simply "score" in symbols
FunctionPointer(BOOL, GetPlayerPosition, (unsigned __int8 pno, char frame, NJS_VECTOR* pos, Rotation3* ang), 0x4419C0);
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
FunctionPointer(void, DrawCustomObject, (NJS_OBJECT* top_object, CUSTOM_OBJ* custom), 0x4BA5D0);
VoidFunc(setRainEffect, 0x546320);
VoidFunc(C4SuimenYurashiSet, 0x5535D0);
VoidFunc(C4LeafSetOld, 0x554480);
VoidFunc(InitIndirectEffect3D_Bossegm1, 0x5723E0);
DataArray(_OBJ_LANDENTRY, ri_landentry_buf, 0x3B2E518, 128);
DataPointer(int, ri_landentry_nmb, 0x3B36D3C);
VoidFunc(dsEditLightInit, 0x40AEE0);
FunctionPointer(void, DrawLineV, (NJS_VECTOR*, NJS_VECTOR*), 0x412990);
FunctionPointer(void, Set3DPositionPCM, (int, float, float, float), 0x4102C0);
DataPointer(taskwk*, gpCharTwp, 0x3ABDF60);
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
FunctionPointer(int, GetPlayerNumber, (), 0x4144E0);
FunctionPointer(task*, ADXTaskInit, (), 0x4258B0);
FunctionPointer(void, BGM_Play, (int no), 0x425690);
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
FunctionPointer(void, late_DrawSprite2D, (NJS_SPRITE* sp, Int n, Float pri, NJD_SPRITE atr, int flgs), 0x404660);
TaskFunc(ObjShelterTunnelscroll, 0x5AC4A0);
TaskFunc(ObjShelterNo2cargo, 0x5ABED0);
FunctionPointer(void, SetSwitchOnOff, (unsigned int ID, int OnOff), 0x4CB4C0);
VoidFunc(CreateTunnelcol, 0x5AC2C0);
TaskFunc(EffectSpark, 0x4CE830);
TaskFunc(PathKassha, 0x603640);
TaskFunc(KasshaDisplayer, 0x603590);
FunctionPointer(void, SetVelocityAndRotationAndNoconTimeWithSpinDashP, (unsigned __int8 pno, NJS_POINT3* v, Angle3* ang, __int16 tm), 0x441540);
DataPointer(int, ke_ongame_flg, 0x3C52AD8);
DataPointer(int, found_feme_nmb, 0x3C52C04);
FunctionPointer(BOOL, isMissionClearDisp, (), 0x414FE0);
DataArray(KnFragmSetStr, fragmset_tbl, 0x3C52B20, 3);
DataArray(KnFragmNmbStr, fragmnmb_tbl, 0x7E0CD8, 6);
TaskFunc(KnucklesLaterSE, 0x474F50);
TaskFunc(FragmEmeraldDigDisplay, 0x4A31D0);
FunctionPointer(int, Knuckles_Status, (int plnmb), 0x475600);
FunctionPointer(void, CreateKiranR, (NJS_POINT3* pos, NJS_POINT3* velo, float scl, Angle ang), 0x4BAD80);
FunctionPointer(BOOL, EV_CheckCansel, (), 0x42FB00);
TaskFunc(Knuckles_KakeraGame, 0x476440);
DataPointer(int, ulGlobalMode, 0x3ABDC7C);
TaskFunc(SonicTheHedgehog, 0x49A9B0);
FunctionPointer(int, GetMRaceLevel, (), 0x47C200);
DataPointer(int, MRaceLevel, 0x3C53AB8);
DataPointer(int, MRaceStageNumber, 0x3C539EC);
TaskFunc(InitMoble2PControl, 0x47D8C0);
TaskFunc(InitSonic2PControl, 0x47D820);
TaskFunc(late_DispMilesMeter2P, 0x47C260);
DataPointer(sSonicCtrl, SonicCtrlBuff, 0x3C539F8);
DataPointer(sMRacePath*, PathTbl_Sonic, 0x03C539F4);
DataPointer(sMRacePath*, PathTbl_Miles, 0x3C53A64);
DataPointer(sParabola, SonicPaboBuff, 0x3C53A68);
DataPointer(NJS_POINT3, VecTemp0, 0x3B0F140);
DataPointer(NJS_POINT3, VecTemp1, 0x3B0F12C);
FunctionPointer(void, MakeParabolaInitSpeed, (sParabola* para), 0x4BD2D0);
FunctionPointer(int, ChkParabolaEnd, (sParabola* para), 0x4BD380);
FunctionPointer(void, CharColliOn, (taskwk* twp), 0x4BD180);
FunctionPointer(void, CharColliOff, (taskwk* twp), 0x4BD1B0);
FunctionPointer(void, SetAccelerationP, (unsigned __int8 pno, float x, float y, float z), 0x441750);
DataPointer(int, slJudge, 0x3B0EF44);
DataArray(sMRacePath*, PPT_MRaceEachStage, 0x91C0B8, 10);
DataArray(int*, MPT_MRaceEachStage, 0x91C0E0, 10);
DataPointer(taskwk*, rd_mountain_twp, 0x03C80F84);
FunctionPointer(void, SetParabolicMotionP, (int playerNum, float a2, NJS_VECTOR* a3), 0x446D90);
FunctionPointer(void, DrawSNumbers, (_SC_NUMBERS* pscn), 0x427BB0);
TaskFunc(exitFishingLure, 0x46C8D0);
TaskFunc(dispFishingLure, 0x470580);
FunctionPointer(int, BGM_Replay, (), 0x4256E0);
FunctionPointer(BOOL, BigSetPosition, (NJS_POINT3* p, NJS_POINT3* v, Angle3* a, float r), 0x46F130);
TaskFunc(BigDisplayHit, 0x46EBC0);
TaskFunc(BigDisplayStatus, 0x470090);
TaskFunc(dispBigKaeru, 0x7A6BB0);
TaskFunc(BigDisplayFishWeight, 0x4701A0);
DataPointer(uint32_t, MaskBlock, 0x3B36D48);
DataPointer(task*, pRd_Master, 0x3B0EFD8);

DataPointer(int, Sakana_Num, 0x3C524E8);
DataPointer(__int16, Big_Fish_Flag, 0x3C524EC);
DataPointer(int, Big_Sakana_Weight_Limit, 0x3C524F0);
DataPointer(float, reel_tension_add, 0x3C524F4);
DataPointer(task*, Big_Fish_Ptr, 0x3C524F8);
DataPointer(int, Big_Fishing_Timer, 0x3C524FC);
DataPointer(task*, Big_Lure_Ptr, 0x3C52500);
DataPointer(NJS_POINT3, big_item_pos, 0x3C52504);
DataPointer(int, Big_Sakana_Weight, 0x3C52510);
DataPointer(int, Big_Sakana_Kind_High, 0x3C52514);
DataPointer(int, Big_Stg12_Flag, 0x3C52518);
DataPointer(__int16, Big_Sakana_Weight_High, 0x3C5251C);
DataPointer(float, water_level, 0x3C52520);
DataPointer(float, reel_length, 0x3C52524);
DataPointer(float, reel_length_d, 0x3C52528);
DataPointer(float, reel_tension, 0x3C5252C);
DataPointer(float, reel_tension_aim, 0x3C52530);
DataPointer(Angle, reel_angle, 0x3C52534);
DataPointer(int, caution_timer, 0x3C52538);
DataArray(NJS_OBJECT*, lure_kind, 0x3C5253C, 7);
DataPointer(SaveFileData, sd, 0x3C52558);
DataPointer(float, distance, 0x3C52AC8);
DataPointer(float, distancep, 0x3C52ACC);
FunctionPointer(float, BigChkHeavyWeight, (), 0x46F7D0);
FunctionPointer(BOOL, GetMiClearStatus, (), 0x590650);
FunctionPointer(int, getLureKind, (), 0x46C870);
FunctionPointer(void, String_IniEasy, (String* ___this, const NJS_POINT3* v0, const NJS_POINT3* vN), 0x4BF860);
FunctionPointer(void, String_Exe, (String* ___this, const NJS_POINT3* v0, const NJS_POINT3* vN, int mode), 0x4BFCA0);
FunctionPointer(void, String_Dsp, (String* ___this), 0x4BFD40);

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

static const void* const HeliPathPtr = (void*)0x6134A0;
static inline void HeliPath(taskwk* twp, pathtag* tag, pathinfo* info)
{
	__asm
	{
		push[info]
		mov edx, [tag]
		mov esi, [twp]
		call HeliPathPtr
		add esp, 4
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


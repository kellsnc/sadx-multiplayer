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

struct erctrlstr
{
	int command;
	NJS_POINT3 pos0;
	NJS_POINT3 pos1;
};

struct KeyInfo
{
	int LastKey;
	int Buff[4];
	int Point;
	int RightCount;
	int LeftCount;
};

struct amyhndlstr
{
	int hndlmode;
	int touchflag;
	int turnang;
	int hndlangy;
	NJS_POINT3 hndlpos;
};

struct ITEM_MANAGER_DATA
{
	int item_list;
	float item_pos;
	float scale;
	int random_ring;
};

struct ITEM_MANAGER
{
	int mode;
	int current_list;
	unsigned int counter;
	ITEM_MANAGER_DATA itemdata[20];
};

struct OBJECT_ITEMBOX_AIR_DATA
{
	int flag;
	int item;
	NJS_POINT3 position;
	int panel_ang;
	float scale;
	float timer;
};

struct VolumeInfo
{
	__int16 nearVolOfs;
	__int16 farVolOfs;
	float nearDist;
	float farDist;
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
DataPointer(int, ulGlobalMode, 0x3ABDC7C);
TaskFunc(SonicTheHedgehog, 0x49A9B0);
FunctionPointer(int, GetMRaceLevel, (), 0x47C200);
DataPointer(int, MRaceLevel, 0x3C53AB8);
DataPointer(int, MRaceStageNumber, 0x3C539EC);
TaskFunc(InitMoble2PControl, 0x47D8C0);
TaskFunc(InitSonic2PControl, 0x47D820);
TaskFunc(late_DispMilesMeter2P, 0x47C260);
DataPointer(sParabola, SonicPaboBuff, 0x3C53A68);
DataPointer(NJS_POINT3, VecTemp0, 0x3B0F140);
DataPointer(NJS_POINT3, VecTemp1, 0x3B0F12C);
FunctionPointer(void, MakeParabolaInitSpeed, (sParabola* para), 0x4BD2D0);
FunctionPointer(int, ChkParabolaEnd, (sParabola* para), 0x4BD380);
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
DataArray(KeyInfo, KeyBuff, 0x3C72C38, 8);
DataPointer(SaveFileData, sd, 0x3C52558);
DataPointer(float, distance, 0x3C52AC8);
DataPointer(float, distancep, 0x3C52ACC);
FunctionPointer(float, BigChkHeavyWeight, (), 0x46F7D0);
FunctionPointer(int, getLureKind, (), 0x46C870);
FunctionPointer(void, String_IniEasy, (String* ___this, const NJS_POINT3* v0, const NJS_POINT3* vN), 0x4BF860);
FunctionPointer(void, String_Exe, (String* ___this, const NJS_POINT3* v0, const NJS_POINT3* vN, int mode), 0x4BFCA0);
FunctionPointer(void, String_Dsp, (String* ___this), 0x4BFD40);
DataPointer(float, No2CargoSpd, 0x3C72E5C);
DataPointer(int, item_kind, 0x3C5C888);
DataPointer(OBJECT_ITEMBOX_AIR_DATA*, itembox_air_data, 0x3C5A9D4);
FunctionPointer(void, CreateSnow, (NJS_POINT3* pos, NJS_POINT3* velo, float scl), 0x4B9B10);
FunctionPointer(void, DrawSquareC, (unsigned int col, float x, float y, float z, float magx, float magy), 0x431CB0);
FunctionPointer(void, DrawDlgCsrSqr, (unsigned __int8 col_alpha, float x, float y, float z, float magx, float magy), 0x431BB0);
FunctionPointer(void, CreateWater, (NJS_POINT3* pos, NJS_POINT3* velo, float scl), 0x4B9540);
FunctionPointer(void, MirenSoundPlayOneShotSE, (int se, const NJS_POINT3* pos_p, const VolumeInfo* info_p), 0x79E400);
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
FunctionPointer(void, MSG_Open, (MSGC* msgc, int x, int y, int width, int height, unsigned int globalindex), 0x40E430);
FunctionPointer(void, MSG_Disp, (MSGC* msgc), 0x40D490);
FunctionPointer(void, MSG_Puts_, (MSGC* msgc, const char* str), 0x40E570);
FunctionPointer(void, NH_MSG_Close, (MSGC* msgc), 0x40E2C0);
DataPointer(int, bWake, 0x912DF0); // Timer toggle

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

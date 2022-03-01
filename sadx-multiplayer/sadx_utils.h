#pragma once

#define TASKWK_PLAYERID(a) a->counter.b[0]
#define TASKWK_CHARID(a) a->counter.b[1]
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

struct CUSTOM_OBJ
{
	NJS_OBJECT* obj;
	void(__cdecl* exad)(NJS_OBJECT*);
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


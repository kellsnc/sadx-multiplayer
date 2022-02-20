#pragma once

#define TASKWK_PLAYERID(a) a->counter.b[0]
#define TASKWK_CHARID(a) a->counter.b[1]
static constexpr int ADVA_MODE_MULTI = 0x8;

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

struct LATE_LNK
{
	LATE_LNK* lnk;
	LATE_LNK* last;
};

struct LATE_RQ
{
	LATE_RQ* lnk;
	float zkey;
	unsigned __int8 typ;
	unsigned __int8 alpMd;
	unsigned __int16 no;
	NJS_TEXLIST* texLst;
	NJS_ARGB argb;
	unsigned int ctrl3dFlg;
	unsigned int atrAnd;
	unsigned int atrOr;
	unsigned int etc;
};

struct LATE_DRAWTEX_RQ
{
	LATE_RQ cmn;
	NJS_MATRIX mtx;
	NJS_TEXTURE_VTX* vtx;
	int num;
	int ind;
	int tf;
};

struct LATE_ACT_RQ
{
	LATE_RQ cmn;
	NJS_MATRIX mtx;
	NJS_ACTION* act;
	float frm;
};

struct LATE_ACTLNK_RQ
{
	LATE_RQ cmn;
	NJS_MATRIX mtx;
	float frm;
	NJS_ACTION_LINK actlnk;
	NJS_MOTION_LINK motlnk;
};

struct LATE_PO2D_RQ
{
	LATE_RQ cmn;
	NJS_POINT2COL p2c;
	NJS_MATRIX mtx;
	unsigned int atr;
	int texNum;
	float pri;
};

struct LATE_S3D_RQ
{
	LATE_RQ cmn;
	NJS_SPRITE spr;
	unsigned int atr;
	unsigned int dmy;
	NJS_MATRIX mtx;
};

struct LATE_PO3D_RQ
{
	LATE_RQ cmn;
	NJS_POINT3COL p3c;
	NJS_MATRIX mtx;
	unsigned int atr;
	int texNum;
};

struct LATE_SHPMOT_RQ
{
	LATE_RQ cmn;
	NJS_MATRIX mtx;
	float frm;
	NJS_OBJECT* obj;
	NJS_MOTION* mot;
	NJS_MOTION* shp;
	int flgs;
	float clpScl;
	void* drwMdl;
};

struct LATE_OBJ_RQ
{
	LATE_RQ cmn;
	NJS_MATRIX mtx;
};

struct LATE_BOX2D_RQ
{
	LATE_RQ cmn;
	float x;
	float y;
	float x2;
	float y2;
	float pri;
	unsigned int argb;
};

struct LATE_S2D_RQ
{
	LATE_RQ cmn;
	NJS_SPRITE spr;
	unsigned int atr;
	float pri;
	NJS_MATRIX mtx;
};

struct LATE_FUNC_RQ
{
	LATE_RQ cmn;
	NJS_MATRIX mtx;
	void(__cdecl* fnc)(void*);
	void* arg;
};

union LATE_RQ_T
{
	LATE_S3D_RQ s3d;
	LATE_S2D_RQ s2d;
	LATE_OBJ_RQ obj;
	LATE_ACT_RQ act;
	LATE_PO2D_RQ po2d;
	LATE_PO3D_RQ po3d;
	LATE_SHPMOT_RQ shpmot;
	LATE_BOX2D_RQ box2d;
	LATE_DRAWTEX_RQ tex;
	LATE_FUNC_RQ func;
	LATE_RQ rq;
};

struct ___stcFogEmu
{
	unsigned __int8 u8Emulation;
	unsigned __int8 u8FogSplitCnt;
};

struct OBJECT_SAVEPOINT_DATA
{
	int flag;
	Angle3 ang;
	Angle3 ang_spd;
	task* tp[2];
	float materiral[3];
	int write_timer;
	__int16 kiseki_timer;
};

struct FCWRK
{
	float dist1;
	float dist2;
	float dist0;
	float dist;
	Angle3 _ang;
	NJS_POINT3 cammovepos;
	NJS_POINT3 campos;
	int counter;
	int timer;
	Angle3 pang;
	NJS_POINT3 camspd;
	NJS_POINT3 pos;
	NJS_POINT3 _vec;
};

struct ITEM_INFOMATION
{
	unsigned int texture_id;
	void(__cdecl* effect_func)(taskwk*);
};

struct ENEMY_CART_DATA
{
	int flag;
	int ring_timer;
	float player_colli_r;
	int fan_angle;
	__int16 cart_color;
	__int16 invincible_timer;
	__int16 motion_timer;
	__int16 ignor_collision;
	__int16 drift_mode;
	__int16 drift_timer;
	int drift_angle;
	char vitality;
	Angle3 unstable;
	int loop_pos;
	float restless;
	float restless_timer;
	int smoke_timer;
	int drift_effect_timer;
	float shadow_pos;
	NJS_POINT3 vector;
	Angle3 shadow_ang;
	float max_spd;
	NJS_POINT3 add_key;
	NJS_POINT3 add_nature;
	float bonus_add;
	int rest_timer;
	unsigned __int8 next_point;
	unsigned __int8 load_line;
	unsigned __int8 load_indp[2];
	NJS_POINT3 last_pos;
	char cart_type;
	char start_wait;
	__int16 last_player_flag;
	char explose_flag[10];
	NJS_POINT3 explose_point[10];
	NJS_POINT3 explose_spd[10];
	Angle3 explose_angle[10];
	Angle3 explose_rotate[10];
	float tmp_posi[3];
	int tmp_angle[3];
	unsigned int hamari_cnt;
};

struct CART_PLAYER_PARAMETER
{
	float max_spd;
	float min_spd;
	float max_back_spd;
	float slide_spd;
	float gravitation;
	float gravitation_max;
	float hover_point;
	float grip;
	float spd_accell;
	float spd_break;
	float spd_masatu;
	int max_vitality;
	int smoke_vitality;
	float cart_handle;
	float drift_rate;
};

struct CUSTOM_OBJ
{
	NJS_OBJECT* obj;
	void(__cdecl* exad)(NJS_OBJECT*);
};

struct CART_OTHER_PARAM
{
	float chase_distance;
	int rest_time;
	float cart_enemy_accell;
	float cart_enemy_break;
	float cart_enemy_search_addspd;
	float explose_spd;
	int explose_ang;
	int dead_wait_time;
	int ring_sub_timer;
};

struct CART_LOAD_DATA
{
	NJS_POINT3* load_data;
	int point_num;
};

struct _OBJ_CAMERAENTRY
{
	char scMode;
	char scPriority;
	unsigned __int8 ucAdjType;
	char scColType;
	unsigned __int16 xColAng;
	unsigned __int16 yColAng;
	float xColPos;
	float yColPos;
	float zColPos;
	float xColScl;
	float yColScl;
	float zColScl;
	unsigned __int16 xCamAng;
	unsigned __int16 yCamAng;
	float xDirPos;
	float yDirPos;
	float zDirPos;
	float xCamPos;
	float yCamPos;
	float zCamPos;
	float fDistance;
};

struct TaskInfo
{
	unsigned int size;
	unsigned int initMode;
	void(__cdecl* init_p)(task*, void*);
	TaskFuncPtr exec_p;
	TaskFuncPtr disp_p;
	TaskFuncPtr dest_p;
};

struct CartDispTime
{
	char sign;
	char min;
	char sec;
	char sec100;
};

struct PLAYER_CONTINUE_DATA
{
	unsigned __int16 stage_number;
	unsigned __int8 minutes;
	unsigned __int8 second;
	unsigned __int8 frame;
	int continue_flag;
	NJS_POINT3 pos;
	Angle3 ang;
};

VoidFunc(DisplayTask, 0x40B540);
TaskFunc(Camera, 0x438090);
DataPointer(taskwk*, camera_twp, 0x3B2CBB0);
FunctionPointer(void, ds_DrawBoxFill2D, (float x, float y, float x2, float y2, float pri, int argb), 0x4071C0);
FunctionPointer(void, late_DrawBoxFill2D, (float x, float y, float x2, float y2, float pri, int argb, int flgs), 0x4073B0);
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
FunctionPointer(void, ___dsDrawObject, (NJS_OBJECT* obj), 0x408530);
FunctionPointer(void, njSetZCompare, (Uint8 index), 0x77ED00);
FunctionPointer(void, njSetZUpdateMode, (DWORD enable), 0x77ED20);
DataPointer(Angle, ds_perspective_value, 0x3AB98EC);
FunctionPointer(void, ___SAnjDrawPolygon2D, (NJS_POINT2COL* p, int n, float pri, NJD_DRAW attr), 0x4010D0);
FunctionPointer(void, njDrawPolygon3D, (NJS_POINT3COL* p, int n, NJD_DRAW  attr), 0x77EAD0);
DataPointer(NJS_TEXLIST*, njds_texList, 0x3ABD950);
DataPointer(int, lig_curGjPaletteNo___, 0x3B12208);
FunctionPointer(void, lig_setGjPaletteNo___, (int no), 0x412160);
FunctionPointer(void, lig_resetGjPaletteNo___, (signed int no), 0x412400);
DataPointer(NJS_ARGB, cur_argb, 0x3AB9864);
DataPointer(bool, gu8FogEnbale, 0x3ABDCFE);
DataPointer(int, late_execMode, 0x3AB98AC);
VoidFunc(npSetZCompare, 0x401420);
DataPointer(___stcFogEmu, gFogEmu, 0x909EB4);
FunctionPointer(BOOL, GetPlayerPosition, (unsigned __int8 pno, char frame, NJS_VECTOR* pos, Rotation3* ang), 0x4419C0);
DataPointer(BOOL, cameraready, 0x3B2CBB8);
TaskFunc(RingDoneDisplayer, 0x44FC80);
FunctionPointer(BOOL, CheckItemExplosion, (NJS_POINT3* pos), 0x4C0550);
DataPointer(int, slEnemyScore, 0x3B0F104);
VoidFunc(InitActionScore, 0x427EF0);
FunctionPointer(void, AddNumRing, (Sint16 amount), 0x425BE0);
FunctionPointer(Sint16, GetNumRing, (), 0x425CC0);
FunctionPointer(int, ResetNumRing, (), 0x425AB0);
FunctionPointer(int, ResetNumPlayer, (), 0x425AF0);
FunctionPointer(int, GetNumPlayer, (), 0x425FE0);
FunctionPointer(void, AddNumPlayer, (__int16 lives), 0x425B60);
DataPointer(char, scNumPlayer, 0x3B0EF34);
DataPointer(Sint16, ssNumRing, 0x3B0F0E4);
DataArray(OBJ_CONDITION, objStatusEntry, 0x3C4E460, 1024);
DataPointer(__int16, numStatusEntry, 0x3C4E454);
DataPointer(_OBJ_ITEMTABLE*, pObjItemTable, 0x3C4E448);
DataPointer(BOOL, boolOneShot, 0x3C52464);
DataPointer(OBJECT_SAVEPOINT_DATA*, savepoint_data, 0x3B42F7C);
FunctionPointer(void, updateContinueData, (NJS_POINT3* pos, Angle3* ang), 0x44EE70);
FunctionPointer(bool, CheckEditMode, (), 0x4258F0);
FunctionPointer(void, CameraFilter, (task* tp), 0x436CD0);
TaskFunc(CameraDisplay, 0x4370F0);
TaskFunc(CameraPause, 0x4373D0);
DataPointer(FCWRK, fcwrk, 0x3B2C958); // free cam worker
DataPointer(Uint32, free_camera_mode, 0x3B2CBA8);
FunctionPointer(void, DrawCharacterShadow, (taskwk* twp, shadowwk* swp), 0x49F0B0);
TaskFunc(execTPCoaster, 0x61D6E0);
FunctionPointer(void, GetOutOfCartP, (int pno, float x, float y, float z), 0x441820);
VoidFunc(MakeLandCollLandEntryALL, 0x43B580);
DataPointer(float, mleriRangeRad, 0x915458); // minimum radius for ground collisions lookup
DataPointer(Uint16, numLandCollList, 0x3B32724);
DataArray(_OBJ_LANDCOLL, ri_landcoll, 0x3B32728, 128);
DataArray(_OBJ_LANDCOLL, LandCollList, 0x3B2F720, 1024);
DataPointer(int, ri_landcoll_nmb, 0x3B36D38);
FunctionPointer(void, CalcAdvanceAsPossible, (NJS_POINT3* src, NJS_POINT3* dst, float dist, NJS_POINT3* ans), 0x4BA860);
DataArray(ITEM_INFOMATION, item_info, 0x9BF190, 9);
DataPointer(int, RandomRingNum, 0x3C5AB30);
FunctionPointer(void, GetInvincibleBodyP, (int character), 0x441F10);
FunctionPointer(void, GetThunderBarrierP, (char character), 0x441E30);
FunctionPointer(void, GetBarrierP, (char character), 0x441EA0);
DataPointer(NJS_POINT3, explosion_pos, 0x3C5AB24);
DataPointer(Float, explosion_r, 0x3C5A9D0);
FunctionPointer(void, ef_speed, (taskwk* twp), 0x4D6BF0);
FunctionPointer(void, ef_muteki, (taskwk* twp), 0x4D6D80);
FunctionPointer(void, ef_5ring, (taskwk* twp), 0x4D6C50);
FunctionPointer(void, ef_10ring, (taskwk* twp), 0x4D6C90);
FunctionPointer(void, ef_random_ring, (taskwk* twp), 0x4D6CD0);
FunctionPointer(void, ef_baria, (taskwk* twp), 0x4D6DC0);
FunctionPointer(void, ef_1up, (taskwk* twp), 0x4D6D40);
FunctionPointer(void, ef_explosion, (taskwk* twp), 0x4D6E00);
FunctionPointer(void, ef_th_baria, (taskwk* twp), 0x4D6E40);
TaskFunc(ThunderB, 0x4BA100);
TaskFunc(TBarrierDisp, 0x4B9D90);
FunctionPointer(float, DrawShadow_, (Angle3* ang, NJS_POINT3* pos, float scl), 0x49EF30);
FunctionPointer(BOOL, CheckRange, (task* tp), 0x46C330);
FunctionPointer(BOOL, CheckRangeWithR, (task* tp, Float fRange), 0x46BFA0);
FunctionPointer(int, GetGlobalTime, (), 0x4261B0);
FunctionPointer(bool, dsCheckViewV, (NJS_POINT3* ft, float radius), 0x403330);
FunctionPointer(void, DrawCustomObject, (NJS_OBJECT* top_object, CUSTOM_OBJ* custom), 0x4BA5D0);
FunctionPointer(void, SetVelocityP, (uint8_t pno, float x, float y, float z), 0x441280);
FunctionPointer(void, VibShot, (int pno, int Time), 0x4BCBC0);
FunctionPointer(void, VibConvergence, (int pno, int Power, int Freq, int Time), 0x4BCC10);
FastcallFunctionPointer(bool, njCollisionCheckSS, (float* p1, float* p2), 0x789360);
FunctionPointer(void, KillHimP, (int pno), 0x440CD0); // kill player
FunctionPointer(void, DamegeRingScatter, (char pno), 0x4506F0);
DataPointer(_OBJ_CAMERAENTRY*, pObjCameraEntry, 0x3B2CAA4);
VoidFunc(SetTableBG_Chaos0, 0x545CE0);
VoidFunc(SetTableBG_Chaos2, 0x54A540);
VoidFunc(SetTableBG_Chaos4, 0x550840);
VoidFunc(SetTableBG_Chaos6, 0x5577E0);
VoidFunc(SetTableBG_E101, 0x566AF0);
VoidFunc(setRainEffect, 0x546320);
VoidFunc(C4SuimenYurashiSet, 0x5535D0);
VoidFunc(C4LeafSetOld, 0x554480);
VoidFunc(InitIndirectEffect3D_Bossegm1, 0x5723E0);
TaskFunc(BossChaos0, 0x548640);
TaskFunc(Chaos2Column, 0x548640);
TaskFunc(EggCarrierCloud_c6, 0x557690);
DataPointer(taskwk*, chaostwp, 0x3C5A7D8);
FunctionPointer(void, MirenInitTask, (task* task_p, const TaskInfo* info_p, void* param_p), 0x796B30);
FunctionPointer(void, MirenSetTask, (int level, const TaskInfo* info_p, void* param_p), 0x796B90);
DataPointer(task*, RaceManageTask_p, 0x3C5D554);
DataPointer(int, Cart_demo_flag, 0x3C4ACA8);
FunctionPointer(void, CartInitLetter, (), 0x4DC740);
DataArray(NJS_TEXANIM, cartsprite_score, 0x3C5D558, 13); // simply "score" in symbols
FunctionPointer(CartDispTime, CartStateCentiSecToDispTime, (int sec100), 0x4DBBA0);
VoidFunc(ResetMleriRangeRad, 0x43B6F0);
DataPointer(BOOL, boolLandCollision, 0x915460);
DataArray(_OBJ_LANDENTRY*, pDisplayEntry, 0x3B2D518, 1024);
DataArray(_OBJ_LANDENTRY, ri_landentry_buf, 0x3B2E518, 128);
DataPointer(int, ri_landentry_nmb, 0x3B36D3C);
DataPointer(_OBJ_LANDTABLE*, pObjLandTable, 0x3B2F718);
VoidFunc(dsEditLightInit, 0x40AEE0);
FunctionPointer(void, DrawLineV, (NJS_VECTOR*, NJS_VECTOR*), 0x412990);
DataArray(_SEcallbuf, sebuf, 0x3B292F8, 36); // 20 in xbox version
DataArray(taskwk*, gpDolbyTask, 0x3B29B90, 36); // 20 in xbox version
FunctionPointer(BOOL, Get3Dmode, (), 0x40FF40);
FunctionPointer(int, dsPlay_oneshot_v, (int tone, int id, int pri, int volofs, float x, float y, float z), 0x424FC0);
FunctionPointer(void, Set3DPositionPCM, (int, float, float, float), 0x4102C0);
DataPointer(taskwk*, gpCharTwp, 0x3ABDF60);
TaskFunc(relbox_switch_draw, 0x46A8D0);
FunctionPointer(void, PClearSpeed, (motionwk2* mwp, playerwk* pwp), 0x43C550);
DataPointer(int, MRaceResult, 0x3C53A94);
DataPointer(NJS_SPRITE, Spr_MRaceDisp, 0x3C53A98);
VoidFunc(SetFinishAction, 0x415540);
DataPointer(PLAYER_CONTINUE_DATA, continue_data, 0x3B42F80);
FunctionPointer(task*, SetDialogTask, (), 0x432C60);
DataPointer(pathtag, pathtag_hw1_heli0807, 0x26A72D4);
FunctionPointer(void, SetVelocityAndRotationAndNoconTimeP, (int pno, NJS_POINT3* v, Angle3* ang, int tm), 0x441490);
DataPointer(NJS_OBJECT, object_turnasi_oya_koa, 0x267D3B4);
DataPointer(NJS_OBJECT, object_turnasi_oya_kob, 0x267C7AC);

DataPointer(ENEMY_CART_DATA*, cart_data, 0x3D08E0C);
DataArray(__int16, cartColor, 0x88C004, 7);
DataArray(CART_PLAYER_PARAMETER, CartParameter, 0x38C5DA8, 8);
DataArray(CCL_INFO, cci_cart, 0x38A6BF0, 8);
TaskFunc(cartDisplay, 0x796CE0);
DataArray(CUSTOM_OBJ, fan_model_big, 0x88C03C, 2);
DataPointer(NJS_OBJECT, object_b_cart_cart_cart, 0x38A9130);
DataArray(CUSTOM_OBJ, fan_model_e102, 0x88C014, 2);
DataPointer(NJS_OBJECT, object_e_cart_cart_cart, 0x38AB250);
DataArray(CUSTOM_OBJ, fan_model, 0x88C02C, 2);
DataPointer(NJS_OBJECT, object_sarucart_sarucart_sarucart, 0x38BAAA4);
DataPointer(NJS_OBJECT, object_sarucart_saru_body_saru_body, 0x38B8780);
DataArray(NJS_MODEL_SADX*, cart_model, 0x38F4FC8, 32);
FunctionPointer(void, cartTopographicalCollision, (task* tp, taskwk* twp), 0x799380);
DataPointer(CART_OTHER_PARAM, CartOtherParam, 0x38C5F88);
DataPointer(BOOL, CartGoalFlag, 0x3D08E00);
DataArray(CART_LOAD_DATA, cart_load, 0x38C7FF0, 18);
TaskFunc(EnemyCart, 0x79A9E0);

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

static const void* const setCartDirectionPtr = (void*)0x798050;
static inline void setCartDirection(taskwk* twp, NJS_POINT3* vec)
{
	__asm
	{
		push[vec]
		mov esi, [twp]
		call setCartDirectionPtr
		add esp, 4
	}
}

static const void* const cartExplosionPtr = (void*)0x797300;
static inline void cartExplosion(taskwk* twp)
{
	__asm
	{
		mov edi, [twp]
		call cartExplosionPtr
	}
}

static const void* const cartCheckPassPtr = (void*)0x7980C0;
static inline void cartCheckPass(taskwk* twp)
{
	__asm
	{
		mov ecx, [twp]
		call cartCheckPassPtr
	}
}

static const void* const cartSpdControlSonicOnTheCartPtr = (void*)0x798E40;
static inline void cartSpdControlSonicOnTheCart(taskwk* twp)
{
	__asm
	{
		mov ebx, [twp]
		call cartSpdControlSonicOnTheCartPtr
	}
}

static const void* const setupCartStagePtr = (void*)0x7981F0;
static inline void setupCartStage(task* tp)
{
	__asm
	{
		mov eax, [tp]
		call setupCartStagePtr
	}
}

static const void* const cartSELoopPtr = (void*)0x798170;
static inline void cartSELoop(int se_no, task* tp)
{
	__asm
	{
		mov edi, [tp]
		mov ebx, [se_no]
		call cartSELoopPtr
	}
}

static const void* const cartThinkPtr = (void*)0x79A8E0;
static inline void cartThink(taskwk* twp, task* tp)
{
	__asm
	{
		mov ecx, [tp]
		mov eax, [twp]
		call cartThinkPtr
	}
}

static const void* const cartSpdForceOfNaturePtr = (void*)0x799EB0;
static inline void cartSpdForceOfNature(taskwk* twp)
{
	__asm
	{
		mov edx, [twp]
		call cartSpdForceOfNaturePtr
	}
}

static const void* const cartShadowPosPtr = (void*)0x7977C0;
static inline void cartShadowPos(taskwk* twp)
{
	__asm
	{
		mov eax, [twp]
		call cartShadowPosPtr
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

static const void* const CameraSetViewPtr = (void*)0x435600;
static inline void CameraSetView(taskwk* twp)
{
	__asm
	{
		mov eax, [twp]
		call CameraSetViewPtr
	}
}

// Simply turn vector to direction
static const void* const calcModerateVectorPtr = (void*)0x465F00;
static inline void calcModerateVector(NJS_POINT3* vec, Angle3* ang)
{
	__asm
	{
		mov esi, [ang]
		mov edi, [vec]
		call calcModerateVectorPtr
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

static const void* const RdMountainInitPtr = (void*)0x601390;
static inline void RdMountainInit(task* tp)
{
	__asm
	{
		mov esi, [tp]
		call RdMountainInitPtr
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

static const void* const RdHighwayInitPtr = (void*)0x60FF80;
static inline void RdHighwayInit(task* tp)
{
	__asm
	{
		mov edi, [tp]
		call RdHighwayInitPtr
	}
}

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

enum DiaTypeEnum : __int32
{
	DIA_TYPE_FILESEL = 0x0,
	DIA_TYPE_FILEDEL = 0x1,
	DIA_TYPE_MAINMENU = 0x2,
	DIA_TYPE_MAINMENU_NPUTI = 0x3,
	DIA_TYPE_TITLEMENU = 0x4,
	DIA_TYPE_TRIALACTSEL = 0x5,
	DIA_TYPE_MINIGAMESEL = 0x6,
	DIA_TYPE_CHARSEL1 = 0x7,
	DIA_TYPE_CHARSEL2 = 0x8,
	DIA_TYPE_CHARSEL3 = 0x9,
	DIA_TYPE_CHARSEL4 = 0xA,
	DIA_TYPE_OPT_MONOSTE = 0xB,
	DIA_TYPE_OPT_MSG = 0xC,
	DIA_TYPE_OPT_VOICE = 0xD,
	DIA_TYPE_OPT_TEXT = 0xE,
	DIA_TYPE_TVSETTING1 = 0xF,
	DIA_TYPE_SNDTEST = 0x10,
	DIA_TYPE_VMSNOMEM = 0x11,
	DIA_TYPE_OPT_VIBSEL = 0x12,
	DIA_TYPE_OPT_SNDOUT = 0x13,
	DIA_TYPE_FILE_CRC_ERR = 0x14,
	DIA_TYPE_FORMAT = 0x15,
	DIA_TYPE_NEW_FILESEL = 0x16,
	DIA_TYPE_MAINMENU_SUB_UN = 0x17,
	DIA_TYPE_MAINMENU_SUB = 0x18,
	DIA_TYPE_MAINMENU_SUB_DL = 0x19,
	DIA_TYPE_OPTIONMENU_SUB = 0x1A,
	DIA_TYPE_HOW_TO_PLAY = 0x1B,
	DIA_TYPE_LB_MENU = 0x1C,
	DIA_TYPE_MAINMENU_NDOWNLOAD = 0x1D,
	DIA_TYPE_MAINMENU_EXIT = 0x1E,
	DIA_TYPE_MAX = 0x1F,
};

struct __declspec(align(4)) DDlgType
{
	float CntrX;
	float CntrY;
	float BaseZ;
	float SclX;
	float SclY;
	char Csr;
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
	void(__cdecl* EachDrawFnc)(DDlgType*);
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

DataArray(const AvaTexLdEnum*, AvaTexLdLists, 0x10D7CC4, 14);
DataPointer(AvaStgActT, AvaCmnPrm, 0x3C5FED0);
DataPointer(ADVERTISE_WORK, AdvertiseWork, 0x3B2A2FA);
DataArray(DialogPrmType, DialogPrm, 0x7EE328, 22); // Menu dialogs
DataPointer(const DialogPrmType, DialogAskQuit, 0x7DD48C);
TaskFunc(title_new_exec, 0x5101A0);
TaskFunc(trial_act_sel_exec, 0x50B220);
TaskFunc(trial_act_sel_disp, 0x50B410);

DataPointer(int, unk_3C49C23, 0x3C49C23);
DataPointer(task*, SeqTp, 0x3C5E8D0);
DataPointer(task*, TrialActStelTp, 0x3C5FEE0);
DataArray(PVMEntry*, MenuTexlists, 0x10D7CB0, 5);
DataArray(void*, CreateModeFncPtrs, 0x10D7B4C, 14);
DataArray(void*, FreeModeFncPtrs, 0x10D7B84, 14);
DataPointer(task*, TitleNewTp, 0x3C5FF00);
DataPointer(BOOL, Menu_CanSelect, 0x3C5E8E0);
FunctionPointer(void, AvaLoadTexForEachMode, (int mode) ,0x506010);
VoidFunc(AvaReleaseTexForEachMode, 0x506040); // also sets menu ready flag
DataPointer(BOOL, TldFlg, 0x3C5E8E0); // menu ready flag

FunctionPointer(BOOL, IsMiniGameMenuEnabled, (), 0x506460);
FunctionPointer(BOOL, GetEnableTrialActNumAll, (), 0x5061F0);
FunctionPointer(BOOL, GetEnableMiniGameNumAll, (), 0x506210);
FunctionPointer(void, AdvaOpenDialogCsrLet, (DiaTypeEnum dialog_type, char csr, char* csrp), 0x5057D0);

VoidFunc(PlayMenuBipSound, 0x6FC8A0);
VoidFunc(PlayMenuEnterSound, 0x505810);
VoidFunc(PlayMenuBackSound, 0x505830);
FunctionPointer(void, PlayMenuMusicID, (MusicIDs id), 0x505900);
FunctionPointer(void, PlayMenuMusic, (int id), 0x505990);
FunctionPointer(void, CmnAdvaModeProcedure, (AdvaModeEnum mode), 0x505B40);
FunctionPointer(void, CharSelAdvaModeProcedure, (AdvaModeEnum mode), 0x505E60);
FunctionPointer(int, GetFadeOutColFromT, (float t), 0x506E10);
FunctionPointer(int, GetFadeInColFromT, (float t), 0x506E40);
DataArray(int, GblMenuTbl, 0x7EF8E8, 6);
FunctionPointer(void, OpenDialog, (const DialogPrmType* dp), 0x432DB0);
FunctionPointer(void, OpenDialogCsrLet, (const DialogPrmType* dp, char csr, char* dis_csr_ptr), 0x432D20);
FunctionPointer(BOOL, AvaGetTrialEnable, (), 0x506780);
FunctionPointer(BOOL, AvaGetMissionEnable, (), 0x506410);
FunctionPointer(void, AdvaOpenDialogQuick, (DiaTypeEnum dialog_type, char csr, char* csrp), 0x5057F0);
FunctionPointer(char, GetDialogStat, (), 0x432550);
FunctionPointer(BOOL, CloseDialog, (), 0x432580);
VoidFunc(DialogJimakuInit, 0x40BC80);
FunctionPointer(void, DialogJimakuPut, (const char* str), 0x40BD30);
DataPointer(task*, DialogTp, 0x3B2C588);

VoidFunc(ghDefaultBlendingMode, 0x433170);
FunctionPointer(void, DrawSkyBg, (float z), 0x507BB0);
FunctionPointer(void, ghSetPvrTexBaseColor, (Uint32 color), 0x433010);
FunctionPointer(void, ghSetPvrTexMaterial, (Uint32 color), 0x432F40);
FunctionPointer(void, DrawTitleBack, (float x, float y, float z, float w, float h), 0x4334F0);
FunctionPointer(void, ghSetPvrTexVertexColor, (unsigned int c0, unsigned int c1, unsigned int c2, unsigned int c3), 0x432F90);
FunctionPointer(void, ghDrawPvrTexture, (int index, float x, float y, float z), 0x4338D0);
FunctionPointer(void, DrawShadowWindow, (float x, float y, float z, float w, float h), 0x4343E0);

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

DataPointer(MSGC, jimakumsgc, 0x3ABDC18);
DataPointer(void*, jimakubuf, 0x3ABDF84);
FunctionPointer(void, MSG_Cls, (MSGC* msgc), 0x40D850);
FunctionPointer(void, MSG_LoadTexture, (MSGC* msgc), 0x40D290);
FunctionPointer(void, MSG_LoadTexture2, (MSGC* msgc), 0x40D2A0);
FunctionPointer(void, MSG_Puts, (MSGC* msgc, const char* text), 0x40D290);
FunctionPointer(void, NH_MSG_Open, (MSGC* a1, __int16 x, __int16 y, int width, int height, int globalindex, void* buf), 0x40E430);
FunctionPointer(void, MSG_Close, (MSGC* msgc), 0x40D450);

struct __declspec(align(4)) CharMdlWk
{
	task* PlTskPtrs[7];
	task* CamTskPtr;
	float LocX;
	float LocY;
	float Scale;
	char MdlDspCnt;
	char MotCnts[7];
	unsigned __int8 SelFlg;
};

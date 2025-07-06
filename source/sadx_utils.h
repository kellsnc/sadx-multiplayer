#pragma once

#define TASKWK_PLAYERID(a) a->counter.b[0] // Get player id from player's taskwk
#define TASKWK_CHARID(a) a->counter.b[1] // Get character id from player's taskwk

#define GET_STAGE(num) ((num) >> 8)  /* Extract stage from GetStageNumber */
#define GET_ACT(num) ((num) & 0xFF)  /* Extract act from GetStageNumber */

#define SHORT_ANG(ang) ((ang) & 0xFFFF)
#define ROTATEX(m, ang) if (ang != 0) njRotateX(m, SHORT_ANG(ang));
#define ROTATEY(m, ang) if (ang != 0) njRotateY(m, SHORT_ANG(ang));
#define ROTATEZ(m, ang) if (ang != 0) njRotateZ(m, SHORT_ANG(ang));

#define IM_MOTIONWK 0x1
#define IM_TASKWK   0x2
#define IM_FORCEWK  0x4
#define IM_ANYWK    0x8

#define TWK_FLAG_CCL_DMG    0x4    /* Has been touched by a CCL collision inflicting damage */
#define TWK_FLAG_CCL_HIT    0x8    /* Has been touched by a CCL collision */
#define TWK_FLAG_CCL_BELOW  0x10   /* Currently below another collision */
#define TWK_FLAG_CCL_ABOVE  0x20   /* Currently above another collision */
#define TWK_FLAG_MOVE       0x40   /* Movable system */
#define TWK_FLAG_COL_ACTIVE 0x100  /* The geometry collision can run */
#define TWK_FLAG_HELD       0x1000 /* Is being held */
#define TWK_FLAG_OCC_M      0x2000 /* OCC: use "mesh" draw variant */
#define TWK_FLAG_OCC_WZ     0x4000 /* OCC: queue model with LATE_WZ flag */
#define TWK_FLAG_OCC_NWZ    0x8000 /* OCC: queue model without LATE_WZ flag */

#define CWK_FLAG_HIT 0x1

#define TWK_FLAG_MOVE_FLOOR 0x1

#define MOVE_FLAG_1          0x1    /* ? */
#define MOVE_FLAG_4          0x4    /* ? */
#define MOVE_FLAG_CHECK_STOP 0x8    /* Auto-assign stop flag if the chao is not moving */
#define MOVE_FLAG_STOP       0x10   /* Is not moving, disables collision check with world */
#define MOVE_FLAG_100        0x100  /* ? */
#define MOVE_FLAG_200        0x200  /* ? */
#define MOVE_FLAG_FLOOR      0x400  /* On ground */
#define MOVE_FLAG_WALL       0x800  /* Hit a wall */
#define MOVE_FLAG_1000       0x1000 /* ? */
#define MOVE_FLAG_WATER      0x2000 /* Colliding with water */
#define MOVE_FLAG_WALL_FRONT 0x4000 /* Front has hit a wall (colli info in FrontWall) */

static constexpr int ADVA_MODE_MULTI = 0x8;

enum {
	PLNO_SONIC,
	PLNO_EGGMAN,
	PLNO_TAILS,
	PLNO_KNUCKLES,
	PLNO_TIKAL,
	PLNO_AMY,
	PLNO_E102,
	PLNO_BIG,
	PLNO_METAL_SONIC,
	NB_PLNO
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

enum
{
	LATE_TYP_MDL = 0x1,
	LATE_TYP_S2D = 0x2,
	LATE_TYP_S3D = 0x3,
	LATE_TYP_LIN3D = 0x4,
	LATE_TYP_TRI3D = 0x5,
	LATE_TYP_POLY2D = 0x6,
	LATE_TYP_POLY3D = 0x7,
	LATE_TYP_ACT = 0x8,
	LATE_TYP_BOX2D = 0x9,
	LATE_TYP_OBJ = 0xA,
	LATE_TYP_ACTLNK = 0xB,
	LATE_TYP_FUNC = 0xC,
	LATE_TYP_DRAWTEX = 0xD,
	LATE_TYP_LIN2D = 0xE,
	LATE_TYP_SHPMOT = 0xF,
	LATE_TYP_MSK = 0xF,
	LATE_TYP_WZF = 0x10,
	LATE_TYP_FOG = 0x20,
};

enum AVOID_CAMERA_MODE
{
	AVOID_MODE_INIT,
	AVOID_MODE_CHASE,
	AVOID_MODE_KNUCKLES,
	AVOID_MODE_END,
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

struct beamhitstr
{
	float reach;
	NJS_POINT3 pos;
	NJS_POINT3 vec;
	float dist;
	NJS_POINT3 hitpos;
	xssunit hitinfo;
};

VoidFunc(FreeQueueSound, 0x424460);
ObjectFunc(sub_425B30, 0x425B30);
ObjectFunc(sub_425BB0, 0x425BB0);
DataPointer(int, Cart_demo_flag, 0x3C4ACA8);
DataArray(NJS_TEXANIM, cartsprite_score, 0x3C5D558, 13); // simply "score" in symbols
VoidFunc(InitActionScore, 0x427EF0);
DataPointer(Bool, boolOneShot, 0x3C52464);
FunctionPointer(void, CameraFilter, (task* tp), 0x436CD0);
FunctionPointer(void, DrawCharacterShadow, (taskwk* twp, shadowwk* swp), 0x49F0B0);
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
TaskFunc(relbox_switch_draw, 0x46A8D0);
DataPointer(int, MRaceResult, 0x3C53A94);
DataPointer(NJS_SPRITE, Spr_MRaceDisp, 0x3C53A98);
FunctionPointer(task*, SetDialogTask, (), 0x432C60);
DataPointer(pathtag, pathtag_hw1_heli0807, 0x26A72D4);
DataPointer(NJS_OBJECT, object_turnasi_oya_koa, 0x267D3B4);
DataPointer(NJS_OBJECT, object_turnasi_oya_kob, 0x267C7AC);
DataPointer(Bool, tornade_flag, 0x3C5D670);
FunctionPointer(Bool, NearTornade, (NJS_POINT3* pos, Float* dist), 0x4BA860);
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
FunctionPointer(Bool, IsMiniGameMenuEnabled, (), 0x506460);
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
FunctionPointer(Bool, isMissionClearDisp, (), 0x414FE0);
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
FunctionPointer(Bool, BigSetPosition, (NJS_POINT3* p, NJS_POINT3* v, Angle3* a, float r), 0x46F130);
TaskFunc(BigDisplayHit, 0x46EBC0);
TaskFunc(BigDisplayStatus, 0x470090);
TaskFunc(dispBigKaeru, 0x7A6BB0);
TaskFunc(BigDisplayFishWeight, 0x4701A0);
DataPointer(task*, pRd_Master, 0x3B0EFD8);
TaskFunc(Eggrob_Init, 0x4D3ED0);
VoidFunc(EV_NpcMilesStandByOff, 0x42CE20);
FunctionPointer(void, SetFreeCameraMode, (int sw), 0x4348A0);
DataPointer(SaveFileData, sd, 0x3C52558);
DataPointer(float, distance, 0x3C52AC8);
DataPointer(float, distancep, 0x3C52ACC);
FunctionPointer(float, BigChkHeavyWeight, (), 0x46F7D0);
FunctionPointer(int, getLureKind, (), 0x46C870);
DataPointer(float, No2CargoSpd, 0x3C72E5C);
FunctionPointer(void, CreateWater, (NJS_POINT3* pos, NJS_POINT3* velo, float scl), 0x4B9540);
TaskFunc(CartGetOffPlayer, 0x798C60);
TaskFunc(EnemyAir, 0x4AA340);
TaskFunc(EffectSkyScope, 0x5F1AA0);
TaskFunc(dispSkyScope, 0x5F19F0);
DataPointer(NJS_OBJECT, object_sky_scope_a_scope_a, 0x214E4B8);
TaskFunc(ObjectSkydeck_crane_hang_Draw, 0x5F2F50);
DataPointer(Angle, cloud_roll, 0x3C7F05C);
DataPointer(Angle, cloud_pitch, 0x3C7F028);
DataPointer(NJS_OBJECT, object_cl_uki_cl_uki, 0x1A29EEC);
DataPointer(NJS_OBJECT, object_mdlelv1_cl_elvtr_cl_elvtr, 0x1A3D74C);
DataPointer(NJS_ACTION, action_fun_funflot, 0x1A3037C);
FunctionPointer(int, PCheckJump, (taskwk* twp), 0x43BF40);
DataPointer(int, bWake, 0x912DF0); // Timer toggle
VoidFunc(late_execCancel, 0x403E60);
VoidFunc(sub_437100, 0x437100);
DataPointer(_OBJ_CAMERAPARAM, cameraParam, 0x3B2CB7C);
DataPointer(NJS_PLANE, plCollision, 0x915094);
FunctionPointer(void, DispTask, (int level), 0x40B4F0);
DataPointer(Sint16, ssAct, 0x3B2CAC8);
DataPointer(Float, clipZ_near, 0x90082C);
DataPointer(Float, clipZ_far, 0x900830);
DataPointer(NJS_CACTION*, pCameraAction, 0x3B2C950);
DataPointer(Float, fActionFrame, 0x3B2CA6C);
DataPointer(Sint32, camera_mode, 0x3B2C6C4); // no symbol name
FastcallFunctionPointer(Float, njDistancePL2PL, (NJS_PLANE* pl1, NJS_PLANE* pl2, NJS_LINE* l), 0x7887D0);
DataPointer(CCL_INFO, camera_sphere_info, 0x9BE8A8);
FunctionPointer(Bool, IsThisTaskPlayer, (task* tp), 0x441AD0);
VoidFunc(SetPlayer, 0x4157C0);
DataArray(PL_ACTION, big_action, 0x3C556A0, 105);
VoidFunc(_advertise_prolog, 0x5034A0);
VoidFunc(SetCartCameraDemo, 0x4DB4E0);
FunctionPointer(void, CreateHitmark, (NJS_POINT3* pos, Float scl), 0x4CAB40);
DataPointer(NJS_OBJECT, object_youUP_yogun_yogun, 0x2484D5C);
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
DataArray(Sint8[STAGE_NUMBER], flgCompletedActionStage, 0x3B18250, 8);
FunctionPointer(void, PPutHeldObject, (taskwk* twp, taskwk* htwp, playerwk* pwp), 0x4421F0);
FunctionPointer(Float, GetUnitVector, (NJS_POINT3* vec), 0x459AC0);
FunctionPointer(Float, orGetWaveHeight, (Float pos_x, Float pos_z), 0x4F8A00);
DataArray(Float, hasi_a_posy, 0x3C5E7F4, 2);
DataArray(Float, hasi_b_posy, 0x3C5E790, 25);
DataArray(Float, hasi_b_angz, 0x3C5E800, 25);
FunctionPointer(void, GM_SECall, (Sint32 se), 0x414170);
FunctionPointer(Sint32, CartDataGetBestTotalTime, (), 0x4DAC90);
FunctionPointer(Sint32, CartDataGetBestSubTotalTime, (Sint8 lap), 0x4DAB80);
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
FunctionPointer(void, Knux_RunsActions, (taskwk* twp, motionwk2* mwp, playerwk* pwp), 0x478020);
TaskFunc(dispKnuEffectChargeUpStay, 0x4C0FC0);
FunctionPointer(void, KnuEffectDrawTsubu, (NJS_POINT3* v, float r), 0x4C0F20);
TaskFunc(KnuEffectChargeUpStay, 0x4C0FF0);
FunctionPointer(void, KnuEffectPutChargeComp, (NJS_VECTOR* position, float alpha), 0x4C1330);
FunctionPointer(void, SetLocalPathCamera, (pathtag* path, Sint8 mode, int timer), 0x469300);
DataPointer(pathtag, pathtag_s_camera, 0x919BF4);
TaskFunc(E101_Main, 0x567fd0);
TaskFunc(E103_Main, 0x4e7e90);
TaskFunc(E104_Main, 0x605A90);
FastcallFunctionPointer(void, InitBgAct, (int a2, task* tp), 0x541D80);
TaskFunc(Past_Disp, 0x541E00);
DataPointer(int, MRTorokkoRideFlg, 0x3C6333C);
DataPointer(NJS_VECTOR, RidePos, 0x3C63330);
DataArray(NJS_ACTION*, MoveAction, 0x3C63340, 3);
DataPointer(NJS_VECTOR, XVec1, 0x111DD40);
DataPointer(Float, oldn, 0x90A098);
DataPointer(Float, oldf, 0x90A09C);
DataPointer(Float, SAL_telepotsonicmovespd, 0x1E777BC);
DataPointer(int, RdCasino_JumpAct, 0x3C7487C);
DataPointer(int, SAL_telepotsonicrotspd, 0x1E777B8);
DataPointer(void*, canselEvent, 0x3B2C578);
FunctionPointer(Sint32, CalcSpline, (SPLINE_DATA* spline_data), 0x7AAC70);
FunctionPointer(void, Free, (void* mp), 0x40B310);
FunctionPointer(void, lig_setLight4gjpalNo, (Sint32 no), 0x411F30);
DataArray(__int16*, plADXNamePlayingAsWaiting, 0x915CC8, 43);
DataArray(task*, btp, 0x3ABDBC4, 8);
FunctionPointer(void, DispChildrenTask, (task* tp), 0x40B130);
FunctionPointer(Sint32, GetRivalPlayerNumber, (Sint8 pno), 0x441BF0);
FunctionPointer(void, KnuEffectHormTubePut, (taskwk* twp), 0x4C14F0);
FunctionPointer(void, SetEffectSpray, (NJS_POINT3* pos, NJS_POINT3* vec), 0x4C1C20);
FunctionPointer(Sint32, GetPlayerCharacterName, (Uint8 pno), 0x441970);
FunctionPointer(Bool, OnEdit, (task* tp), 0x4F88A0);
FunctionPointer(void, SeqChangeStage, (Sint32 stg, Sint32 act), 0x412D80);
FunctionPointer(void, SeqClrFlag, (Sint32 no), 0x412D10);
FunctionPointer(void, CCL_Enable, (taskwk* twp, int no), 0x41C213);
FunctionPointer(void, CCL_Disable, (taskwk* twp, int no), 0x41C220);
FunctionPointer(Bool, ALW_CommunicationOff, (task* tp), 0x71A520);
FunctionPointer(Bool, ALW_IsHeld, (task* tp), 0x71A320);
DataPointer(Float, CamPosItpRatio, 0x33D0D48);
DataPointer(Float, CamPosItpRatioY, 0x33D0D4C);
DataPointer(Float, CamPosMaxSpd, 0x33D0D50);
DataPointer(Float, ChaoCameraWaterOffsetY, 0x33D0D54);
FunctionPointer(void, AL_DetectCollisionStandard, (task* tp), 0x73FE10);
FunctionPointer(MOVE_WORK*, MOV_Init, (task* tp), 0x73FE70);
FunctionPointer(void, MOV_SetGravity, (task* tp, Float g), 0x740040);
FunctionPointer(bool, IsSonicShakingTree, (taskwk* twp), 0x494810);
FunctionPointer(bool, IsKnucklesShakingTree, (taskwk* twp), 0x4767E0);
FunctionPointer(bool, IsAmyShakingTree, (taskwk* twp), 0x487250);
FunctionPointer(bool, IsE102ShakingTree, (taskwk* twp), 0x48CE50);
DataPointer(int, PinballFlag, 0x3C748F8);
DataPointer(char, ThreeCardCounter, 0x3C748EA);
FastcallFunctionPointer(Float, njDistanceP2PL, (NJS_VECTOR* a1, NJS_LINE* a2, NJS_VECTOR* a3), 0x788D40);
FastcallFunctionPointer(Float, njOuterProduct, (NJS_VECTOR* a1, NJS_VECTOR* a2, NJS_VECTOR* a3), 0x7889F0);

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
static inline Bool RoboHeadCaptureBeam(task* tp)
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
static inline int E102CheckInput(playerwk* pwp, taskwk* twp, motionwk2* mwp)
{
	int result;
	__asm
	{
		push[mwp]
		mov esi, [twp]
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
static inline int AmyCheckInput(playerwk* pwp, motionwk2* mwp, taskwk* twp)
{
	int result;
	__asm
	{
		mov esi, [twp]
		mov edi, [mwp]
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

static const void* const ScanNearPath2LPtr = (void*)0x5E4C50;
static inline Sint32 ScanNearPath2L(Sint32 frame, Sint32 pathpitch, NJS_POINT3* postgt, NJS_POINT3* pospath, Sint32 pathnum, Float* pdismin, NJS_POINT3* posnearpath, NJS_POINT3* vecnearpath)
{
	Sint32 result;
	__asm
	{
		push[vecnearpath]
		push[posnearpath]
		push[pdismin]
		push[pathnum]
		push[pospath]
		push[postgt]
		mov ecx, [pathpitch]
		mov eax, [frame]
		call ScanNearPath2LPtr
		mov result, eax
		add esp, 24
	}
	return result;
}

static const void* const ScanNearPath2LXZPtr = (void*)0x5C1F70;
static inline Sint32 ScanNearPath2LXZ(Sint32 frame, Sint32 pathpitch, NJS_POINT3* postgt, NJS_POINT3* pospath, Sint32 pathnum, Float* pdismin, NJS_POINT3* posnearpath, NJS_POINT3* vecnearpath)
{
	Sint32 result;
	__asm
	{
		push[vecnearpath]
		push[posnearpath]
		push[pdismin]
		push[pathnum]
		push[pospath]
		push[postgt]
		mov ecx, [pathpitch]
		mov eax, [frame]
		call ScanNearPath2LXZPtr
		mov result, eax
		add esp, 24
	}
	return result;
}

static const void* const CalcPathPosRangePtr = (void*)0x5E4920;
static inline void CalcPathPosRange(pathtag* pttp, Sint32 gaptop, Sint32 gapbottom, NJS_POINT3* vecrange, NJS_POINT3* posmin, NJS_POINT3* posmax)
{
	Sint32 result;
	__asm
	{
		push[posmax]
		push[posmin]
		push[gapbottom]
		mov ebx, [vecrange]
		mov edx, [pttp]
		mov eax, [gaptop]
		call CalcPathPosRangePtr
		add esp, 12
	}
}



static const void* const ReleaseTextureOnCasinoPtr = (void*)0x5C03F0;
static inline Sint32 ReleaseTextureOnCasino(Sint32 id)
{
	Sint32 result;
	__asm
	{
		mov eax, [id]
		call ReleaseTextureOnCasinoPtr
		mov result, eax
	}
	return result;
}


static const void* const CheckYakuLinePtr = (void*)0x5DAAF0;
static inline Uint8 CheckYakuLine(taskwk* twp)
{
	Uint8 result;
	__asm
	{
		mov edi, [twp]
		call CheckYakuLinePtr
		mov result, al
	}
	return result;
}
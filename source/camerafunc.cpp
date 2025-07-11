#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "VariableHook.hpp"
#include "splitscreen.h"
#include "sadx_utils.h"
#include "gravity.h"
#include "camera.h"
#include "camerafunc.h"

VariableHook<CAM_ANYPARAM, 0x3B2CA38> CamAnyParam_m;

/* CAMERA KLAMATH */
VariableHook<Float, 0x3C4ABC8> y_off_m;
VariableHook<Float, 0x3C4ADEC> last_y_off_m;
VariableHook<Bool, 0x3C4ABD4> klamath_LR_flag_m;

/* ADJUST THREE POINT */
VariableHook<NJS_POINT3, 0x3C4ABB8> inertia_m;
VariableHook<Angle3, 0x3C4AC1C> angle_c2p_m;
VariableHook<Angle3, 0x3C4AC38> angle_spd_m;
VariableHook<Sint8, 0x3C4ACBC> update_inertia_m;
VariableHook<Bool, 0x3C4AC9C> param_collision_m;
VariableHook<Float, 0x3C4AC60> param_max_time_m;   /* guessed name */
VariableHook<Sint32, 0x3C4ABD8> param_inertia_m;
VariableHook<Angle3, 0x3C4AC64> screen_in_ang_m;
VariableHook<Float, 0x3C4ABCC> adjust_point_m;

/* CAMERA CART */
VariableHook<NJS_POINT3, 0x3C4AB98> cart_save_pos_m; /* guessed name*/
VariableHook<NJS_POINT3, 0x3C4ACB0> cart_save_tgt_m;
VariableHook<Float, 0x3C4ABEC> air_pos_m;

/* PATHCAMERA1 */
VariableHook<pathtag*, 0x3C4AD88> pnowpathtag_m;
VariableHook<NJS_POINT3, 0x3C4AD7C> vecnearsonic_m;
VariableHook<NJS_POINT3, 0x3C4AD70> posSonic_m;
VariableHook<NJS_POINT3, 0x3C4AD64> vecCameraSpeed_m;
VariableHook<Sint32, 0x3C4AD60> nSonicFrame_m;
VariableHook<Sint32, 0x3C4AD5C> nCameraFramef_m;
VariableHook<Sint32, 0x3C4AD58> nCameraFrame_m;

/* PATHCAMERA2CORE */
VariableHook<NJS_POINT3, 0x3C4ABFC> CamPathCam2Core_Pos_m;
VariableHook<Angle3, 0x3C4ACDC> CamPathCam2Core_Angle_m;
VariableHook<Bool, 0x3C4AC98> CamPathCam2Core_AliveFlag_m;

/* AVOID */
VariableHook<Sint32, 0x3C4AC8C> avoidMode_m;
VariableHook<Sint32, 0x3C4ADC4> nowFrame_m;

/* BOSSES */
VariableHook<NJS_POINT3, 0x3C4ACE8> chaoscam_tgtpos_ofs_def_m;
VariableHook<NJS_POINT3, 0x91B610> chaoscam_campos_ofs_def_m;
VariableHook<NJS_POINT3, 0x3C4ACC4> chaoscam_tgtpos_ofs_m;
VariableHook<Angle3, 0x3C4AC54> chaoscam_angacc_lim_def_m;
VariableHook<Angle3, 0x91B628> chaoscam_angspd_lim_def_m;
VariableHook<NJS_POINT3, 0x91B634> chaoscam_camspd_lim_def_m;

FastFunctionHook<void, _OBJ_CAMERAPARAM*> PathCamera1_h(0x4653E0);

DataPointer(Sint32, demo_count, 0x3C4ACC0);

struct CameraLocalPathData
{
	pathtag* ptp;
	Sint8 mode;
	Sint32 timer;
	Uint16 point;
	Sint8 path_switch;
};

CameraLocalPathData cameraLocalPathData[PLAYER_MAX];

CAM_ANYPARAM* GetCamAnyParam(int pnum)
{
	return &CamAnyParam_m[pnum];
}

void SetRuinWaka1Data(NJS_POINT3* pos, NJS_POINT3* tgt, int pnum)
{
	if (pnum == 0)
	{
		camera_twp->scl = *pos;
		camera_twp->counter.f = tgt->x;
		camera_twp->timer.f = tgt->y;
		camera_twp->value.f = tgt->z;
	}

	if (auto param = GetCamAnyParam(pnum))
	{
		param->camAnyParamPos = *pos;
		param->camAnyParamTgt = *tgt;
	}
}

bool GetRuinWaka1Data(NJS_POINT3* pos, NJS_POINT3* tgt, int pnum)
{
	if (auto param = GetCamAnyParam(pnum))
	{
		if (pos) 
			*pos = param->camAnyParamPos;
		if (tgt) 
			*tgt = param->camAnyParamTgt;

		return true;
	}

	return false;
}

void gravDispose3_m(int pnum)
{
	NJS_VECTOR up = { 0.0f, 1.0f, 0.0f };

	if (GetStageNumber() == 0x602)
	{
		NJS_VECTOR v;
		v.x = camcont_wp->camxpos - camcont_wp->tgtxpos;
		v.z = camcont_wp->camzpos - camcont_wp->tgtzpos;

		Angle gz = angGz;
		Angle gx = angGx;

		gravity::GetUserGravity(pnum, nullptr, &gx, &gz);

		njUnitVector(&up);
		njPushMatrix(0);
		njUnitMatrix(0);
		ROTATEZ(0, gz);
		ROTATEX(0, gx);
		njCalcVector(0, &up, &up);
		njPopMatrix(1);

		Angle roll = njArcSin(njSqrt(up.z * up.z + up.x * up.x));

		v.y = 0.0f;
		up.y = 0.0f;
		njUnitVector(&v);
		njUnitVector(&up);

		if (roll & 0x8000)
		{
			roll |= 0xFFFF0000;
		}
		else
		{
			roll &= 0xFFFF;
		}

		Float diff = v.z * up.z + v.x * up.x; /* distance between up vector and camera direction */
		roll *= (Angle)(1.0f - (diff * diff));

		if (v.z * up.z - v.x * up.x < 0.0f)
		{
			camcont_wp->angz = -roll;
		}
		else
		{
			camcont_wp->angz = roll;
		}
	}
	else
	{
		camcont_wp->angz = 0;
	}
}

void __cdecl CameraKlamath_m(_OBJ_CAMERAPARAM* pParam)
{
	if (!splitscreen::IsActive())
	{
		CameraKlamath(pParam);
		return;
	}

	auto pnum = TASKWK_PLAYERID(playertwp[0]);
	auto ptwp = playertwp[pnum];
	auto& _y_off = y_off_m[pnum];
	auto& _last_y_off = last_y_off_m[pnum];
	auto& _klamath_LR_flag = klamath_LR_flag_m[pnum];

	if (pParam->ulTimer)
	{
		if (_klamath_LR_flag)
		{
			/* Revert to KNUCKLE camera for LR logic */
			CameraKnuckle(pParam);
			if ((perG[0].l - 128) << 8 <= 128 && (perG[0].r - 128) << 8 <= 128 && Player_stop_flag == FALSE)
			{
				_klamath_LR_flag = Player_stop_flag;
				SetAdjustMode(CAMADJ_RELATIVE6C);
			}
			return;
		}
	}
	else
	{
		/* If xCamAng is set, overide Y distance */
		if (pParam->xCamAng)
		{
			_y_off = (Float)pParam->xCamAng * 0.1f;
		}
		else
		{
			_y_off = pParam->fDistance * 0.5f;
		}

		_last_y_off = 0.0f;
		_klamath_LR_flag = FALSE;
	}

	/* Check if LR mode should trigger */
	if ((perG[0].l - 128) << 8 > 128 || (perG[0].r - 128) << 8 > 128)
	{
		if (CheckPadReadModeP(pnum))
		{
			_klamath_LR_flag = TRUE;
		}
	}

	NJS_POINT3 save = camera_twp->pos;

	camcont_wp->tgtxpos = ptwp->pos.x;
	camcont_wp->tgtypos = ptwp->pos.y;
	camcont_wp->tgtzpos = ptwp->pos.z;

	NJS_VECTOR v;
	v.y = 0.0f;
	v.x = ptwp->pos.x - pParam->xDirPos;
	v.z = ptwp->pos.z - pParam->zDirPos;

	njUnitVector(&v);
	camcont_wp->camxpos = v.x * pParam->fDistance + camcont_wp->tgtxpos;
	camcont_wp->camypos = _y_off + camcont_wp->tgtypos;
	camcont_wp->camzpos = v.z * pParam->fDistance + camcont_wp->tgtzpos;

	if (!(ptwp->flag & (Status_Ground | Status_OnColli)) && pParam->yCamAng == 0)
	{
		_last_y_off -= 0.2f;
	}
	else
	{
		_last_y_off *= 0.9f;
	}

	camcont_wp->camypos -= _last_y_off;

	v.y = 0.0f;
	v.x = ptwp->pos.x - pParam->xDirPos;
	v.z = ptwp->pos.z - pParam->zDirPos;

	Float r = njUnitVector(&v);
	camcont_wp->tgtypos += ((r - pParam->fDistance) / r) * _y_off;
	CameraAdditionalCollision((NJS_POINT3*)&camcont_wp->camxpos);
	CameraAdditionalPlane(&save, (NJS_POINT3*)&camcont_wp->camxpos);
	CameraPositionSmooth(&save, (NJS_POINT3*)&camcont_wp->camxpos);
	CameraCollisitonCheck(&save, (NJS_POINT3*)&camcont_wp->camxpos);
	gravDispose3_m(pnum);
}

void sub_466790_m(int pnum)
{
	if (GetCamAnyParam(pnum)->camAnyParamTgt.x == 0.0f)
	{
		air_pos_m[pnum] = 0.0f;
	}
	else
	{
		air_pos_m[pnum] += 0.5f;
	}
}

void sub_466420_m(Float height, int pnum)
{
	auto CamAnyParam = GetCamAnyParam(pnum);
	camcont_wp->camxpos = CamAnyParam->camAnyParamPos.x;
	camcont_wp->camypos = CamAnyParam->camAnyParamPos.y + air_pos_m[pnum] + height;
	camcont_wp->camzpos = CamAnyParam->camAnyParamPos.z;
	camcont_wp->tgtxpos = CamAnyParam->camAnyParamPos.x;
	camcont_wp->tgtypos = CamAnyParam->camAnyParamPos.y;
	camcont_wp->tgtzpos = CamAnyParam->camAnyParamPos.z;
}

void calcCartCamPos_m(int pnum)
{
	auto CamAnyParam = GetCamAnyParam(pnum);
	NJS_POINT3 v;

	if (camcont_wp->cammode || GetPlayerNumber() != Characters_Big)
	{
		v = camCartData[camcont_wp->cammode].transCam;
	}
	else
	{
		v = { 0.0f, 20.0f, 15.0f };
	}

	calcModerateVector(&v, (Angle3*)&CamAnyParam->camAnyParamAng);
	camcont_wp->camxpos = v.x + camcont_wp->camxpos;
	camcont_wp->camypos = v.y + camcont_wp->camypos;
	camcont_wp->camzpos = v.z + camcont_wp->camzpos;
}

void calcCartCamTgt_m(int pnum)
{
	auto CamAnyParam = GetCamAnyParam(pnum);

	NJS_POINT3 v;
	v.x = camCartData[camcont_wp->cammode].transTgt.x;
	v.y = camCartData[camcont_wp->cammode].transTgt.y;
	v.z = camCartData[camcont_wp->cammode].transTgt.z;

	njPushMatrix(nj_unit_matrix_);
	ROTATEZ(0, CamAnyParam->camAnyParamAng.z);
	ROTATEX(0, CamAnyParam->camAnyParamAng.x);
	ROTATEY(0, CamAnyParam->camAnyParamAng.y);
	njCalcVector(0, &v, &v);
	njPopMatrixEx();

	camcont_wp->tgtxpos = v.x + camcont_wp->tgtxpos;
	camcont_wp->tgtypos = v.y + camcont_wp->tgtypos;
	camcont_wp->tgtzpos = v.z + camcont_wp->tgtzpos;
}

void calcCartCamAdjForFollow_m(int pnum)
{
	auto& cart_save_pos = cart_save_pos_m[pnum];
	auto& cart_save_tgt = cart_save_tgt_m[pnum];

	camcont_wp->camxpos = (camcont_wp->camxpos - cart_save_pos.x) * 0.2f + cart_save_pos.x;
	camcont_wp->camypos = (camcont_wp->camypos - cart_save_pos.y) * 0.2f + cart_save_pos.y;
	camcont_wp->camzpos = (camcont_wp->camzpos - cart_save_pos.z) * 0.2f + cart_save_pos.z;
	camcont_wp->tgtxpos = (camcont_wp->tgtxpos - cart_save_tgt.x) * 0.2f + cart_save_tgt.x;
	camcont_wp->tgtypos = (camcont_wp->tgtypos - cart_save_tgt.y) * 0.2f + cart_save_tgt.y;
	camcont_wp->tgtzpos = (camcont_wp->tgtzpos - cart_save_tgt.z) * 0.2f + cart_save_tgt.z;
}

void cartCameraDemo_m(int pnum)
{
	auto& cart_save_pos = cart_save_pos_m[pnum];
	auto& cart_save_tgt = cart_save_tgt_m[pnum];
	auto CamAnyParam = GetCamAnyParam(pnum);

	if (demo_count == CamAnyParam->camAnyTmpSint32[1] + 70)
	{
		if (pnum == 0)
		{
			Cart_demo_flag = 0;
		}

		camcont_wp->cammode = 1;
		camcont_wp->camxpos = (camcont_wp->camxpos - cart_save_pos.x) * 0.05f + cart_save_pos.x;
		camcont_wp->camypos = (camcont_wp->camypos - cart_save_pos.y) * 0.05f + cart_save_pos.y;
		camcont_wp->camzpos = (camcont_wp->camzpos - cart_save_pos.z) * 0.05f + cart_save_pos.z;
		camcont_wp->tgtxpos = (camcont_wp->tgtxpos - cart_save_tgt.x) * 0.05f + cart_save_tgt.x;
		camcont_wp->tgtypos = (camcont_wp->tgtypos - cart_save_tgt.y) * 0.05f + cart_save_tgt.y;
		camcont_wp->tgtzpos = (camcont_wp->tgtzpos - cart_save_tgt.z) * 0.05f + cart_save_tgt.z;
	}
	else if (demo_count <= CamAnyParam->camAnyTmpSint32[1] + 10)
	{
		if (demo_count <= CamAnyParam->camAnyTmpSint32[1])
		{
			Float* v3 = (float*)(CamAnyParam->camAnyTmpSint32[0] + 24 * demo_count);
			camcont_wp->camxpos = *v3;
			camcont_wp->camypos = v3[1];
			camcont_wp->camzpos = v3[2];
			camcont_wp->tgtxpos = v3[3];
			camcont_wp->tgtypos = v3[4];
			camcont_wp->tgtzpos = v3[5];
		}
	}
	else
	{

		auto save = camcont_wp->cammode;
		camcont_wp->cammode = 1;
		sub_466420_m(playerpwp[pnum]->p.eyes_height, pnum);
		calcCartCamPos_m(pnum);
		calcCartCamTgt_m(pnum);
		camcont_wp->cammode = save;
		camcont_wp->camxpos = (camcont_wp->camxpos - cart_save_pos.x) * 0.05f + cart_save_pos.x;
		camcont_wp->camypos = (camcont_wp->camypos - cart_save_pos.y) * 0.05f + cart_save_pos.y;
		camcont_wp->camzpos = (camcont_wp->camzpos - cart_save_pos.z) * 0.05f + cart_save_pos.z;
		camcont_wp->tgtxpos = (camcont_wp->tgtxpos - cart_save_tgt.x) * 0.05f + cart_save_tgt.x;
		camcont_wp->tgtypos = (camcont_wp->tgtypos - cart_save_tgt.y) * 0.05f + cart_save_tgt.y;
		camcont_wp->tgtzpos = (camcont_wp->tgtzpos - cart_save_tgt.z) * 0.05f + cart_save_tgt.z;
	}

	if (pnum == 0)
	{
		demo_count += 1;
	}

	cart_save_pos = *(NJS_POINT3*)&camcont_wp->camxpos;
	cart_save_tgt = *(NJS_POINT3*)&camcont_wp->tgtxpos;
}

void __cdecl CameraCart_m(_OBJ_CAMERAPARAM* pParam)
{
	if (!splitscreen::IsActive())
	{
		CameraCart(pParam);
		return;
	}

	auto pnum = TASKWK_PLAYERID(playertwp[0]);
	auto ptwp = playertwp[pnum];
	auto ppwp = playerpwp[pnum];

	auto& cart_save_pos = cart_save_pos_m[pnum];
	auto& cart_save_tgt = cart_save_tgt_m[pnum];
	auto& air_pos = air_pos_m[pnum];

	auto save = camera_twp->pos;

	if (pParam->ulTimer == 0)
	{
		camcont_wp->cammode = 1;
		cart_save_pos = ptwp->pos;
		cart_save_tgt = ptwp->pos;
		cart_save_pos.y += ppwp->p.eyes_height;
		air_pos = 0.0f;
	}

	if (Cart_demo_flag == 1)
	{
		SetAdjustMode(0);
		ChangeCamsetMode(2);
		cartCameraDemo_m(pnum);
		return;
	}

	if (pnum == 0)
	{
		demo_count = 0;
	}


	if (Cart_demo_flag == 2)
	{
		SetAdjustMode(0);
		//cameraDebugMake(CamAnyParam.camAnyTmpSint32[1], CamAnyParam.camAnyTmpSint32[0]);
		ChangeCamsetMode(1);
		return;
	}

	ChangeCamsetMode(2);
	
	switch (camcont_wp->cammode)
	{
	case 0:
		sub_466420_m(ppwp->p.eyes_height * 0.25f, pnum);
		calcCartCamPos_m(pnum);
		calcCartCamTgt_m(pnum);

		camcont_wp->tgtxpos = (camcont_wp->tgtxpos - cart_save_tgt.x) * 0.2f + cart_save_tgt.x;
		camcont_wp->tgtypos = (camcont_wp->tgtypos - cart_save_tgt.y) * 0.2f + cart_save_tgt.y;
		camcont_wp->tgtzpos = (camcont_wp->tgtzpos - cart_save_tgt.z) * 0.2f + cart_save_tgt.z;

		break;
	case 1:
		sub_466790_m(pnum);
		sub_466420_m(ppwp->p.eyes_height, pnum);
		calcCartCamPos_m(pnum);
		calcCartCamTgt_m(pnum);
		calcCartCamAdjForFollow_m(pnum);
		break;
	case 2:
		sub_466420_m(ppwp->p.eyes_height, pnum);
		calcCartCamPos_m(pnum);
		calcCartCamTgt_m(pnum);
		calcCartCamAdjForFollow_m(pnum);
		break;
	default:
		camcont_wp->cammode = 1;
		break;
	}

	camcont_wp->angz = 0;


	if (camcont_wp->cammode == 1)
	{
		auto x = ptwp->pos.x - camcont_wp->camxpos;
		auto z = ptwp->pos.z - camcont_wp->camzpos;

		CameraCollisitonCheck(&save, (NJS_POINT3*)&camcont_wp->camxpos);

		auto x2 = ptwp->pos.x - camcont_wp->camxpos;
		auto y2 = ptwp->pos.z - camcont_wp->camzpos;

		if ((x2 * x2 + y2 * y2) - (x * x + z * z) > 0.0f)
		{
			camcont_wp->camxpos = x;
			camcont_wp->camzpos = z;
		}
	}
	else
	{
		CameraCollisitonCheck(&save, (NJS_POINT3*)&camcont_wp->camxpos);
	}

	cart_save_pos = *(NJS_POINT3*)&camcont_wp->camxpos;
	cart_save_tgt = *(NJS_POINT3*)&camcont_wp->tgtxpos;

	auto press = per[pnum]->press;

	if (press & Buttons_Up)
	{
		++camcont_wp->cammode;

		if (camcont_wp->cammode >= 3u)
		{
			camcont_wp->cammode = 0;
		}
	}
	else if (press & Buttons_Down)
	{
		--camcont_wp->cammode;
	}
}

// Make this use CamAnyParam as it should have...
void __cdecl CameraRuinWaka1_m(_OBJ_CAMERAPARAM* pParam)
{
	if (!splitscreen::IsActive())
	{
		CameraRuinWaka1(pParam);
		return;
	}

	auto pnum = TASKWK_PLAYERID(playertwp[0]);

	if (pnum == 0)
	{
		camcont_wp->camxpos = camera_twp->scl.x;
		camcont_wp->camypos = camera_twp->scl.y;
		camcont_wp->camzpos = camera_twp->scl.z;
		camcont_wp->tgtxpos = camera_twp->counter.f;
		camcont_wp->tgtypos = camera_twp->timer.f;
		camcont_wp->tgtzpos = camera_twp->value.f;
	}
	else
	{
		auto param = GetCamAnyParam(pnum);

		if (param)
		{
			camcont_wp->camxpos = param->camAnyParamPos.x;
			camcont_wp->camypos = param->camAnyParamPos.y;
			camcont_wp->camzpos = param->camAnyParamPos.z;
			camcont_wp->tgtxpos = param->camAnyParamTgt.x;
			camcont_wp->tgtypos = param->camAnyParamTgt.y;
			camcont_wp->tgtzpos = param->camAnyParamTgt.z;
			camcont_wp->angz = 0;
		}
	}
}

void __cdecl PathCamera1_m(_OBJ_CAMERAPARAM* pParam)
{
	if (!splitscreen::IsActive())
	{
		PathCamera1_h.Original(pParam);
		return;
	}

	auto pnum = TASKWK_PLAYERID(playertwp[0]);
	auto ptwp = playertwp[pnum];
	auto pathwk = pnum == 0 ? (PATHCAMERA1WORK*)camera_twp->counter.ptr : (PATHCAMERA1WORK*)GetCamAnyParam(pnum)->camAnyTmpSint32[0];

	auto& pnowpathtag = pnowpathtag_m[pnum];
	auto& vecnearsonic = vecnearsonic_m[pnum];
	auto& posSonic = posSonic_m[pnum];
	auto& vecCameraSpeed = vecCameraSpeed_m[pnum];
	auto& nSonicFrame = nSonicFrame_m[pnum];
	auto& nCameraFramef = nCameraFramef_m[pnum];
	auto& nCameraFrame = nCameraFrame_m[pnum];

	Bool initialize;

	if (pathwk->pPathTag != pnowpathtag || pParam->ulTimer == 0
		|| vecnearsonic.x == 0.0f && vecnearsonic.y == 0.0f && vecnearsonic.z == 0.0f)
	{
		initialize = 1;
		pParam->ulTimer = 1;
		camcont_wp->camxpos = camera_twp->pos.x;
		camcont_wp->camypos = camera_twp->pos.y;
		camcont_wp->camzpos = camera_twp->pos.z;
		pnowpathtag = pathwk->pPathTag;
	}
	else
	{
		initialize = 0;
	}

	camcont_wp->angx = camera_twp->ang.x;
	camcont_wp->angy = camera_twp->ang.y;
	camcont_wp->angz = camera_twp->ang.z;

	NJS_POINT3 v, orig;
	v.x = 0.0f;
	v.y = pathwk->fSonicSize;
	v.z = 0.0f;
	njPushMatrix(_nj_unit_matrix_);
	njTranslateEx(&ptwp->pos);
	ROTATEZ(0, ptwp->ang.z);
	ROTATEX(0, ptwp->ang.x);
	ROTATEY(0, ptwp->ang.y);
	njCalcPoint(0, &v, &orig);
	njPopMatrixEx();

	if (initialize)
	{
		posSonic = orig;
		camera_twp->smode = 0;
		vecCameraSpeed.x = 16.0f;
		vecCameraSpeed.y = 16.0f;
		vecCameraSpeed.z = 16.0f;
		nSonicFrame = PC1_SearchNearPath(&posSonic, pathwk);
		nCameraFramef = nSonicFrame;
		nCameraFrame = nSonicFrame;
	}

	posSonic.x = (orig.x - posSonic.x) * 0.98f + posSonic.x;
	posSonic.y = (orig.y - posSonic.y) * 0.98f + posSonic.y;
	posSonic.z = (orig.z - posSonic.z) * 0.98f + posSonic.z;

	NJS_POINT3 posonpath, vecnear;
	PC1_PathMoveScan(&nSonicFrame, &posonpath, &posSonic, &vecnear, pathwk);

	if (pathwk->modeflag & 0x8)
	{
		if (GetDistance(&posSonic, &posonpath) <= pathwk->fPathCameraRangeOut)
		{
			camera_twp->smode &= ~0x1;
		}
		else
		{
			camera_twp->smode |= 0x1;
		}
	}

	njUnitVector(&vecnear);
	if (initialize)
	{
		vecnearsonic = vecnear;
	}
	else
	{
		vecnearsonic.x = (vecnear.x - vecnearsonic.x) * 0.3f + vecnearsonic.x;
		vecnearsonic.y = (vecnear.y - vecnearsonic.y) * 0.3f + vecnearsonic.y;
		vecnearsonic.z = (vecnear.z - vecnearsonic.z) * 0.3f + vecnearsonic.z;
		njUnitVector(&vecnearsonic);
	}

	camera_twp->smode &= ~0x4;

	// Check if the path ends behind player
	NJS_POINT3 postgt_behind, posonpath_behind;
	postgt_behind.x = vecnearsonic.x * pathwk->fBackPathDist + posSonic.x;
	postgt_behind.y = vecnearsonic.y * pathwk->fBackPathDist + posSonic.y;
	postgt_behind.z = vecnearsonic.z * pathwk->fBackPathDist + posSonic.z;
	PC1_PathMoveScan(&nCameraFrame, &posonpath_behind, &postgt_behind, 0, pathwk);
	if (nCameraFrame >= pathwk->pPathTag->points - 3 || !nCameraFrame)
	{
		camera_twp->smode |= 0x4;
	}

	// Check if if the path ends past player
	NJS_POINT3 postgt_forward, posonpath_forward;
	postgt_forward.x = posSonic.x - vecnearsonic.x * pathwk->fForwardPathDist;
	postgt_forward.y = posSonic.y - vecnearsonic.y * pathwk->fForwardPathDist;
	postgt_forward.z = posSonic.z - vecnearsonic.z * pathwk->fForwardPathDist;
	PC1_PathMoveScan(&nCameraFramef, &posonpath_forward, &postgt_forward, 0, pathwk);
	if (nCameraFramef >= pathwk->pPathTag->points - 3 || !nCameraFramef)
	{
		camera_twp->smode |= 0x4;
	}

	NJS_POINT3 offset;
	offset.x = posSonic.x - posonpath.x;
	offset.y = posSonic.y - posonpath.y;
	offset.z = posSonic.z - posonpath.z;

	NJS_POINT3 offset_behind, offset_forward;
	offset_behind = offset;
	offset_forward = offset;

	if (pathwk->modeflag & 0x1)
	{
		camera_twp->smode &= ~0x2;

		Float dist1 = njScalor(&offset_behind);

		beamhitstr bhs1;
		bhs1.reach = dist1;
		bhs1.pos = posonpath_behind;
		bhs1.vec = offset_behind;

		if (CL_ColPolBeamHit(&bhs1) == TRUE && bhs1.dist <= dist1)
		{
			if (pathwk->modeflag & 0x4)
			{
				Float dist2 = njScalor(&offset);

				beamhitstr bhs2;
				bhs2.reach = dist2;
				bhs2.pos = posonpath;
				bhs2.vec = offset;

				if (CL_ColPolBeamHit(&bhs2) == TRUE && bhs2.dist <= dist2)
				{
					camera_twp->smode |= 0x2;
				}
			}

			offset_behind.x = bhs1.hitpos.x - bhs1.pos.x;
			offset_behind.y = bhs1.hitpos.y - bhs1.pos.y;
			offset_behind.z = bhs1.hitpos.z - bhs1.pos.z;
		}

		Float scalor = njScalor(&offset_behind) / dist1;
		offset_forward.x *= scalor;
		offset_forward.y *= scalor;
		offset_forward.z *= scalor;
	}

	if (pathwk->fCameraSize != 0.0)
	{
		Float dist = njScalor(&offset_behind);
		Float scalor = (dist - pathwk->fCameraSize) / dist;
		if (scalor < 0.0f)
		{
			scalor = 0.0f;
		}
		offset_behind.x *= scalor;
		offset_behind.y *= scalor;
		offset_behind.z *= scalor;
		offset_forward.x *= scalor;
		offset_forward.y *= scalor;
		offset_forward.z *= scalor;
	}

	NJS_POINT3 vec_behind;
	vec_behind.x = offset_behind.x * pathwk->fBackPathMul + pathwk->vecCamOfs.x + posonpath_behind.x;
	vec_behind.y = offset_behind.y * pathwk->fBackPathMul + pathwk->vecCamOfs.y + posonpath_behind.y;
	vec_behind.z = offset_behind.z * pathwk->fBackPathMul + pathwk->vecCamOfs.z + posonpath_behind.z;

	NJS_POINT3 vec_forward;
	vec_forward.x = offset_forward.x * pathwk->fForwardPathMul + posonpath_forward.x;
	vec_forward.y = offset_forward.y * pathwk->fForwardPathMul + posonpath_forward.y;
	vec_forward.z = offset_forward.z * pathwk->fForwardPathMul + posonpath_forward.z;

	if (pathwk->modeflag & 0x2)
	{
		NJS_POINT3 diff;
		diff.x = vec_behind.x - camcont_wp->camxpos;
		diff.y = vec_behind.y - camcont_wp->camypos;
		diff.z = vec_behind.z - camcont_wp->camzpos;

		offset_forward.x = diff.x - vecCameraSpeed.x;
		offset_forward.y = diff.y - vecCameraSpeed.y;
		offset_forward.z = diff.z - vecCameraSpeed.z;

		Float dist1 = njScalor(&offset_forward);
		Float dist2 = njSqrt((vec_behind.x - posSonic.x) * (vec_behind.x - posSonic.x)
			+ (vec_behind.y - posSonic.y) * (vec_behind.y - posSonic.y)
			+ (vec_behind.z - posSonic.z) * (vec_behind.z - posSonic.z));

		if (vecCameraSpeed.z * vecCameraSpeed.z + vecCameraSpeed.y * vecCameraSpeed.y + vecCameraSpeed.x * vecCameraSpeed.x > diff.z * diff.z + diff.y * diff.y + diff.x * diff.x)
		{
			dist2 = dist1;
		}

		if (dist1 > dist2)
		{
			Float scalor = dist2 / dist1;
			offset_forward.x = offset_forward.x * scalor;
			offset_forward.y = offset_forward.y * scalor;
			offset_forward.z = offset_forward.z * scalor;
		}

		vecCameraSpeed.x = offset_forward.x + vecCameraSpeed.x;
		vecCameraSpeed.y = offset_forward.y + vecCameraSpeed.y;
		vecCameraSpeed.z = offset_forward.z + vecCameraSpeed.z;

		vec_behind.x = vecCameraSpeed.x + camcont_wp->camxpos;
		vec_behind.y = vecCameraSpeed.y + camcont_wp->camypos;
		vec_behind.z = vecCameraSpeed.z + camcont_wp->camzpos;
	}

	if (pathwk->modeflag & 0x10)
	{
		vec_behind.x = pathwk->fCameraAccMul * (vec_behind.x - camcont_wp->camxpos) + camcont_wp->camxpos;
		vec_behind.y = pathwk->fCameraAccMul * (vec_behind.y - camcont_wp->camypos) + camcont_wp->camypos;
		vec_behind.z = pathwk->fCameraAccMul * (vec_behind.z - camcont_wp->camzpos) + camcont_wp->camzpos;
	}

	Float xdist = vec_behind.x - vec_forward.x;
	Float zdist = vec_behind.z - vec_forward.z;

	camcont_wp->camxpos = vec_behind.x;
	camcont_wp->camypos = vec_behind.y;
	camcont_wp->camzpos = vec_behind.z;
	camcont_wp->angx = SubAngle(camcont_wp->angx, njArcTan2(vec_forward.y - vec_behind.y, njSqrt(xdist * xdist + zdist * zdist))) * pathwk->angCamSpdMul + camcont_wp->angx;
	camcont_wp->angy = SubAngle(camcont_wp->angy, njArcTan2(xdist, zdist)) * pathwk->angCamSpdMul + camcont_wp->angy;

	CamcontSetCameraCAMSTATUS(camera_twp);
}

void __cdecl PathCamera2Core_m(_OBJ_CAMERAPARAM* pParam)
{
	auto pnum = TASKWK_PLAYERID(playertwp[0]);
	auto CamPathCam2Core_Pos = CamPathCam2Core_Pos_m[pnum];
	auto CamPathCam2Core_Angle = CamPathCam2Core_Angle_m[pnum];

	camcont_wp->camxpos = CamPathCam2Core_Pos.x;
	camcont_wp->camypos = CamPathCam2Core_Pos.y;
	camcont_wp->camzpos = CamPathCam2Core_Pos.z;
	camcont_wp->angx = CamPathCam2Core_Angle.x;
	camcont_wp->angy = CamPathCam2Core_Angle.y;
	camcont_wp->angz = CamPathCam2Core_Angle.z;
	CamPathCam2Core_AliveFlag_m[pnum] = TRUE;
	CamcontSetCameraCAMSTATUS(camera_twp);
}

void __cdecl CameraAvoid_r(_OBJ_CAMERAPARAM* pParam)
{
	auto pnum = TASKWK_PLAYERID(playertwp[0]);
	auto ptwp = playertwp[pnum];

	auto& avoidMode = avoidMode_m[pnum];
	auto& nowFrame = nowFrame_m[pnum];

	if (avoidMode == AVOID_MODE_INIT)
	{
		nowFrame = 0;
		avoidMode = AVOID_MODE_CHASE;
	}
	else if (avoidMode != AVOID_MODE_CHASE)
	{
		if (avoidMode < AVOID_MODE_KNUCKLES)
		{
			switch (GetCameraMode_m(pnum))
			{
			case CAMMD_A_AVOID:
				SetCameraMode_m(pnum, CAMMD_A_KNUCKLES);
				break;
			case CAMMD_C_AVOID:
				SetCameraMode_m(pnum, CAMMD_C_KNUCKLES);
				break;
			case CAMMD_COL_AVOID:
				SetCameraMode_m(pnum, CAMMD_COL_KNUCKLES);
				break;
			default:
				SetCameraMode_m(pnum, CAMMD_KNUCKLES);
				SetAdjustMode_m(pnum, CAMADJ_RELATIVE4C);
				break;
			}
		}
	}

	Float frame = (Float)nowFrame / 90.0f;
	Float numf = 49.0f - frame * 49.0f;
	Sint32 num = (Sint32)(njFloor(numf) + 1.0f);

	NJS_VECTOR* pos_now = FollowRingData_m(pnum, num);
	NJS_VECTOR* pos_next = FollowRingData_m(pnum, num + 1);

	NJS_VECTOR pos;
	Float inv_numf = numf - num;
	pos.x = (pos_next->x - pos_now->x) * inv_numf + pos_now->x;
	pos.y = (pos_next->y - pos_now->y) * inv_numf + pos_now->y;
	pos.z = (pos_next->z - pos_now->z) * inv_numf + pos_now->z;

	camcont_wp->camxpos += (pos.x - camcont_wp->camxpos) * frame + camcont_wp->camxpos;
	camcont_wp->camypos += (pos.y - camcont_wp->camypos) * frame + camcont_wp->camypos;
	camcont_wp->camzpos += (pos.z - camcont_wp->camzpos) * frame + camcont_wp->camzpos;

	camcont_wp->tgtxpos = ptwp->pos.x;
	camcont_wp->tgtypos = ptwp->pos.y + GetPlayerWorkPtr(pnum)->p.eyes_height;
	camcont_wp->tgtzpos = ptwp->pos.z;
	
	pos.x = camcont_wp->camxpos - camcont_wp->tgtxpos;
	pos.y = camcont_wp->camypos - camcont_wp->tgtypos;
	pos.z = camcont_wp->camzpos - camcont_wp->tgtzpos;

	if (njScalor(&pos) < 20.0f)
	{
		avoidMode = AVOID_MODE_KNUCKLES;
	}
	if (++nowFrame >= 90)
	{
		avoidMode = AVOID_MODE_KNUCKLES;
	}
}

void __cdecl AdjustNormal_m(taskwk* pTaskWork, taskwk* pOldTaskWork, _OBJ_ADJUSTPARAM* pCameraAdjustWork)
{
	if (!splitscreen::IsActive())
	{
		AdjustNormal(pTaskWork, pOldTaskWork, pCameraAdjustWork);
		return;
	}

	auto pnum = TASKWK_PLAYERID(playertwp[0]);
	auto ptwp = playertwp[pnum];

	NJS_POINT3 dst;
	dst.x = pTaskWork->pos.x - ptwp->pos.x;
	dst.y = pTaskWork->pos.y - ptwp->pos.y;
	dst.z = pTaskWork->pos.z - ptwp->pos.z;
	Float dst_radius = njScalor(&dst);

	NJS_POINT3 src;
	GetPlayerPosition(pnum, 2, &src, 0);
	src.x = pOldTaskWork->pos.x - src.x;
	src.y = pOldTaskWork->pos.y - src.y;
	src.z = pOldTaskWork->pos.z - src.z;
	Float src_radius = njScalor(&src);

	Float dist = max(src_radius + (dst_radius - src_radius) * 0.3f, 20.0f);

	if (camcont_wp->ssFlag & 1)
	{
		camcont_wp->ssFlag &= ~1;
	}
	else
	{
		Angle angy = SubAngle(pOldTaskWork->ang.y, pTaskWork->ang.y);

		if (angy & 0x8000)
		{
			angy |= 0xFFFF0000;
		}

		if (angy > -0x40 && angy < 0x40)
		{
			camcont_wp->angy = pTaskWork->ang.y;
			pCameraAdjustWork->ssAdjustFlag &= ~0x40u;
			pCameraAdjustWork->angSpeed[1] = 0;
		}
		else
		{
			Angle limit = angy / 4;
			Angle ang;

			if (angy >= 0)
			{
				ang = pCameraAdjustWork->angSpeed[1] + 0x40;
				if (ang >= limit)
				{
					ang = limit;
				}

				if (ang >= 0x100)
				{
					ang = 0x100;
				}
			}
			else
			{
				ang = pCameraAdjustWork->angSpeed[1] - 0x40;
				if (ang <= limit)
				{
					ang = limit;
				}

				if (ang <= -0x100)
				{
					ang = -0x100;
				}
			}

			pCameraAdjustWork->angSpeed[1] = ang;
			camcont_wp->angy = ang + pOldTaskWork->ang.y;
		}
	}

	Angle angx = SubAngle(pOldTaskWork->ang.x, pTaskWork->ang.x);

	if (angx & 0x8000)
	{
		angx |= 0xFFFF0000;
	}

	if (angx > -0x80 && angx < 0x80)
	{
		camcont_wp->angx = pTaskWork->ang.x;
		pCameraAdjustWork->ssAdjustFlag &= ~0x20u;
		pCameraAdjustWork->angSpeed[0] = 0;
	}
	else
	{
		Angle limit = angx / 4;
		Angle ang;

		if (angx >= 0)
		{
			ang = pCameraAdjustWork->angSpeed[0] + 0x80;
			if (ang >= limit)
			{
				ang = limit;
			}

			if (ang >= 0x200)
			{
				ang = 0x200;
			}
		}
		else
		{
			ang = pCameraAdjustWork->angSpeed[0] - 0x80;
			if (ang <= limit)
			{
				ang = limit;
			}

			if (ang <= -0x200)
			{
				ang = -0x200;
			}
		}

		pCameraAdjustWork->angSpeed[0] = ang;
		camcont_wp->angx = ang + pOldTaskWork->ang.x;
	}

	camcont_wp->tgtxpos = ptwp->pos.x;
	camcont_wp->tgtypos = ptwp->pos.y + 15.0f;
	camcont_wp->tgtzpos = ptwp->pos.z;
	camcont_wp->tgtdist = dist;
	camcont_wp->angz = 0;
	CamcontSetCameraTGTOFST(pTaskWork);
}

void __cdecl AdjustForFreeCamera_m(taskwk* pTaskWork, taskwk* pOldTaskWork, _OBJ_ADJUSTPARAM* pCameraAdjustWork)
{
	if (!splitscreen::IsActive())
	{
		AdjustForFreeCamera(pTaskWork, pOldTaskWork, pCameraAdjustWork);
		return;
	}

	auto pnum = TASKWK_PLAYERID(playertwp[0]);
	auto ptwp = playertwp[pnum];
	auto ppwp = playerpwp[pnum];

	NJS_POINT3 dst;
	dst.x = pTaskWork->pos.x - ptwp->pos.x;
	dst.y = pTaskWork->pos.y - ptwp->pos.y;
	dst.z = pTaskWork->pos.z - ptwp->pos.z;
	Float dst_radius = njScalor(&dst);

	NJS_POINT3 src;
	GetPlayerPosition(pnum, 2, &src, 0);
	src.x = pOldTaskWork->pos.x - src.x;
	src.y = pOldTaskWork->pos.y - src.y;
	src.z = pOldTaskWork->pos.z - src.z;
	Float src_radius = njScalor(&src);

	Float dist = max(src_radius + (dst_radius - src_radius) * 0.3f, 20.0f);

	if (camcont_wp->ssFlag & 1) // LR Flag carried on from some auto cameras
	{
		camcont_wp->ssFlag &= ~1;
	}
	else
	{
		Angle angy = SubAngle(pOldTaskWork->ang.y, pTaskWork->ang.y);

		if (angy & 0x8000)
		{
			angy |= 0xFFFF0000;
		}

		if (angy > -0x200 && angy < 0x200)
		{
			camcont_wp->angy = pTaskWork->ang.y;
			pCameraAdjustWork->ssAdjustFlag &= ~0x40u;
			pCameraAdjustWork->angSpeed[1] = 0;
		}
		else
		{
			Angle spd = angy >> 2;

			if (angy >= 0)
			{
				Angle max = pCameraAdjustWork->angSpeed[1] + 0x200;
				if (spd >= max)
				{
					spd = max;
				}
			}
			else
			{
				Angle min = pCameraAdjustWork->angSpeed[1] - 0x200;
				if (spd <= min)
				{
					spd = min;
				}
			}

			pCameraAdjustWork->angSpeed[1] = spd;
			camcont_wp->angy = spd + pOldTaskWork->ang.y;
		}
	}

	Angle angx = SubAngle(pOldTaskWork->ang.x, pTaskWork->ang.x);

	if (angx & 0x8000)
	{
		angx |= 0xFFFF0000;
	}

	if (angx > -0x200 && angx < 0x200)
	{
		camcont_wp->angx = pTaskWork->ang.x;
		pCameraAdjustWork->ssAdjustFlag &= ~0x20u;
		pCameraAdjustWork->angSpeed[0] = 0;
	}
	else
	{
		Angle spd = angx >> 2;

		if (angx >= 0)
		{
			Angle max = pCameraAdjustWork->angSpeed[0] + 0x200;
			if (spd >= max)
			{
				spd = max;
			}
		}
		else
		{
			Angle min = pCameraAdjustWork->angSpeed[0] - 0x200;
			if (spd <= min)
			{
				spd = min;
			}
		}

		pCameraAdjustWork->angSpeed[0] = spd;
		camcont_wp->angx = spd + pOldTaskWork->ang.x;
	}

	camcont_wp->tgtxpos = ptwp->pos.x;
	camcont_wp->tgtypos = ppwp->p.center_height + ptwp->pos.y;
	camcont_wp->tgtzpos = ptwp->pos.z;
	camcont_wp->tgtdist = dist;
	camcont_wp->angz = 0;
	CamcontSetCameraLOOKAT(pTaskWork);
}

void atpInitParam_m(int pnum, taskwk* twp, taskwk* ptwp, _OBJ_ADJUSTPARAM* adjwp)
{
	struct atp_init_data {
		Bool collision;
		Float max_time;
		int inertia;
	};

	DataArray(atp_init_data, init_data, 0x7E0888, 20);

	angle_spd_m[pnum].x = DiffAngle(ptwp->ang.x, angle_c2p_m[pnum].x);
	angle_spd_m[pnum].y = DiffAngle(ptwp->ang.y, angle_c2p_m[pnum].y);

	inertia_m[pnum] = { 0.0f, 0.0f, 0.0f };

	update_inertia_m[pnum] = 1;

	param_collision_m[pnum] = init_data[GetAdjustMode_m(pnum) - 6].collision;
	param_max_time_m[pnum] = init_data[GetAdjustMode_m(pnum) - 6].max_time;
	param_inertia_m[pnum] = init_data[GetAdjustMode_m(pnum) - 6].inertia;
}

void atpCalcCamera2PlayerAngle_m(int pnum, taskwk* twp, taskwk* ptwp, _OBJ_ADJUSTPARAM* adjwp)
{
	Float x = ptwp->pos.x - playertwp[pnum]->pos.x;
	Float z = ptwp->pos.z - playertwp[pnum]->pos.z;
	angle_c2p_m[pnum].y = njArcTan2(x, z);
	angle_c2p_m[pnum].x = njArcTan2(playertwp[pnum]->pos.y - ptwp->pos.y, njSqrt(z * z + x * x));
}

void atpAdjustCameraPoint_m(int pnum, taskwk* twp, taskwk* ptwp, _OBJ_ADJUSTPARAM* adjwp)
{
	taskwk* pltwp = playertwp[pnum];
	Float power = 1.0f - adjust_point_m[pnum];

	Float x = (ptwp->pos.x - pltwp->pos.x) * power * 0.2f;
	Float y = (ptwp->pos.y - pltwp->pos.y) * power * 0.2f;
	Float z = (ptwp->pos.z - pltwp->pos.z) * power * 0.2f;

	x = (twp->pos.x - ptwp->pos.x + x) * adjust_point_m[pnum];
	y = (twp->pos.y - ptwp->pos.y + y) * adjust_point_m[pnum];
	z = (twp->pos.z - ptwp->pos.z + z) * adjust_point_m[pnum];

	if (adjwp->counter >= param_inertia_m[pnum])
	{
		twp->pos.x = x + ptwp->pos.x;
		twp->pos.y = y + ptwp->pos.y;
		twp->pos.z = z + ptwp->pos.z;
	}
	else
	{
		twp->pos.x = ((Float)(param_inertia_m[pnum] - adjwp->counter) * inertia_m[pnum].x + (Float)adjwp->counter * x) / (Float)param_inertia_m[pnum] + ptwp->pos.x;
		twp->pos.y = ((Float)(param_inertia_m[pnum] - adjwp->counter) * inertia_m[pnum].y + (Float)adjwp->counter * y) / (Float)param_inertia_m[pnum] + ptwp->pos.y;
		twp->pos.z = ((Float)(param_inertia_m[pnum] - adjwp->counter) * inertia_m[pnum].z + (Float)adjwp->counter * z) / (Float)param_inertia_m[pnum] + ptwp->pos.z;
	}
}

void atpAdjustCameraPointWithDistance_m(int pnum, taskwk* twp, taskwk* ptwp, _OBJ_ADJUSTPARAM* adjwp)
{
	NJS_POINT3 ppos;
	GetPlayerPosition(pnum, 1, &ppos, 0);
	Float x1 = ptwp->pos.x - ppos.x;
	Float y1 = ptwp->pos.y - ppos.y;
	Float z1 = ptwp->pos.z - ppos.z;
	Float x2 = twp->pos.x - playertwp[pnum]->pos.x;
	Float y2 = twp->pos.y - playertwp[pnum]->pos.y;
	Float z2 = twp->pos.z - playertwp[pnum]->pos.z;
	twp->pos.x = (x2 - x1) * adjust_point_m[pnum] + playertwp[pnum]->pos.x + x1;
	twp->pos.y = (y2 - y1) * adjust_point_m[pnum] + playertwp[pnum]->pos.y + y1;
	twp->pos.z = (z2 - z1) * adjust_point_m[pnum] + playertwp[pnum]->pos.z + z1;
}

void atpAdjustCameraTarget_m(int pnum, taskwk* twp, taskwk* ptwp, _OBJ_ADJUSTPARAM* adjwp)
{
	twp->ang.x = AdjustAngle(ptwp->ang.x, twp->ang.x, (Angle)((Float)DiffAngle(twp->ang.x, ptwp->ang.x) * adjust_point_m[pnum]));
	twp->ang.y = AdjustAngle(ptwp->ang.y, twp->ang.y, (Angle)((Float)DiffAngle(twp->ang.y, ptwp->ang.y) * adjust_point_m[pnum]));
	twp->ang.z = AdjustAngle(ptwp->ang.z, twp->ang.z, (Angle)((Float)DiffAngle(twp->ang.z, ptwp->ang.z) * adjust_point_m[pnum]));
}

void sub_468790_m(int pnum, taskwk* twp, taskwk* ptwp, _OBJ_ADJUSTPARAM* adjwp)
{
	if (DiffAngle(twp->ang.x, angle_c2p_m[pnum].x) > screen_in_ang_m[pnum].x)
	{
		twp->ang.x = AdjustAngle(angle_c2p_m[pnum].x, twp->ang.x, screen_in_ang_m[pnum].x);
	}

	if (DiffAngle(twp->ang.y, angle_c2p_m[pnum].y) > screen_in_ang_m[pnum].y)
	{
		twp->ang.y = AdjustAngle(angle_c2p_m[pnum].y, twp->ang.y, screen_in_ang_m[pnum].y);
	}
}

void __cdecl AdjustThreePoint_m(taskwk* pTaskWork, taskwk* pOldTaskWork, _OBJ_ADJUSTPARAM* pCameraAdjustWork)
{
	if (!splitscreen::IsActive())
	{
		AdjustThreePoint(pTaskWork, pOldTaskWork, pCameraAdjustWork);
		return;
	}

	int pnum = TASKWK_PLAYERID(playertwp[0]);

	atpCalcCamera2PlayerAngle_m(pnum, pTaskWork, pOldTaskWork, pCameraAdjustWork);

	if (pCameraAdjustWork->counter == 0)
	{
		atpInitParam_m(pnum, pTaskWork, pOldTaskWork, pCameraAdjustWork);
	}

	adjust_point_m[pnum] = (Float)pCameraAdjustWork->counter / param_max_time_m[pnum];

	if (adjust_point_m[pnum] >= 1.0f)
	{
		adjust_point_m[pnum] = 1.0f;

		if (GetCameraMode_m(pnum) == CAMMD_KOSCAM)
		{
			SetAdjustMode_m(pnum, CAMADJ_THREE3C);
		}
		else
		{
			SetAdjustMode_m(pnum, CAMADJ_NONE);
		}

		if (update_inertia_m[pnum])
		{
			inertia_m[pnum] = CameraInertia;
			update_inertia_m[pnum] = 0;
		}
		else
		{
			inertia_m[pnum] = { 0.0f, 0.0f, 0.0f };
			pCameraAdjustWork->counter = (int)param_max_time_m[pnum];
		}
	}

	screen_in_ang_m[pnum].x = (Angle)((Float)(0x1C71 - angle_spd_m[pnum].x) * adjust_point_m[pnum] + (Float)angle_spd_m[pnum].x);
	screen_in_ang_m[pnum].y = (Angle)((Float)(0xE38 - angle_spd_m[pnum].y) * adjust_point_m[pnum] + (Float)angle_spd_m[pnum].y);

	if (GetAdjustMode_m(pnum) > CAMADJ_THREE5C)
	{
		atpAdjustCameraPointWithDistance_m(pnum, pTaskWork, pOldTaskWork, pCameraAdjustWork); /* Only for RELATIVE adjusts */
	}
	else
	{
		atpAdjustCameraPoint_m(pnum, pTaskWork, pOldTaskWork, pCameraAdjustWork); /* Only for THREE adjusts*/
	}

	atpAdjustCameraTarget_m(pnum, pTaskWork, pOldTaskWork, pCameraAdjustWork);
	sub_468790_m(pnum, pTaskWork, pOldTaskWork, pCameraAdjustWork);

	if (param_collision_m[pnum])
	{
		CameraCollisitonCheckAdj(&pTaskWork->pos, &pOldTaskWork->pos);
	}
}

void __cdecl CameraLocalPath_m(_OBJ_CAMERAPARAM* pParam)
{
	auto pnum = TASKWK_PLAYERID(playertwp[0]);
	auto ptwp = playertwp[pnum];
	auto data = &cameraLocalPathData[pnum];

	if (!pParam->ulTimer)
	{
		data->point = 0;
		data->path_switch = 1;
	}

	++data->point;

	float onpos;
	Bool onpath = SCPathPntnmbToOnpos(data->ptp, data->point, &onpos);

	switch (data->mode)
	{
	case 0:
		if (!onpath)
		{
			if (data->path_switch == 1)
			{
				CameraReleaseEventCamera_m(pnum);
				data->path_switch = 0;
			}
			return;
		}
		break;
	case 1:
	case 2:
		if (!onpath)
		{
			--data->point;
			onpath = SCPathPntnmbToOnpos(data->ptp, data->point, &onpos);
		}
		break;
	case 3:
		if (pParam->ulTimer >= data->timer)
		{
			if (data->path_switch == 1)
			{
				CameraReleaseEventCamera_m(pnum);
				data->path_switch = 0;
			}
			return;
		}

		if (!onpath)
		{
			--data->point;
			onpath = SCPathPntnmbToOnpos(data->ptp, data->point, &onpos);
		}
		break;
	case 4:
		if (pParam->ulTimer >= data->timer)
		{
			if (data->path_switch == 1)
			{
				CameraReleaseEventCamera_m(pnum);
				data->path_switch = 0;
			}
			return;
		}

		if (!onpath)
		{
			--data->point;
			onpath = SCPathPntnmbToOnpos(data->ptp, data->point, &onpos);
		}
		break;
	default:
		if (data->path_switch == 1)
		{
			CameraReleaseEventCamera_m(pnum);
			data->path_switch = 0;
		}
		return;
	}

	if (onpath)
	{
		pathinfo pi;
		pi.onpathpos = onpos;
		GetStatusOnPath(data->ptp, &pi);

		NJS_VECTOR pos = { pi.xpos, pi.ypos, pi.zpos };
		njPushMatrix(nj_unit_matrix_);
		ROTATEY(0, 0, 0x8000 - ptwp->ang.y);
		njCalcVector(0, &pos, &pos);
		pi.xpos = pos.x;
		pi.ypos = pos.y;
		pi.zpos = pos.z;
		njPopMatrix(1);

		camcont_wp->camxpos = playermwp[pnum]->spd.x + ptwp->pos.x + pi.xpos;
		camcont_wp->camypos = playermwp[pnum]->spd.y + ptwp->pos.y + pi.ypos;
		camcont_wp->camzpos = playermwp[pnum]->spd.z + ptwp->pos.z + pi.zpos;

		NJS_VECTOR eye_vec = { 0.0f, playerpwp[pnum]->p.eyes_height, 0.0f };
		PConvertVector_P2G(ptwp, &eye_vec);
		camcont_wp->tgtxpos = playermwp[pnum]->spd.x + ptwp->pos.x + eye_vec.x;
		camcont_wp->tgtypos = playermwp[pnum]->spd.y + ptwp->pos.y + eye_vec.y;
		camcont_wp->tgtzpos = playermwp[pnum]->spd.z + ptwp->pos.z + eye_vec.z;
	}
}

void SetCameraChaosStdParam(NJS_POINT3* tgtpos_ofs, NJS_POINT3* campos_ofs)
{
	auto pnum = TASKWK_PLAYERID(playertwp[0]);

	if (!tgtpos_ofs)
		tgtpos_ofs = &chaoscam_tgtpos_ofs_def_m[pnum];
	if (!campos_ofs)
		campos_ofs = &chaoscam_campos_ofs_def_m[pnum];

	chaoscam_tgtpos_ofs_def_m[pnum] = *tgtpos_ofs;
	chaoscam_campos_ofs_def_m[pnum].x = campos_ofs->x;
	chaoscam_campos_ofs_def_m[pnum].y = campos_ofs->y;
	chaoscam_angacc_lim_def_m[pnum] = chaoscam_angacc_lim_def_m[pnum];
	chaoscam_angspd_lim_def_m[pnum] = chaoscam_angspd_lim_def_m[pnum];
	chaoscam_camspd_lim_def_m[pnum] = chaoscam_camspd_lim_def_m[pnum];
	chaoscam_campos_ofs_def_m[pnum].z = campos_ofs->z;
}

void SetCameraChaosStdSpeed(Angle3* angacc, Angle3* angspd, NJS_POINT3* camspd)
{
	auto pnum = TASKWK_PLAYERID(playertwp[0]);

	if (!angacc)
		angacc = &chaoscam_angacc_lim_def_m[pnum];
	if (!angspd)
		angspd = &chaoscam_angspd_lim_def_m[pnum];
	if (!camspd)
		camspd = &chaoscam_camspd_lim_def_m[pnum];
	chaoscam_angacc_lim_def_m[pnum] = *angacc;
	chaoscam_angspd_lim_def_m[pnum] = *angspd;
	chaoscam_camspd_lim_def_m[pnum].x = camspd->x;
	chaoscam_camspd_lim_def_m[pnum].y = camspd->y;
	chaoscam_camspd_lim_def_m[pnum].z = camspd->z;
}



void SetLocalPathCamera_m(pathtag* ptp, Sint32 mode, Sint32 timer, int pnum)
{
	if (pnum < 0 || pnum >= PLAYER_MAX)
	{
		return;
	}

	CameraSetEventCameraFunc_m(pnum, CameraLocalPath_m, CAMADJ_NONE, CDM_LOOKAT);

	auto data = &cameraLocalPathData[pnum];
	data->ptp = ptp;
	data->mode = mode;
	data->timer = timer;
	data->point = 0;
}


void PatchCameraFuncs()
{
	PathCamera1_h.Hook(PathCamera1_m);
	WriteJump(PathCamera2Core, PathCamera2Core_m);
	CameraMode[CAMMD_KLAMATH].fnCamera = CameraKlamath_m;
	CameraMode[CAMMD_A_KLAMATH].fnCamera = CameraKlamath_m;
	CameraMode[CAMMD_C_KLAMATH].fnCamera = CameraKlamath_m;
	CameraMode[CAMMD_CART].fnCamera = CameraCart_m;
	CameraMode[CAMMD_RuinWaka1].fnCamera = CameraRuinWaka1_m;
	//CameraMode[CAMMD_CHAOS_STD].fnCamera = CameraChaosStd_m; //todo

	CameraAdjust[CAMADJ_NORMAL].fnAdjust = AdjustNormal_m;
	CameraAdjust[CAMADJ_NORMAL_S].fnAdjust = AdjustNormal_m;
	CameraAdjust[CAMADJ_FORFREECAMERA].fnAdjust = AdjustForFreeCamera_m;


	for (int i = CAMADJ_THREE1; i <= CAMADJ_RELATIVE6C; ++i)
		CameraAdjust[i].fnAdjust = AdjustThreePoint_m;
}

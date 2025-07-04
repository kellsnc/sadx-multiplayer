#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "splitscreen.h"
#include "camera.h"
#include "camerafunc.h"

// Lost World camera paths

#define PATH_MODE_NOCOLLI 0x1
#define PATH_MODE_AUTOEND 0x2
#define PATH_MODE_AUTODIR 0x4
#define PATH_MODE_REVERSE 0x8
#define PATH_MODE_XZ      0x10

#define CAM_FLAG_REVERSE  0x2

#define PATH(twp) ((pathtag*)twp->value.ptr)
#define PATHWORK(twp) ((PATHCAMERAWORK*)twp->counter.ptr)
#define CAMWORK(twp) *(CamPathWk**)&twp->scl.x;

struct CamPathWk
{
	Sint32 frame;
	Sint32 frame_back;
	Sint32 frame_forward;
	NJS_VECTOR dir;
	NJS_VECTOR* pospath;
	Sint32 pathnum;
	Sint32 flag;
};

FastUsercallHookPtr<void(*)(task* tp), noret, rEDI> SetupCamPathCam2_Highway_h(0x613370);

static auto CheckRangeIn = GenerateUsercallWrapper<Bool(*)(NJS_POINT3*, NJS_POINT3*, NJS_POINT3*, Angle3*)>(rEAX, 0x6122C0, rEAX, rEDX, rECX, rESI);
static auto Camera_PathLinearScan = GenerateUsercallWrapper<Sint32(*)(taskwk*, NJS_POINT3*, NJS_POINT3*, NJS_POINT3*, Float*)>(rEAX, 0x612650, rEAX, rECX, rEDI, rESI, stack4);
static auto Camera_PathLinearScanXZ = GenerateUsercallWrapper<Sint32(*)(taskwk*, NJS_POINT3*, NJS_POINT3*, NJS_POINT3*, Float*)>(rEAX, 0x6126B0, rEAX, rECX, rEDI, rESI, stack4);

static VariableHook<Sint32, 0x3C81074> Paths_m;

static Bool CameraExec_m(taskwk* twp, taskwk* ptwp, taskwk* ctwp)
{
	auto pnum = twp->timer.w[1];
	auto pathwk = PATHWORK(twp);
	auto camwk = CAMWORK(twp);

	NJS_VECTOR* cam_pos = GetCameraPosition(pnum);
	Angle3* cam_ang = GetCameraAngle(pnum);

	if (!cam_pos || !cam_ang)
	{
		return FALSE;
	}

	Bool end = FALSE;

	NJS_POINT3 v1 = { 0.0f, pathwk->fSonicSize + 0.01f, 0.0f };
	NJS_POINT3 v2 = { 0.0f, 0.0f, 0.0f };
	NJS_POINT3 postgt;

	njPushMatrix(nj_unit_matrix_);
	njTranslateV(0, &ptwp->pos);
	ROTATEZ(0, 0, ptwp->ang.z);
	ROTATEX(0, 0, ptwp->ang.x);
	ROTATEY(0, 0, 0x8000 - ptwp->ang.y);
	njCalcPoint(0, &v1, &postgt);
	//njCalcVector(0, &v1, &dir1);
	//njCalcVector(0, &v2, &dir2);
	njPopMatrix(1);

	if (!CamPathCam2Core_AliveFlag_m[pnum])
	{
		Sint32 frame;
		Float dist;

		if (pathwk->modeflag & PATH_MODE_XZ)
		{
			frame = Camera_PathLinearScanXZ(twp, &camwk->dir, &postgt, &v1, &dist);
		}
		else
		{
			frame = Camera_PathLinearScan(twp, &camwk->dir, &postgt, &v1, &dist);
		}

		camwk->frame = frame;
		camwk->frame_back = frame;
		camwk->frame_forward = frame;

		if (pathwk->modeflag & PATH_MODE_AUTODIR)
		{
			Sint16 angdiff = -SHORT_ANG(cam_ang->y) - -njArcTan2(camwk->dir.x, camwk->dir.z);
			if (angdiff > 0x4000 || angdiff < -0x4000)
			{
				camwk->flag |= CAM_FLAG_REVERSE; // reverse
			}
			else
			{
				camwk->flag &= ~CAM_FLAG_REVERSE; // no reverse
			}
		}
		else if (pathwk->modeflag & PATH_MODE_REVERSE)
		{
			camwk->flag |= CAM_FLAG_REVERSE;
		}

		if (camwk->flag & CAM_FLAG_REVERSE)
		{
			camwk->dir.x = -camwk->dir.x;
			camwk->dir.y = -camwk->dir.y;
			camwk->dir.z = -camwk->dir.z;
		}
	}

	NJS_POINT3 posnear, vecnear;
	if (pathwk->modeflag & PATH_MODE_XZ)
	{
		camwk->frame = ScanNearPath2LXZ(camwk->frame, 100, &postgt, camwk->pospath, camwk->pathnum, NULL, &posnear, &vecnear);
	}
	else
	{
		camwk->frame = ScanNearPath2L(camwk->frame, 100, &postgt, camwk->pospath, camwk->pathnum, NULL, &posnear, &vecnear);
	}

	if (camwk->flag & CAM_FLAG_REVERSE)
	{
		njUnitVector(&vecnear);
		camwk->dir.x = -camwk->dir.x;
		camwk->dir.y = -camwk->dir.y;
		camwk->dir.z = -camwk->dir.z;
		camwk->dir.x = (camwk->dir.x - vecnear.x) * 0.1f + vecnear.x;
		camwk->dir.y = (camwk->dir.y - vecnear.y) * 0.1f + vecnear.y;
		camwk->dir.z = (camwk->dir.z - vecnear.z) * 0.1f + vecnear.z;
		njUnitVector(&camwk->dir);
		camwk->dir.x = -camwk->dir.x;
		camwk->dir.y = -camwk->dir.y;
		camwk->dir.z = -camwk->dir.z;
	}
	else
	{
		njUnitVector(&vecnear);
		camwk->dir.x = (camwk->dir.x - vecnear.x) * 0.1f + vecnear.x;
		camwk->dir.y = (camwk->dir.y - vecnear.y) * 0.1f + vecnear.y;
		camwk->dir.z = (camwk->dir.z - vecnear.z) * 0.1f + vecnear.z;
		njUnitVector(&camwk->dir);
	}

	if (pathwk->modeflag & PATH_MODE_AUTOEND)
	{
		if (camwk->frame >= camwk->pathnum - 2 || camwk->frame <= 0)
		{
			end = TRUE;
		}
	}

	Float dist; // from path line
	if (pathwk->modeflag & PATH_MODE_XZ)
	{
		dist = (postgt.z - posnear.z) * (postgt.z - posnear.z)
			+ (postgt.y - posnear.y) * (postgt.y - posnear.y)
			+ (postgt.x - posnear.x) * (postgt.x - posnear.x);
	}
	else
	{
		dist = (postgt.x - posnear.x) * (postgt.x - posnear.x)
			+ (postgt.y - posnear.y) * (postgt.y - posnear.y)
			+ (postgt.z - posnear.z) * (postgt.z - posnear.z);
	}

	if (njSqrt(dist) > pathwk->fPathCameraRangeOut)
	{
		end = TRUE;
	}

	NJS_VECTOR tgtback;
	tgtback.x = camwk->dir.x * pathwk->fBackPathDist + postgt.x;
	tgtback.y = camwk->dir.y * pathwk->fBackPathDist + postgt.y;
	tgtback.z = camwk->dir.z * pathwk->fBackPathDist + postgt.z;

	NJS_POINT3 posnear_back;
	if (pathwk->modeflag & PATH_MODE_XZ)
	{
		camwk->frame_back = ScanNearPath2LXZ(camwk->frame_back, 100, &tgtback, camwk->pospath, camwk->pathnum, NULL, &posnear_back, NULL);
	}
	else
	{
		camwk->frame_back = ScanNearPath2L(camwk->frame_back, 100, &tgtback, camwk->pospath, camwk->pathnum, NULL, &posnear_back, NULL);
	}

	NJS_VECTOR tgtforward;
	tgtforward.x = camwk->dir.x * -pathwk->fForwardPathDist + postgt.x;
	tgtforward.y = camwk->dir.y * -pathwk->fForwardPathDist + postgt.y;
	tgtforward.z = camwk->dir.z * -pathwk->fForwardPathDist + postgt.z;

	NJS_POINT3 posnear_forward;
	if (pathwk->modeflag & PATH_MODE_XZ)
	{
		camwk->frame_forward = ScanNearPath2LXZ(camwk->frame_forward, 100, &tgtforward, camwk->pospath, camwk->pathnum, NULL, &posnear_forward, NULL);
	}
	else
	{
		camwk->frame_forward = ScanNearPath2L(camwk->frame_forward, 100, &tgtforward, camwk->pospath, camwk->pathnum, NULL, &posnear_forward, NULL);
	}

	NJS_POINT3 pos;
	pos.x = pathwk->fBackPathMul * (postgt.x - posnear.x) + posnear_back.x;
	pos.y = pathwk->fBackPathMul * (postgt.y - posnear.y) + posnear_back.y;
	pos.z = pathwk->fBackPathMul * (postgt.z - posnear.z) + posnear_back.z;

	NJS_POINT3 pos_forward;
	pos_forward.x = pathwk->fForwardPathMul * (postgt.x - posnear.x) + posnear_forward.x;
	pos_forward.y = pathwk->fForwardPathMul * (postgt.y - posnear.y) + posnear_forward.y;
	pos_forward.z = pathwk->fForwardPathMul * (postgt.z - posnear.z) + posnear_forward.z;

	if (!(pathwk->modeflag & PATH_MODE_NOCOLLI))
	{
		NJS_POINT3 col_pos = *cam_pos;
		Float dist1 = njSqrt((posnear_back.x - col_pos.x) * (posnear_back.x - col_pos.x)
			+ (posnear_back.y - col_pos.y) * (posnear_back.y - col_pos.y)
			+ (posnear_back.z - col_pos.z) * (posnear_back.z - col_pos.z));
		Float dist2 = njSqrt((posnear_back.x - pos.x) * (posnear_back.x - pos.x)
			+ (posnear_back.y - pos.y) * (posnear_back.y - pos.y)
			+ (posnear_back.z - pos.z) * (posnear_back.z - pos.z));
		if (dist2 >= dist1)
		{
			NJS_POINT3 col_dir;
			col_dir.x = pos.x - col_pos.x;
			col_dir.y = pos.y - col_pos.y;
			col_dir.z = pos.z - col_pos.z;
			if (MSetPosition(&col_pos, &col_dir, 0, pathwk->fSonicSize))
			{
				pos.x = v1.x + cam_pos->x;
				pos.y = v1.y + cam_pos->y;
				pos.z = v1.z + cam_pos->z;
			}
		}
	}

	pos.x = (pos.x - cam_pos->x) * pathwk->fCamMovMul + cam_pos->x;
	pos.y = (pos.y - cam_pos->y) * pathwk->fCamMovMul + cam_pos->y;
	pos.z = (pos.z - cam_pos->z) * pathwk->fCamMovMul + cam_pos->z;

	Float diffx = pos.x - pos_forward.x;
	Float diffz = pos.z - pos_forward.z;

	Angle angy = -njArcTan2(pos.x - pos_forward.x, pos.z - pos_forward.z);
	Angle angspdy = -cam_ang->y - angy;
	((Sint16*)&angspdy)[1] = ((-cam_ang->y - angy) >> 16) * pathwk->fCamMovMul;

	Angle angx = -njArcTan2(pos_forward.y - pos.y, njSqrt(diffx * diffx + diffz * diffz));
	Angle angspdx = -cam_ang->x - angx;
	((Sint16*)&angspdx)[1] = ((-cam_ang->x - angx) >> 16) * pathwk->fCamMovMul;

	Angle3 ang;
	ang.y = cam_ang->y + angspdy;
	ang.x = cam_ang->x + angspdx;
	ang.z = 0;

	CamPathCam2Core_Pos_m[pnum] = pos;
	CamPathCam2Core_Angle_m[pnum] = ang;

	if (twp->wtimer >= pathwk->nChangeFrame)
	{
		SetAdjustMode_m(pnum, pathwk->ucAdjustTypeAfter);
	}

	CamPathCam2Core_AliveFlag_m[pnum] = FALSE;

	return end == FALSE;
}

static Bool CameraInit_m(taskwk* twp, taskwk* ptwp)
{
	auto pnum = twp->timer.w[1];
	auto pathwk = PATHWORK(twp);
	auto camwk = CAMWORK(twp);

	NJS_VECTOR* cam_pos = GetCameraPosition(pnum);
	Angle3* cam_ang = GetCameraAngle(pnum);

	if (!cam_pos || !cam_ang)
	{
		return FALSE;
	}

	Sint32 frame;
	Float dist;
	NJS_POINT3 posr;
	NJS_POINT3 vecr;

	NJS_POINT3 postgt = ptwp->pos;
	if (pathwk->modeflag & PATH_MODE_XZ)
	{
		frame = Camera_PathLinearScanXZ(twp, &vecr, &postgt, &posr, &dist);
	}
	else
	{
		frame = Camera_PathLinearScan(twp, &vecr, &postgt, &posr, &dist);
	}

	camwk->frame = frame;
	camwk->frame_back = frame;
	camwk->frame_forward = frame;
	camwk->dir = vecr;

	if (camwk->frame >= camwk->pathnum - 2 || camwk->frame <= 0)
	{
		return FALSE;
	}

	if (pathwk->fPathCameraRangeIn <= dist)
	{
		return FALSE;
	}

	CameraSetEventCameraFunc_m(pnum, PathCamera2Core, pathwk->ucAdjustType, 0);
	CamPathCam2Core_AliveFlag_m[pnum] = FALSE;
	CamPathCam2Core_Pos_m[pnum] = *cam_pos;
	CamPathCam2Core_Angle_m[pnum] = *cam_ang;

	if (pathwk->modeflag & PATH_MODE_AUTODIR)
	{
		Sint16 angdiff = -SHORT_ANG(cam_ang->y) - -njArcTan2(camwk->dir.x, camwk->dir.z);
		if (angdiff > 0x4000 || angdiff < -0x4000)
		{
			camwk->flag |= CAM_FLAG_REVERSE;
		}
		else
		{
			camwk->flag &= ~CAM_FLAG_REVERSE;
		}
	}
	else if (pathwk->modeflag & PATH_MODE_REVERSE)
	{
		camwk->flag |= CAM_FLAG_REVERSE;
	}

	if (camwk->flag & CAM_FLAG_REVERSE)
	{
		camwk->dir.x = -camwk->dir.x;
		camwk->dir.y = -camwk->dir.y;
		camwk->dir.z = -camwk->dir.z;
	}

	return TRUE;
}

static void ObjectCamPathCam_Exec_m(task* tp)
{
	taskwk* twp = tp->twp;

	auto pnum = twp->timer.w[1];
	taskwk* ptwp = playertwp[pnum];

	if (GetStageNumber() != twp->timer.w[0])
	{
		FreeTask(tp);
		return;
	}

	if (loop_count > 0)
	{
		return;
	}

	if (!ptwp || !splitscreen::IsScreenEnabled(pnum))
	{
		return;
	}

	if (twp->btimer & 1)
	{
		if (IsCompulsionCamera_m(pnum) || !IsEventCamera_m(pnum) || !CameraExec_m(twp, ptwp, nullptr))
		{
			if (!IsCompulsionCamera_m(pnum) && IsEventCamera_m(pnum))
				CameraReleaseEventCamera_m(pnum);
			CamPathCam2Core_AliveFlag_m[pnum] = 0;
			twp->wtimer = 0;
			twp->btimer &= ~1;
		}
	}
	else
	{
		if (!IsCompulsionCamera_m(pnum) && !IsEventCamera_m(pnum) && twp->wtimer > 20)
		{
			auto pathwk = PATHWORK(twp);
			auto camwk = CAMWORK(twp);

			NJS_POINT3 rangein;
			rangein.x = pathwk->fPathCameraRangeIn;
			rangein.y = camwk->frame;
			rangein.z = camwk->frame;

			if ((camwk->pospath || Paths_m[pnum] <= 2) && CheckRangeIn(&rangein, &twp->pos, &ptwp->pos, &twp->ang))
			{
				if (!camwk->pospath)
				{
					int nbPoint = PATH(twp)->points - pathwk->nBottomPathGap - pathwk->nTopPathGap;
					camwk->pospath = (NJS_POINT3*)MAlloc(sizeof(NJS_POINT3) * 5 * (nbPoint + 3));

					NJS_POINT3* points = (NJS_POINT3*)MAlloc(sizeof(NJS_POINT3) * nbPoint);

					for (int i = 0; i < nbPoint; ++i)
					{
						points[i] = *(NJS_POINT3*)&PATH(twp)->tblhead[pathwk->nBottomPathGap + i].xpos;
					}

					SPLINE_DATA spline_data;
					spline_data.src = points;
					spline_data.data_size = nbPoint;
					spline_data.fmt = (char*)0x7EC82C;
					spline_data.rate = 0.2f; // 1/5
					spline_data.dst = camwk->pospath;
					spline_data.spline_type = 1;
					spline_data.variety = 0;
					camwk->pathnum = CalcSpline(&spline_data) - 18.0; // weird
					camwk->pospath += 6;

					Free(points);
					++Paths_m[pnum];
				}
				else
				{
					if (CameraInit_m(twp, ptwp))
					{
						twp->wtimer = 0;
						twp->btimer |= 1;
					}
				}
			}
			else
			{
				if (camwk->pospath)
				{
					--Paths_m[pnum];
					Free(camwk->pospath - 6);
					camwk->pospath = 0;
					camwk->pathnum = 0;
				}
			}
		}
	}

	if (++twp->wtimer == 0)
	{
		twp->wtimer = -1;
	}
}

static void ObjectCamPathCam_End_m(task* tp)
{
	taskwk* twp = tp->twp;

	auto pnum = twp->timer.w[1];
	auto camwk = CAMWORK(twp);

	if (camwk->pospath)
	{
		--Paths_m[pnum];
		Free(camwk->pospath - 6);
		camwk->pospath = 0;
		camwk->pathnum = 0;
	}

	Free(camwk);
}

static void SetupCamPathCam2_Highway_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		SetupCamPathCam2_Highway_h.Original(tp);

		// Create one task per player, it's a bit memory inefficient but easier to do
		for (int i = 1; i < PLAYER_MAX; ++i)
		{
			task* tp2 = CreateElementalTask(LoadObj_Data1, LEV_2, NULL);
			tp2->twp->value.ptr = tp->twp->value.ptr;
			tp2->twp->timer.w[0] = tp->twp->timer.w[0];
			tp2->twp->timer.w[1] = i;
			tp2->twp->counter.ptr = tp->twp->counter.ptr;
			SetupCamPathCam2_Highway_h.Original(tp2);
			tp2->exec = ObjectCamPathCam_Exec_m;
			tp2->dest = ObjectCamPathCam_End_m;
		}
	}
	else
	{
		SetupCamPathCam2_Highway_h.Original(tp);
	}
}

void patch_hw_pathcam_init()
{
	SetupCamPathCam2_Highway_h.Hook(SetupCamPathCam2_Highway_r);
}

RegisterPatch patch_hw_pathcam(patch_hw_pathcam_init);
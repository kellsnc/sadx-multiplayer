#include "pch.h"
#include "Trampoline.h"
#include "sadx_utils.h"
#include "utils.h"
#include "multiplayer.h"
#include "camera.h"

struct AL_CAMERA_WORK {
	Sint8 mode;
	Sint8 smode;
	Sint8 id;
	Sint8 btimer;
	Sint16 flag;
	Uint16 wtimer;
	union {
		Uint8 b[4];
		Uint16 w[2];
		Uint32 l;
		Float f;
		Void* ptr;
	} counter;
	union {
		Sint8 b[4];
		Sint16 w[2];
		Sint32 l;
		Float f;
		Void* ptr;
	} timer, value;
	Angle3 ang;
	NJS_POINT3 pos;
	NJS_POINT3 scl;
	colliwk* cwp;
	Void* ewp;
	Uint16 type;
	NJS_POINT3 velo;
	NJS_POINT3 AimPos;
	Angle3 AngSpd;
	NJS_POINT3 target;
	NJS_POINT3 AimTarget;
	Float dist;
	Angle ViewAngle;
	Float TargetNormalOffsetY;
	Float TargetTreeOffsetY;
	Float PosOffsetY;
	Float PosItpRatio;
	Bool StopFlag;
};

VariableHook<task*, 0x3CDC684> pCamTask_m;
DataArray(AL_CAMERA_WORK, AL_CameraWork, 0x3CDC1A0, 8);

Bool AL_NormalCameraCheckSwingingTree_m(Sint32 pnum)
{
	switch (GetPlayerCharacterName(pnum))
	{
	case PLNO_SONIC:
		return IsSonicShakingTree(playertwp[pnum]);
	case PLNO_TAILS:
		return IsSonicShakingTree(playertwp[pnum]);
	case PLNO_KNUCKLES:
		return IsKnucklesShakingTree(playertwp[pnum]);
	case PLNO_AMY:
		return IsAmyShakingTree(playertwp[pnum]);
	case PLNO_BIG:
		return IsE102ShakingTree(playertwp[pnum]);
	case PLNO_E102:
		return IsE102ShakingTree(playertwp[pnum]);
	}
	return FALSE;
}

void AL_NormalCameraExecutor(task* tp)
{
	AL_CAMERA_WORK* camwk = (AL_CAMERA_WORK*)tp->twp;
	auto pnum = camwk->btimer;

	if (!playertwp[pnum])
	{
		return;
	}

	NJS_POINT3* pPlayerPos = &playertwp[pnum]->pos;

	if (!camwk->mode)
	{
		camwk->pos.x = pPlayerPos->x - njSin(0x4000 - playertwp[pnum]->ang.y) * 10.0f;
		camwk->pos.y = camwk->PosOffsetY + pPlayerPos->y;
		camwk->pos.z = pPlayerPos->z - njCos(0x4000 - playertwp[pnum]->ang.y) * 10.0f;
		camwk->AimPos.x = camwk->pos.x;
		camwk->AimPos.y = camwk->pos.y;
		camwk->AimPos.z = camwk->pos.z;
		++camwk->mode;
	}

	NJS_VECTOR tgt;
	tgt.x = camwk->AimPos.x - pPlayerPos->x;
	tgt.y = 0.0f;
	tgt.z = camwk->AimPos.z - pPlayerPos->z;
	njUnitVector(&tgt);
	tgt.x = CamDist * tgt.x;
	tgt.z = CamDist * tgt.z;

	if (perG[pnum].on & Buttons_L)
	{
		njPushMatrix(_nj_unit_matrix_);
		njRotateY(0, -0x200);
		njCalcVector(0, &tgt, &tgt);
		njPopMatrixEx();
	}
	if (perG[pnum].on & Buttons_R)
	{
		njPushMatrix(_nj_unit_matrix_);
		njRotateY(0, 0x200);
		njCalcVector(0, &tgt, &tgt);
		njPopMatrixEx();
	}

	if (AL_NormalCameraCheckSwingingTree_m(pnum))
	{
		NJS_VECTOR* tree_pos;
		task* tree_tp = playerpwp[pnum]->htp;
		if (tree_tp)
		{
			tree_pos = &tree_tp->twp->pos;
		}
		else
		{
			tree_pos = pPlayerPos;
		}
		camwk->target.x = (tree_pos->x - camwk->target.x) * 0.04f + camwk->target.x;
		camwk->target.y = (tree_pos->y + camwk->TargetTreeOffsetY - camwk->target.y) * 0.04f + camwk->target.y;
		camwk->target.z = (tree_pos->z - camwk->target.z) * 0.04f + camwk->target.z;
	}

	//if (IsLookingAtHomepage)
	//{
	//	camwk->target.x += (70.0f - camwk->target.x) * 0.04f;
	//	camwk->target.y += (95.0f - camwk->target.y) * 0.04f;
	//	camwk->target.z += (-150.0f - camwk->target.z) * 0.04f;
	//}

	camwk->target.x += (pPlayerPos->x - camwk->target.x) * 0.4f;
	camwk->target.y += (camwk->TargetNormalOffsetY + pPlayerPos->y - camwk->target.y) * 0.2f;
	camwk->target.z += (pPlayerPos->z - camwk->target.z) * 0.4f;

	if (0 /* OdekakeMenuFlag && pOdekakeMachineTask */)
	{
		//...
	}
	else
	{
		camwk->AimPos.x = tgt.x + pPlayerPos->x;
		camwk->AimPos.y = camwk->PosOffsetY + pPlayerPos->y;
		camwk->AimPos.z = tgt.z + pPlayerPos->z;
	}

	MOVE_WORK* mwp = (MOVE_WORK*)tp->mwp;

	Float waterY = mwp->WaterY + ChaoCameraWaterOffsetY;
	if (camwk->AimPos.y < waterY)
		camwk->AimPos.y = waterY;

	if (mwp->Flag & MOVE_FLAG_WALL)
	{
		camwk->PosItpRatio = 0.02f;
	}
	//else if (OdekakeMenuFlag)
	//{
	//	camwk->PosItpRatio = 0.05f;
	//}
	else
	{
		camwk->PosItpRatio += 0.01f;
		if (camwk->PosItpRatio > CamPosItpRatio)
		{
			camwk->PosItpRatio = CamPosItpRatio;
		}
	}

	NJS_POINT3 pos;
	pos.x = (camwk->AimPos.x - camwk->pos.x) * camwk->PosItpRatio;
	if (mwp->Flag & MOVE_FLAG_FLOOR && camwk->AimPos.y - camwk->pos.y < 30.0f)
		pos.y = (camwk->AimPos.y - camwk->pos.y) * CamPosItpRatioY * 0.1f;
	else
		pos.y = (camwk->AimPos.y - camwk->pos.y) * CamPosItpRatioY;
	pos.z = (camwk->AimPos.z - camwk->pos.z) * camwk->PosItpRatio;

	Float spd = njScalor(&pos);
	if (spd > 0.0f && spd > CamPosMaxSpd)
	{
		spd = CamPosMaxSpd;
		njUnitVector(&pos);
		pos.x *= CamPosMaxSpd;
		pos.y *= CamPosMaxSpd;
		pos.z *= CamPosMaxSpd;
	}

	njAddVector(&camwk->pos, &pos);

	Float x = camwk->pos.x - pPlayerPos->x;
	Float z = camwk->pos.z - pPlayerPos->z;
	Float dist2 = x * x + z * z;
	if (dist2 < 15.0f * 15.0f)
	{
		Float ratio = 15.0f / njSqrt(dist2);
		camwk->pos.x = x * ratio + pPlayerPos->x;
		camwk->pos.z = z * ratio + pPlayerPos->z;
	}

	if (camwk->StopFlag)
	{
		if (spd > 0.01f || (mwp->Flag & MOVE_FLAG_WALL))
		{
			camwk->StopFlag = FALSE;
		}
	}
	else
	{
		if (spd < 0.005f && !(mwp->Flag & MOVE_FLAG_WALL))
		{
			camwk->StopFlag = TRUE;
		}
		AL_DetectCollisionStandard(tp);
	}

	EntryColliList(tp->twp);
}

void AL_NormalCameraDestructor(task* tp)
{
	pCamTask_m[tp->twp->btimer] = NULL;
	if (tp->mwp)
	{
		FreeMemory(tp->mwp);
		tp->mwp = NULL;
	}
}

task* AL_CreateNormalCameraTask_m(int pnum)
{
	if (pCamTask_m[pnum])
		return NULL;
	task* tp = CreateElementalTask(0, LEV_2, AL_NormalCameraExecutor);
	tp->dest = AL_NormalCameraDestructor;

	AL_CAMERA_WORK* camwk = (AL_CAMERA_WORK*)CAlloc(1, sizeof(AL_CAMERA_WORK));
	tp->twp = (taskwk*)camwk;

	camwk->btimer = pnum;

	CCL_Init(tp, (CCL_INFO*)0x8857A8, 2, CID_OBJECT2);

	switch (GetPlayerNumber())
	{
	case PLNO_SONIC:
	case PLNO_TAILS:
	case PLNO_KNUCKLES:
	case PLNO_AMY:
		camwk->TargetNormalOffsetY = 4.5f;
		camwk->TargetTreeOffsetY = 82.0f;
		camwk->PosOffsetY = 5.0f;
		break;
	case PLNO_E102:
	case PLNO_BIG:
		camwk->TargetNormalOffsetY = 10.0f;
		camwk->TargetTreeOffsetY = 84.0f;
		camwk->PosOffsetY = 11.5f;
		break;
	default:
		break;
	}

	camwk->PosItpRatio = CamPosItpRatio;

	MOVE_WORK* mwp = MOV_Init(tp);
	mwp->Side = 5.0f;
	mwp->Offset.y = 1.0f;
	mwp->Bottom = -3.0f;
	MOV_SetGravity(tp, 0.0f);

	pCamTask_m[pnum] = tp;

	return tp;
}

void AL_CreateNormalCameraTask_r();
Trampoline AL_CreateNormalCameraTask_t(0x0072A570, 0x0072A575, AL_CreateNormalCameraTask_r);
void AL_CreateNormalCameraTask_r()
{
	TARGET_STATIC(AL_CreateNormalCameraTask)();

	if (multiplayer::IsActive())
	{
		for (int i = 1; i < PLAYER_MAX; ++i)
		{
			AL_CreateNormalCameraTask_m(i);
		}
	}
}

void ALCAM_ModeSystem_m(_OBJ_CAMERAPARAM* pParam)
{
	int pnum = TASKWK_PLAYERID(playertwp[0]);
	AL_CAMERA_WORK* camwk;

	if (pCamTask_m[pnum])
		camwk = (AL_CAMERA_WORK*)pCamTask_m[pnum]->twp;
	else
		camwk = &AL_CameraWork[pnum];
	camcont_wp->camxpos = camwk->pos.x;
	camcont_wp->camypos = camwk->pos.y;
	camcont_wp->camzpos = camwk->pos.z;
	camcont_wp->tgtxpos = camwk->target.x;
	camcont_wp->tgtypos = camwk->target.y;
	camcont_wp->tgtzpos = camwk->target.z;
	camcont_wp->angx = camwk->ang.x;
	camcont_wp->angy = camwk->ang.y;
	camcont_wp->angz = camwk->ang.z;
	camcont_wp->tgtdist = camwk->dist;
}

void ALCAM_CreateCameraManager_r();
Trampoline ALCAM_CreateCameraManager_t(0x0072A750, 0x0072A755, ALCAM_CreateCameraManager_r);
void ALCAM_CreateCameraManager_r()
{
	TARGET_STATIC(ALCAM_CreateCameraManager)();
	if (multiplayer::IsActive())
	{
		for (int i = 1; i < PLAYER_MAX; ++i)
		{
			task* tp = CreateElementalTask(IM_TASKWK, LEV_2, (TaskFuncPtr)0x72A670);
			CameraSetCollisionCameraFunc_m(i, ALCAM_ModeSystem_m, CAMADJ_NORMAL, CDM_LOOKAT);
		}
	}
}
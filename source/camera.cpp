#include "pch.h"
#include "splitscreen.h"
#include "camerafunc.h"
#include "camera.h"

#define MODE_ENABLED      0x1          /* If free camera is enabled */
#define MODE_ACTIVE       0x2          /* If free camera logic has run */
#define MODE_AUTHORIZED   0x4          /* If free camera is currently authorized */
#define MODE_TIMER        0x8          /* If free camera is temporarily disabled */
#define MODE_FIX          0x40000000   /* Reposition the camera properly */
#define MODE_UPDATE       0x80000000   /* Reset the camera position */

struct CameraLocation
{
    NJS_POINT3 pos;
    Angle3 ang;
};

Trampoline* CameraPause_t = nullptr;
Trampoline* cameraDisplay_t = nullptr;
Trampoline* Camera_t = nullptr;

VariableHook<_camcontwk, 0x3B2C660> cameraControlWork_m;
VariableHook<_CameraSystemWork, 0x3B2CAD8> cameraSystemWork_m;
VariableHook<NJS_POINT3, 0x3B2CAB0> __PlayerStatus_last_pos_m;
VariableHook<NJS_POINT3, 0x3B2CA8C> __CameraInertia_last_pos_m;
VariableHook<int32_t, 0x3B2CAD0> cameraTimer_m;
VariableHook<int32_t, 0x3B2CA18> eventReleaseTimer_m;
VariableHook<int32_t, 0x3B2C6C4> camera_mode_m;
VariableHook<uint32_t, 0x3B2CBA8> free_camera_mode_m;
VariableHook<_OBJ_ADJUSTPARAM, 0x3B2CA20> objAdjustParam_m;
VariableHook<FCWRK, 0x3B2C958> fcwrk_m;

CameraLocation cameraLocations[PLAYER_MAX];

NJS_POINT3* GetCameraPosition(int pnum)
{
    return &cameraLocations[pnum].pos;
}

Angle3* GetCameraAngle(int pnum)
{
    return &cameraLocations[pnum].ang;
}

void SetFreeCamera_m(int pnum, Sint32 sw)
{
    auto& fcmode = free_camera_mode_m[pnum];
    if (sw)
    {
        if (!(fcmode & MODE_ENABLED))
        {
            fcmode |= MODE_ENABLED | MODE_UPDATE;
        }
    }
    else
    {
        fcmode = fcmode & ~MODE_ENABLED | MODE_UPDATE;
    }
}

Bool GetFreeCamera_m(int pnum)
{
    return !!(free_camera_mode_m[pnum] & MODE_ENABLED);
}

void SetAdjustMode_m(int pnum, Sint32 AdjustType)
{
    cameraSystemWork_m[pnum].G_scCameraAdjust = AdjustType;
    cameraSystemWork_m[pnum].G_pfnAdjust = pObjCameraAdjust[AdjustType].fnAdjust;
    objAdjustParam_m[pnum].counter = 0;
}

Sint32 GetAdjustMode_m(int pnum)
{
    return cameraSystemWork_m[pnum].G_scCameraAdjust;
}

Sint32 GetCameraMode_m(int pnum)
{
    return cameraSystemWork_m[pnum].G_scCameraMode;
}

void SetCameraMode_m(int pnum, Sint32 mode)
{
    cameraSystemWork_m[pnum].G_ssCameraEntry = -1;
    cameraSystemWork_m[pnum].G_scCameraMode = mode;
    cameraSystemWork_m[pnum].G_pfnCamera = pObjCameraMode[mode].fnCamera;
}

void CameraSetCamera_m(int pnum, Sint8 ssCameraMode, Sint16 ssCameraEntry)
{
    auto& system = cameraSystemWork_m[pnum];

    _OBJ_CAMERAMODE* cameraMode = &pObjCameraMode[ssCameraMode];
    _OBJ_CAMERAENTRY* cameraEntry = &pObjCameraEntry[ssCameraEntry];
    _OBJ_CAMERAENTRY* cameraEntryOld = &pObjCameraEntry[system.G_ssCameraEntry];
    Sint8 cameraLevel = cameraMode->scCameraLevel;

    system.G_ssRestoreLevel[cameraLevel] = system.G_scCameraLevel;
    system.G_ssRestoreEntry[cameraLevel] = system.G_ssCameraEntry;
    system.G_scRestoreAttribute[cameraLevel] = system.G_scCameraAttribute;
    system.G_scRestoreCameraMode[cameraLevel] = system.G_scCameraMode;
    system.G_pfnRestoreCamera[cameraLevel] = system.G_pfnCamera;

    if (system.G_ssCameraEntry == -1)
    {
        system.G_scRestoreCameraAdjust[cameraLevel] = 21;
    }
    else
    {
        system.G_scRestoreCameraAdjust[cameraLevel] = pObjCameraEntry[system.G_ssCameraEntry].ucAdjType;
    }

    system.G_pfnRestoreAdjust[cameraLevel] = pObjCameraAdjust[system.G_scRestoreCameraAdjust[cameraLevel]].fnAdjust;
    system.G_scCameraLevel = cameraLevel;
    system.G_ssCameraEntry = ssCameraEntry;
    system.G_scCameraMode = ssCameraMode;
    system.G_pfnCamera = cameraMode->fnCamera;

    if (cameraEntryOld->scPriority < cameraEntry->scPriority)
    {
        //dword_3B2CAC0 = 1;
    }

    Sint8 adjustType = (cameraEntryOld->scPriority < 10 || cameraEntry->scPriority < 10) ? cameraEntry->ucAdjType : 0;
    system.G_scCameraAdjust = adjustType;
    _OBJ_CAMERAADJUST* objCameraAdjust = &pObjCameraAdjust[adjustType];
    system.G_pfnAdjust = objCameraAdjust->fnAdjust;
    objAdjustParam_m[pnum].counter = 0;

    if (objCameraAdjust->slAttribute)
    {
        system.G_scCameraAttribute |= 1u;
    }
    else
    {
        system.G_scCameraAttribute = 0;
    }

    system.G_scCameraDirect = cameraMode->scCameraDirectMode;
    system.G_boolSwitched = 1;
}

void CameraCancelCamera_m(int pnum)
{
    auto& system = cameraSystemWork_m[pnum];

    Sint8 cameraLevel = system.G_scCameraLevel;
    system.G_ssCameraEntry = system.G_ssRestoreEntry[cameraLevel];
    system.G_scCameraAttribute = system.G_scRestoreAttribute[cameraLevel];
    system.G_scCameraMode = system.G_scRestoreCameraMode[cameraLevel];
    system.G_pfnCamera = system.G_pfnRestoreCamera[cameraLevel];
    system.G_scCameraLevel = system.G_ssRestoreLevel[cameraLevel];

    Sint8 priority = pObjCameraEntry[system.G_ssCameraEntry].scPriority;
    Sint8 newPriority = pObjCameraEntry[system.G_ssRestoreEntry[cameraLevel]].scPriority;

    if (priority < 10 || newPriority < 10)
    {
        system.G_scCameraAdjust = system.G_scRestoreCameraAdjust[cameraLevel];
    }
    else
    {
        system.G_scCameraAdjust = 0;
    }

    if (priority < newPriority)
    {
        //dword_3B2CAC0 = 1;
    }

    if (priority < 10 || newPriority < 10)
    {
        system.G_pfnAdjust = system.G_pfnRestoreAdjust[cameraLevel];
    }
    else
    {
        system.G_pfnAdjust = pObjCameraAdjust->fnAdjust;
    }

    system.G_scCameraDirect = pObjCameraMode[system.G_scCameraMode].scCameraDirectMode;
    system.G_boolSwitched = 1;
    system.G_ulTimer = 0;
    objAdjustParam_m[pnum].counter = 0;
}

void CameraReleaseEventCamera_m(int pnum)
{
    auto& system = cameraSystemWork_m[pnum];

    if (system.G_scCameraLevel == CLEVEL_EVENT)
    {
        system.G_ssCameraEntry = system.G_ssRestoreEntry[CLEVEL_EVENT];
        system.G_scCameraAttribute = system.G_scRestoreAttribute[CLEVEL_EVENT];
        system.G_scCameraMode = system.G_scRestoreCameraMode[CLEVEL_EVENT];
        system.G_pfnCamera = system.G_pfnRestoreCamera[CLEVEL_EVENT];
        system.G_scCameraAdjust = system.G_scRestoreCameraAdjust[CLEVEL_EVENT];
        system.G_pfnAdjust = system.G_pfnRestoreAdjust[CLEVEL_EVENT];
        system.G_scCameraLevel = system.G_ssRestoreLevel[CLEVEL_EVENT];
        system.G_scCameraDirect = pObjCameraMode[system.G_scRestoreCameraMode[CLEVEL_EVENT]].scCameraDirectMode;
        system.G_boolSwitched = 1;
        system.G_ulTimer = 0;
        objAdjustParam_m[pnum].counter = 0;

        //if (!(system.G_ssRestoreLevel[CLEVEL_EVENT] >> 8) && default_camera_mode != -1)
        //{
        //    CameraSetNormalCamera(default_camera_mode, default_camera_adjust);
        //}
    }

    eventReleaseTimer_m[pnum] = 0;
}

void ApplyMultiCamera(int pnum)
{
    if (!camera_twp || !playertwp[pnum] || pnum >= PLAYER_MAX)
    {
        return;
    }

    camera_twp->pos = cameraLocations[pnum].pos;
    camera_twp->ang = cameraLocations[pnum].ang;

    CameraSetView(camera_twp);
}

void __cdecl cameraDisplay_r(task* tp)
{
    if (SplitScreen::IsActive())
    {
        ApplyMultiCamera(SplitScreen::numScreen);
        //CameraFilter(tp); <- Works individually but crashes when drawn more than once with DC Conv
    }
    else
    {
        TARGET_DYNAMIC(cameraDisplay)(tp);
    }
}

void __cdecl CameraPause_r(task* tp)
{
    if (SplitScreen::IsActive())
    {
        ApplyMultiCamera(SplitScreen::numScreen);
    }
    else
    {
        TARGET_DYNAMIC(CameraPause)(tp);
    }
}

void CamcontSetCameraCAMSTATUS_m(int pnum)
{
    auto& ctrl = cameraControlWork_m[pnum];
    auto& loc = cameraLocations[pnum];

    loc.pos.x = ctrl.camxpos;
    loc.pos.y = ctrl.camypos;
    loc.pos.z = ctrl.camzpos;
    loc.ang.x = ctrl.angx;
    loc.ang.y = ctrl.angy;
    loc.ang.z = ctrl.angz;
}

void CamcontSetCameraLOOKAT_m(int pnum)
{
    auto& ctrl = cameraControlWork_m[pnum];
    auto& loc = cameraLocations[pnum];

    loc.pos.x = ctrl.camxpos;
    loc.pos.y = ctrl.camypos;
    loc.pos.z = ctrl.camzpos;

    Float x = ctrl.camxpos - ctrl.tgtxpos;
    Float z = ctrl.camzpos - ctrl.tgtzpos;
    loc.ang.x = njArcTan2(ctrl.tgtypos - ctrl.camypos, njSqrt(z * z + x * x));
    loc.ang.y = njArcTan2(x, z);
    loc.ang.z = ctrl.angz;
}

void CamcontSetCameraTGTOFST_m(int pnum)
{
    auto& ctrl = cameraControlWork_m[pnum];
    auto& loc = cameraLocations[pnum];

    Float dist = njCos(ctrl.angx) * ctrl.tgtdist;
    loc.pos.y = ctrl.tgtypos - njSin(ctrl.angx) * ctrl.tgtdist;
    loc.pos.x = njSin(ctrl.angy) * dist + ctrl.tgtxpos;
    loc.pos.z = njCos(ctrl.angy) * dist + ctrl.tgtzpos;
    loc.ang.x = ctrl.angx;
    loc.ang.y = ctrl.angy;
    loc.ang.z = ctrl.angz;
}

void InitCameraParam_m(int pnum)
{
    auto ptwp = playertwp[0];
    auto& ctrl = cameraControlWork_m[pnum];

    ctrl.tgtxpos = ptwp->pos.x;
    ctrl.tgtypos = ptwp->pos.y + 7.5f;
    ctrl.tgtzpos = ptwp->pos.z;
    ctrl.angx = 0;
    ctrl.angy = 0;
    ctrl.angz = 0;
    ctrl.tgtdist = 50.0f;

    NJS_VECTOR v = { 0.0f, 20.0f, 0.0f };
    njPushMatrix(_nj_unit_matrix_);
    njRotateZ_(ptwp->ang.z);
    njRotateX_(ptwp->ang.x);
    njRotateY_(0x4000 - ptwp->ang.y);
    njCalcVector(0, &v, &v);
    njPopMatrix(1);

    ctrl.camxpos = ctrl.tgtxpos + v.x + 3.0f;
    ctrl.camypos = ctrl.tgtypos + v.y;
    ctrl.camzpos = ctrl.tgtzpos + v.z + 3.0f;
    CamcontSetCameraLOOKAT_m(pnum);

    cameraTimer_m[pnum] = 0;
    eventReleaseTimer_m[pnum] = 0;
}

void setdistdata_m(int pnum)
{
    auto& fcwp = fcwrk_m[pnum];

    if (ssStageNumber == STAGE_SHELTER)
    {
        fcwp.dist1 = 15.0f;

        if (GetCharacterID(pnum) == Characters_Amy)
        {
            fcwp.dist2 = 30.0f;
        }
        else
        {
            fcwp.dist2 = 50.0f;
        }
    }
    else
    {
        fcwp.dist1 = 16.0f;

        if (ssStageNumber == STAGE_RUIN || ssActNumber == 2)
        {
            fcwp.dist2 = 50.0f;
        }
        else
        {
            fcwp.dist2 = 70.0f;
        }
    }
}

void calcfreecamerapos_m(int pnum)
{
    auto& fcwp = fcwrk_m[pnum];

    NJS_VECTOR v = { 0.0f, 0.0f, fcwp.dist };

    njPushMatrix(_nj_unit_matrix_);

    Angle y = fcwp._ang.y;
    if (y)
    {
        njRotateY(0, y);
    }

    Angle x = fcwp._ang.x;
    if (x)
    {
        njRotateX(0, x);
    }

    njCalcPoint(0, &v, &v);
    njPopMatrix(1u);

    taskwk* ptwp = playertwp[pnum];
    fcwp.pos.x = v.x + ptwp->pos.x;
    fcwp.pos.y = v.y + ptwp->pos.y + 10.5f;
    fcwp.pos.z = v.z + ptwp->pos.z;
}

bool freecameramode_m(int pnum)
{
    auto& system = cameraSystemWork_m[pnum];
    auto& fcmode = free_camera_mode_m[pnum];
    auto& fcwp = fcwrk_m[pnum];
    auto& loc = cameraLocations[pnum];

    /* If timer is running, free camera is temporarily disabled */
    if (fcwp.timer)
    {
        --fcwp.timer;
    }
    else
    {
        fcmode &= ~MODE_TIMER;
    }

    /* Check if free camera can proceed (enabled, authorized, not on a path, etc.) */
    if (!(fcmode & MODE_ENABLED) /*|| !(fcmode & MODE_AUTHORIZED)*/ || (playertwp[pnum]->flag & Status_OnPath)
        || system.G_scCameraMode == CAMMD_PATHCAM || system.G_scCameraMode == -1
        /*|| checkfreecameraarea(0)*/ || (fcmode & MODE_TIMER) != 0)
    {
        return false;
    }

    /* Move to default position */
    if (fcmode & MODE_UPDATE)
    {
        setdistdata_m(pnum);

        taskwk* ptwp = playertwp[pnum];

        NJS_POINT3 vec;
        vec.x = loc.pos.x - ptwp->pos.x;
        vec.y = loc.pos.y - ptwp->pos.y - 10.5f;
        vec.z = loc.pos.z - ptwp->pos.z;

        Float dist = (Float)abs((int)njScalor(&vec));
        njUnitVector(&vec);

        fcwp._ang.y = NJM_RAD_ANG(atan2f(vec.x, vec.z));
        fcwp._ang.x = NJM_RAD_ANG(-asinf(vec.y));
        fcwp._ang.z = 0;

        if (dist < fcwp.dist1 || dist > fcwp.dist2)
        {
            fcwp.dist0 = fcwp.dist1;
            fcwp.dist = fcwp.dist1;
        }
        else
        {
            fcwp.dist0 = dist;
            fcwp.dist = dist;
        }

        calcfreecamerapos_m(pnum);
        fcwp.campos = fcwp.pos;
        fcwp.counter = 0;
        fcwp.pang = { 0, 0, 0 };
        fcmode |= MODE_FIX;
    }

    /* Try to position camera properly */
    if (fcmode & MODE_FIX)
    {
        if (!(fcmode & MODE_UPDATE))
        {
            setdistdata_m(pnum);
            fcwp.counter = 0;
            fcwp.dist0 = fcwp.dist1;
            fcwp.dist = fcwp.dist1;
            calcfreecamerapos_m(pnum);
            fcwp.campos = fcwp.pos;
        }

        int passes = 60;
        while (1)
        {
            fcwp.dist0 = fcwp.dist;
            fcwp.dist *= 1.2f;

            if (fcwp.dist >= fcwp.dist2)
            {
                break;
            }

            calcfreecamerapos_m(pnum);
            fcwp.cammovepos = fcwp.campos;
            fcwp.camspd = { 0.0f, 0.0f, 0.0f };

            NJS_POINT3 p = fcwp.campos;
            NJS_POINT3 v = {};
            MSetPositionWIgnoreAttribute(&p, &v, 0, 0x400002, 10.0f);

            fcwp.campos.x = v.x + p.x;
            fcwp.campos.y = v.y + p.y;
            fcwp.campos.z = v.z + p.z;

            if (!--passes || fcwp.dist0 == fcwp.dist)
            {
                break;
            }
        }

        fcmode &= ~(MODE_UPDATE | MODE_FIX);
    }

    if (fcwp.pang.y > 0)
    {
        fcwp.pang.y -= 32;

        if (fcwp.pang.y < 0)
        {
            fcwp.pang.y = 0;
        }
    }
    else if (fcwp.pang.y < 0)
    {
        fcwp.pang.y += 32;

        if (fcwp.pang.y > 0)
        {
            fcwp.pang.y = 0;
        }
    }

    if ((perG[pnum].x2 > 0 || (perG[pnum].r - 128) << 8 > 128) && fcwp.pang.y < 256)
    {
        fcwp.pang.y += 64;
    }
    else if ((perG[pnum].x2 < 0 || (perG[pnum].l - 128) << 8 > 128) && fcwp.pang.y > -256)
    {
        fcwp.pang.y -= 64;
    }

    fcwp._ang.y += fcwp.pang.y;

    if (fcwp.pang.x > 0)
    {
        fcwp.pang.x -= 32;

        if (fcwp.pang.x < 0)
        {
            fcwp.pang.x = 0;
        }
    }
    else if (fcwp.pang.x < 0)
    {
        fcwp.pang.x += 32;

        if (fcwp.pang.x > 0)
        {
            fcwp.pang.x = 0;
        }
    }

    if (perG[pnum].y2 > 0 && fcwp.pang.x < 256)
    {
        fcwp.pang.x += 64;
    }
    else if (perG[pnum].y2 < 0 && fcwp.pang.x > -256)
    {
        fcwp.pang.x -= 64;
    }

    fcwp._ang.x += fcwp.pang.x;

    if (fcwp._ang.x >= -12288)
    {
        if (fcwp._ang.x > -512)
        {
            fcwp._ang.x = -513;
        }
    }
    else
    {
        fcwp._ang.x = -12287;
    }

    fcwp.dist = fcwp.dist0;

    calcfreecamerapos_m(pnum);
    fcwp.camspd.x = fcwp.pos.x - fcwp.campos.x;
    fcwp.camspd.y = fcwp.pos.y - fcwp.campos.y;
    fcwp.camspd.z = fcwp.pos.z - fcwp.campos.z;
    fcwp.cammovepos = fcwp.pos;

    NJS_VECTOR p = fcwp.campos;
    NJS_VECTOR v = fcwp.camspd;

    int colli_flag = 0;
    if (MSetPositionWIgnoreAttribute(&p, &v, 0, 0x400002, 10.0f))
    {
        p = fcwp.campos;
        v = fcwp.camspd;
        colli_flag = 1;
        if (MSetPositionWIgnoreAttribute(&p, &v, 0, 0x400002, 8.0f))
        {
            colli_flag = 2;
        }
    }

    fcwp.pos.x = v.x + p.x;
    fcwp.pos.y = v.y + p.y;
    fcwp.pos.z = v.z + p.z;
    fcwp.campos = fcwp.pos;

    NJS_POINT3 vec = playermwp[pnum]->spd;

    if ((float)abs((int)vec.x) >= 1.0f)
    {
        vec.x *= 1.1f;
    }
    else
    {
        vec.x = 0.0f;
    }

    taskwk* ptwp = playertwp[pnum];
    if (GetCharacterID(pnum) == Characters_Gamma && ptwp->mode == 31) /* todo: gamma mode enum */
    {
        float y = (float)abs((int)vec.y);
        if (y >= 2.0f && y <= 4.0f)
        {
            vec.y = 0.0f;
        }
    }

    if ((float)abs((int)vec.y) >= 1.0f)
    {
        vec.y *= 1.1f;
    }
    else
    {
        vec.y = 0.0f;
    }

    if ((float)abs((int)vec.z) >= 1.0f)
    {
        vec.z *= 1.1f;
    }
    else
    {
        vec.z = 0.0f;
    }

    vec.x = fcwp.campos.x - ptwp->pos.x - vec.x;
    vec.y = fcwp.campos.y - ptwp->pos.y - vec.y - 10.5f;
    vec.z = fcwp.campos.z - ptwp->pos.z - vec.z;
    njUnitVector(&vec);

    fcwp._ang.y = njArcTan2(vec.x, vec.z);
    fcwp._ang.x = -njArcSin(vec.y);

    vec.x = fcwp.campos.x - ptwp->pos.x;
    vec.y = fcwp.campos.y - ptwp->pos.y - 10.5f;
    vec.z = fcwp.campos.z - ptwp->pos.z;

    fcwp.dist = (Float)abs((int)njScalor(&vec));
    if (fcwp.dist <= fcwp.dist2)
    {
        fcwp.counter = 0;
    }
    else if (++fcwp.counter > 20)
    {
        fcmode |= MODE_FIX; /* Camera is beyond limit for too long, try to fix it */
    }

    if (colli_flag)
    {
        if (fcwp.dist <= fcwp.dist1)
        {
            fcwp.dist = fcwp.dist1;
        }
    }
    else
    {
        fcwp.dist *= 1.2f;
        if (fcwp.dist >= fcwp.dist2)
        {
            fcwp.dist = fcwp.dist2;
        }
    }

    fcwp.dist0 = fcwp.dist;
    cameraControlWork_m[pnum].camxpos = fcwp.pos.x;
    cameraControlWork_m[pnum].camypos = fcwp.pos.y;
    cameraControlWork_m[pnum].camzpos = fcwp.pos.z;
    return true;
}

void sub_4364B0_m(_OBJ_CAMERAENTRY* pEntry)
{
    cameraParam.xCamPos = pEntry->xCamPos;
    cameraParam.yCamPos = pEntry->yCamPos;
    cameraParam.zCamPos = pEntry->zCamPos;
    cameraParam.xDirPos = pEntry->xDirPos;
    cameraParam.yDirPos = pEntry->yDirPos;
    cameraParam.zDirPos = pEntry->zDirPos;
    cameraParam.xCamAng = pEntry->xCamAng;
    cameraParam.yCamAng = pEntry->yCamAng;
    cameraParam.fDistance = pEntry->fDistance;
}

void CallMode(int pnum)
{
    camera_twp->pos = cameraLocations[pnum].pos;
    camera_twp->ang = cameraLocations[pnum].ang;

    if (pnum != 0)
    {
        auto backup_pl = playertwp[0];
        auto backup_mtn = playermwp[0];
        auto backup_pwp = playerpwp[0];
        auto backup_per = perG[0];
        auto backup_work = cameraControlWork_m[0];

        playertwp[0] = playertwp[pnum];
        playermwp[0] = playermwp[pnum];
        playerpwp[0] = playerpwp[pnum];
        perG[0] = perG[pnum];
        cameraControlWork_m[0] = cameraControlWork_m[pnum];

        cameraSystemWork_m[pnum].G_pfnCamera(&cameraParam);

        cameraControlWork_m[pnum] = cameraControlWork_m[0];

        playertwp[0] = backup_pl;
        playermwp[0] = backup_mtn;
        playerpwp[0] = backup_pwp;
        perG[0] = backup_per;
        cameraControlWork_m[0] = backup_work;
    }
    else
    {
        cameraSystemWork_m[pnum].G_pfnCamera(&cameraParam);
    }
}

void CallAdjust(int pnum, bool freecam)
{
    return;

    camera_twp->pos = cameraLocations[pnum].pos;
    camera_twp->ang = cameraLocations[pnum].ang;

    if (pnum != 0)
    {
        auto backup_pl = playertwp[0];
        auto backup_mtn = playermwp[0];
        auto backup_pwp = playerpwp[0];
        auto backup_per = perG[0];
        auto backup_work = cameraControlWork_m[0];

        playertwp[0] = playertwp[pnum];
        playermwp[0] = playermwp[pnum];
        playerpwp[0] = playerpwp[pnum];
        perG[0] = perG[pnum];
        cameraControlWork_m[0] = cameraControlWork_m[pnum];

        if (freecam)
            pObjCameraAdjust[CAMADJ_FORFREECAMERA].fnAdjust(camera_twp, &oldTaskWork, &objAdjustParam_m[pnum]);
        else
            cameraSystemWork_m[pnum].G_pfnAdjust(camera_twp, &oldTaskWork, &objAdjustParam_m[pnum]);
        
        cameraControlWork_m[pnum] = cameraControlWork_m[0];

        playertwp[0] = backup_pl;
        playermwp[0] = backup_mtn;
        playerpwp[0] = backup_pwp;
        perG[0] = backup_per;
        cameraControlWork_m[0] = backup_work;
    }
    else
    {
        if (freecam)
            pObjCameraAdjust[CAMADJ_FORFREECAMERA].fnAdjust(camera_twp, &oldTaskWork, &objAdjustParam_m[pnum]);
        else
            cameraSystemWork_m[pnum].G_pfnAdjust(camera_twp, &oldTaskWork, &objAdjustParam_m[pnum]);
    }

    cameraLocations[pnum].pos = camera_twp->pos;
    cameraLocations[pnum].ang = camera_twp->ang;
}

void CameraCameraMode_m(int pnum)
{
    auto& ctrl = cameraControlWork_m[pnum];
    auto& system = cameraSystemWork_m[pnum];
    auto& loc = cameraLocations[pnum];
    auto& fcmode = free_camera_mode_m[pnum];

    if (system.G_pfnCamera)
    {
        oldTaskWork.pos = loc.pos;
        oldTaskWork.ang = loc.ang;

        ctrl.camxpos = loc.pos.x;
        ctrl.camypos = loc.pos.y;
        ctrl.camzpos = loc.pos.z;

        if ((perG[pnum].on & (Buttons_L | Buttons_R)) == (Buttons_L | Buttons_R) && (perG[pnum].press & Buttons_B))
        {
            SetFreeCamera_m(pnum, !GetFreeCamera_m(pnum));
        }

        fcmode &= ~MODE_ACTIVE;

        if (fcmode & MODE_ENABLED)
        {
            if (freecameramode_m(pnum))
                fcmode |= MODE_ACTIVE;
        }

        if (!(fcmode & MODE_ACTIVE))
        {
            fcmode |= MODE_UPDATE;

            if (system.G_scCameraAttribute & 2)
            {
                //cameraParam = externCameraParam;
            }
            else
            {
                sub_4364B0_m(system.G_ssCameraEntry == -2 ? system.G_pCameraEntry : &pObjCameraEntry[system.G_ssCameraEntry]);
            }

            cameraParam.ulTimer = system.G_ulTimer;

            /* Call current auto camera mode with information from camera layout */
            CallMode(pnum);
        }

        /* Apply processed auto camera data based on direct mode */
        switch (system.G_scCameraDirect)
        {
        case CDM_TGTOFST:
            CamcontSetCameraTGTOFST_m(pnum);
            break;
        case CDM_LOOKAT:
            CamcontSetCameraLOOKAT_m(pnum);
            break;
        case CDM_CAMSTATUS:
            CamcontSetCameraCAMSTATUS_m(pnum);
            break;
        }

        /* Run adjust function (interpolation between previous and current data) */
        if (fcmode & MODE_ACTIVE)
        {
            CallAdjust(pnum, true);
        }
        else
        {
            if ((system.G_ulTimer || !(system.G_scCameraAttribute & 1)) && cameraTimer_m[pnum] > 0x3C)
            {
                CallAdjust(pnum, false);
                ++objAdjustParam_m[pnum].counter;
            }
        }
    }
}

void CameraRunTimers(int pnum)
{
    auto& system = cameraSystemWork_m[pnum];

    /* If camera has just changed, reset timer */
    if (system.G_boolSwitched)
    {
        system.G_ulTimer = 0;
        system.G_boolSwitched = 0;
        if (!system.G_scCameraAdjust)
        {
            //if (dword_3B2CAC0)
            //{
            //    GetPlayerWorkPointer(0)->nocontimer = 30;
            //}
        }
    }
    /* otherwise run timer */
    else
    {
        ++system.G_ulTimer;
    }
}

Bool CameraCollisionShpere_m(int pnum, _OBJ_CAMERAENTRY* pCameraEntry)
{
    taskwk* ptwp = playertwp[pnum];
    Float x = ptwp->pos.x - pCameraEntry->xColPos;
    Float y = ptwp->pos.y - pCameraEntry->yColPos + 4.0f;
    Float z = ptwp->pos.z - pCameraEntry->zColPos;
    return (x * x + y * y + z * z < pCameraEntry->zColScl);
}

Bool CameraCollisionPlane_m(int pnum, _OBJ_CAMERAENTRY* pCameraEntry)
{
    Float camx = pCameraEntry->xColPos;
    Float camy = pCameraEntry->yColPos;
    Float camz = pCameraEntry->zColPos;

    Float planex = pCameraEntry->xColScl;
    Float planey = pCameraEntry->yColScl;

    taskwk* ptwp = playertwp[pnum];
    Float ctrx = camx - ptwp->pos.x;
    Float ctry = camy - ptwp->pos.y + 4.0f;
    Float ctrz = camz - ptwp->pos.z;

    if (ctrx * ctrx + ctry * ctry + ctrz * ctrz > planex * planex + planey * planey)
    {
        return FALSE;
    }

    /* Get current and previous player position to check if it passed the plane */
    NJS_POINT3 playerpos, prevplayerpos;
    GetPlayerPosition(pnum, 1, &playerpos, NULL);     /* Player position 1 frame ago */
    GetPlayerPosition(pnum, 2, &prevplayerpos, NULL); /* Player position 2 frames ago */
    playerpos.y += 4.0f;
    prevplayerpos.y += 4.0f;

    NJS_MATRIX m;
    njUnitMatrix(m);

    njTranslate(m, camx, camy, camz);

    Angle angx = pCameraEntry->xColAng;
    if (angx)
    {
        njRotateX(m, angx);
    }

    Angle angy = pCameraEntry->yColAng;
    if (angy)
    {
        njRotateY(m, angy);
    }

    njInvertMatrix(m);

    NJS_POINT3 vec;
    njCalcPoint(m, &prevplayerpos, &vec);
    prevplayerpos = vec;
    njCalcPoint(m, &playerpos, &vec);
    playerpos = vec;

    if (prevplayerpos.z < 0.0f && vec.z < 0.0f)
    {
        return FALSE;
    }

    if (prevplayerpos.z > 0.0f && vec.z > 0.0f)
    {
        return FALSE;
    }

    NJS_LINE line;
    line.px = prevplayerpos.x;
    line.vx = vec.x - prevplayerpos.x;
    line.py = prevplayerpos.y;
    line.pz = prevplayerpos.z;
    line.vy = vec.y - prevplayerpos.y;
    line.vz = vec.z - prevplayerpos.z;
    NJS_POINT3 dist;
    njDistanceL2PL(&line, &plCollision, &dist);
    return dist.x <= planex && -planex <= dist.x && dist.y <= planey && -planey <= dist.y;
}

Bool CameraCollisionCube_m(int pnum, _OBJ_CAMERAENTRY* pCameraEntry)
{
    Float camx = pCameraEntry->xColPos;
    Float camy = pCameraEntry->yColPos;
    Float camz = pCameraEntry->zColPos;

    Float boxx = pCameraEntry->xColScl;
    Float boxy = pCameraEntry->yColScl;
    Float boxz = pCameraEntry->zColScl;

    taskwk* ptwp = playertwp[pnum];
    Float ctrx = camx - ptwp->pos.x;
    Float ctry = camy - ptwp->pos.y + 4.0f;
    Float ctrz = camz - ptwp->pos.z;

    if (ctrx * ctrx + ctry * ctry + ctrz * ctrz > boxx * boxx + boxy * boxy + boxz * boxz)
    {
        return FALSE;
    }

    NJS_POINT3 playerpos;
    GetPlayerPosition(pnum, 1, &playerpos, NULL); /* Doesn't check if valid */
    playerpos.y += 4.0f;

    NJS_MATRIX m;
    njUnitMatrix(m);

    njTranslate(m, camx, camy, camz);

    Angle angx = pCameraEntry->xColAng;
    if (angx)
    {
        njRotateX(m, angx);
    }

    Angle angy = pCameraEntry->yColAng;
    if (angy)
    {
        njRotateY(m, angy);
    }

    njInvertMatrix(m);

    NJS_POINT3 local;
    njCalcPoint(m, &playerpos, &local);

    return local.x <= boxx && -boxx <= local.x
        && local.y <= boxy && -boxy <= local.y
        && local.z <= boxz && -boxz <= local.z;
}

void CameraManagerNormal_m(int pnum, Sint16 ssCameraEntry, _OBJ_CAMERAENTRY* pCameraEntry)
{
    auto& system = cameraSystemWork_m[pnum];

    if (system.G_ssCameraEntry != ssCameraEntry && playertwp[pnum])
    {
        Bool collide;
        switch (pCameraEntry->scColType)
        {
        case CAMCOL_SPHERE:
            collide = CameraCollisionShpere_m(pnum, pCameraEntry);
            break;
        case CAMCOL_PLANE:
            collide = CameraCollisionPlane_m(pnum, pCameraEntry);
            break;
        case CAMCOL_BLOCK:
            collide = CameraCollisionCube_m(pnum, pCameraEntry);
            break;
        default:
            return;
        }

        if (collide)
        {
            CameraSetCamera_m(pnum, pCameraEntry->scMode, ssCameraEntry);
        }
    }
}

void CameraManagerRegional_m(int pnum, Sint16 ssCameraEntry, _OBJ_CAMERAENTRY* pCameraEntry)
{
    auto& system = cameraSystemWork_m[pnum];

    if (system.G_ssCameraEntry == ssCameraEntry)
    {
        if (!playertwp[pnum])
        {
            return;
        }

        Bool collide;
        switch (pCameraEntry->scColType)
        {
        case CAMCOL_SPHERE:
            collide = CameraCollisionShpere_m(pnum, pCameraEntry);
            break;
        case CAMCOL_BLOCK:
            collide = CameraCollisionCube_m(pnum, pCameraEntry);
            break;
        default:
            return;
        }

        if (!collide)
        {
            CameraCancelCamera_m(pnum);
        }
    }
    else
    {
        Sint8 cameraLevel = pObjCameraMode[pCameraEntry->scMode].scCameraLevel;
        if (cameraLevel > system.G_scCameraLevel)
        {
            Bool collide;
            switch (pCameraEntry->scColType)
            {
            case CAMCOL_SPHERE:
                collide = CameraCollisionShpere_m(pnum, pCameraEntry);
                break;
            case CAMCOL_BLOCK:
                collide = CameraCollisionCube_m(pnum, pCameraEntry);
                break;
            default:
                return;
            }

            if (collide)
            {
                CameraSetCamera_m(pnum, pCameraEntry->scMode, ssCameraEntry);
            }
        }
        else if (cameraLevel == system.G_scCameraLevel && playertwp[pnum]
            && pObjCameraEntry[system.G_ssCameraEntry].scPriority < pCameraEntry->scPriority)
        {
            Bool collide;
            switch (pCameraEntry->scColType)
            {
            case CAMCOL_SPHERE:
                collide = CameraCollisionShpere_m(pnum, pCameraEntry);
                break;
            case CAMCOL_BLOCK:
                collide = CameraCollisionCube_m(pnum, pCameraEntry);
                break;
            default:
                return;
            }

            if (collide)
            {
                CameraCancelCamera_m(pnum);
                CameraSetCamera_m(pnum, pCameraEntry->scMode, ssCameraEntry);
            }
        }
    }
}

void CameraManagerEvent_m(int pnum, Sint16 ssCameraEntry, _OBJ_CAMERAENTRY* pCameraEntry)
{
    auto& system = cameraSystemWork_m[pnum];

    if (pObjCameraMode[pCameraEntry->scMode].scCameraLevel == CLEVEL_COLLISION)
    {
        if (pCameraEntry->scColType == CAMCOL_PLANE && CameraCollisionPlane_m(pnum, pCameraEntry))
        {
            Sint8 restore = CLEVEL_EVENT;
            if (system.G_ssRestoreLevel[2])
            {
                Sint8 restore_temp = system.G_ssRestoreLevel[2];
                do
                {
                    restore = restore_temp;
                    restore_temp = system.G_ssRestoreLevel[restore];
                } while (restore_temp);
            }

            _OBJ_CAMERAENTRY* entry = &pObjCameraEntry[ssCameraEntry];

            if (entry->scMode != CAMMD_COLLI)
            {
                system.G_scRestoreCameraMode[restore] = entry->scMode;
            }
            system.G_scRestoreCameraAdjust[restore] = entry->ucAdjType;

            if (entry->scMode != CAMMD_COLLI)
            {
                system.G_pfnRestoreCamera[restore] = pObjCameraMode[pCameraEntry->scMode].fnCamera;
            }
            system.G_pfnRestoreAdjust[restore] = pObjCameraAdjust[entry->ucAdjType].fnAdjust;
        }
    }
    else if (playertwp[pnum] && pObjCameraEntry[system.G_ssCameraEntry].scPriority < pCameraEntry->scPriority)
    {
        Bool collide;
        switch (pCameraEntry->scColType)
        {
        case CAMCOL_SPHERE:
            collide = CameraCollisionShpere_m(pnum, pCameraEntry);
            break;
        case CAMCOL_BLOCK:
            collide = CameraCollisionCube_m(pnum, pCameraEntry);
            break;
        default:
            return;
        }

        if (collide)
        {
            CameraReleaseEventCamera_m(pnum);
            CameraSetCamera_m(pnum, pCameraEntry->scMode, ssCameraEntry);
        }
    }
}

void CameraManager_m(int pnum)
{
    if (boolCameraCollision)
    {
        auto& system = cameraSystemWork_m[pnum];

        //somethingElseManager();

        if (system.G_scCameraMode != CAMMD_SURVEY
            && system.G_scCameraMode != CAMMD_KOSCAM
            && objCameraEntryTable[ssActNumber])
        {
            Sint8 cameraLevel = system.G_scCameraLevel;
            Sint8 originalLevel = cameraLevel;

            int i = pNumCameraEntry->ssCount - 1;
            while (i)
            {
                _OBJ_CAMERAENTRY* pEntry = &pObjCameraEntry[i];
                Sint8 modeLevel = pObjCameraMode[pEntry->scMode].scCameraLevel;

                if (modeLevel >= cameraLevel)
                {
                    switch (cameraLevel)
                    {
                    case CLEVEL_NORMAL:
                        if (modeLevel != CLEVEL_COLLISION)
                        {
                            CameraManagerNormal_m(pnum, i, pEntry);
                            cameraLevel = system.G_scCameraLevel; /* check if it has changed */
                        }
                        break;
                    case CLEVEL_AREA:
                    case CLEVEL_COMPULSION:
                        if (modeLevel != CLEVEL_COLLISION)
                        {
                            CameraManagerRegional_m(pnum, i, pEntry);
                            cameraLevel = system.G_scCameraLevel; /* check if it has changed */
                        }
                        break;
                    case CLEVEL_EVENT:
                        CameraManagerEvent_m(pnum, i, pEntry);
                        cameraLevel = system.G_scCameraLevel; /* check if it has changed */
                        break;
                    default:
                        break;
                    }

                    /* Retry if level changed */
                    if (cameraLevel < originalLevel)
                    {
                        i = 0;
                        originalLevel = cameraLevel;
                        continue;
                    }
                }

                --i;
            }
        }
    }
}

void cameraAdjustCheck_m(int pnum, taskwk* pTaskWork, taskwk* pOldTaskWork)
{
    Float y = (pTaskWork->pos.y - pOldTaskWork->pos.y);
    Float z = (pTaskWork->pos.z - pOldTaskWork->pos.z);
    Float x = (pTaskWork->pos.x - pOldTaskWork->pos.x);
    if ((x * x) + ((z * z) + (y * y)) < 0.01f)
    {
        if (DiffAngle(pTaskWork->ang.x, pOldTaskWork->ang.x) < 91
            && DiffAngle(pTaskWork->ang.y, pOldTaskWork->ang.y) < 91
            && DiffAngle(pTaskWork->ang.z, pOldTaskWork->ang.z) < 91)
        {
            switch (cameraSystemWork_m[pnum].G_scCameraAdjust)
            {
            case CAMADJ_NONE:
            case CAMADJ_NORMAL_S:
            case CAMADJ_SLOW:
            case CAMADJ_SLOW_S:
                SetAdjustMode_m(pnum, CAMADJ_NONE);
                break;
            case CAMADJ_NORMAL:
                SetAdjustMode_m(pnum, CAMADJ_SLOW);
                break;
            default:
                return;
            }
        }
    }
}

void sub_436690_m(int pnum)
{
    njSubVector(&__PlayerStatus_last_pos_m[pnum], &playertwp[pnum]->pos);

    if (njScalor(&__PlayerStatus_last_pos_m[pnum]) >= 0.1f || !(playertwp[pnum]->flag & Status_Ground))
    {
        Player_stop_flag = FALSE;
    }
    else
    {
        Player_stop_flag = TRUE;
    }

    __PlayerStatus_last_pos_m[pnum] = playertwp[pnum]->pos;
}

void sub_436700_m(int pnum)
{
    CameraInertia.x = cameraLocations[pnum].pos.x - __CameraInertia_last_pos_m[pnum].x;
    CameraInertia.y = cameraLocations[pnum].pos.y - __CameraInertia_last_pos_m[pnum].y;
    CameraInertia.z = cameraLocations[pnum].pos.z - __CameraInertia_last_pos_m[pnum].z;
    __CameraInertia_last_pos_m[pnum] = cameraLocations[pnum].pos;
}

void __cdecl Camera_r(task* tp)
{
    auto twp = tp->twp;
    ResetMaterial();

    if (twp->mode == 0)      /* First init pass */
    {
        oldTaskWork = {};
        camera_twp = twp;
        twp->mode = 1;
        boolCameraCollision = 1;
        late_execCancel();
        cameraready = FALSE;
    }
    else if (twp->mode == 1) /* Second init pass */
    {
        //if (start_camera_mode != -1)
        //{
        //    CameraSetNormalCamera(start_camera_mode, 0);
        //}

        for (int i = 0; i < PLAYER_MAX; ++i)
        {
            InitCameraParam_m(i);

            __PlayerStatus_last_pos_m[i] = playertwp[0]->pos;
            __CameraInertia_last_pos_m[i] = camera_twp->pos;

            CameraCameraMode_m(i);
        }

        twp->mode = 2;
        tp->disp = cameraDisplay_r;

        sub_437100();
        late_execCancel();
        cameraready = FALSE;
    }
    else
    {
        ++debug_disp_camera_timer;

        for (unsigned int i = 0; i < multiplayer::GetPlayerCount(); ++i)
        {
            if (playertp[i])
            {
                if (camera_mode_m[i] == 2)
                {
                    camera_mode_m[i] = 0;
                    CameraReleaseEventCamera();
                }

                sub_436690_m(i);
                sub_436700_m(i);
                CameraManager_m(i);
                CameraRunTimers(i);
                CameraCameraMode_m(i);
                cameraAdjustCheck_m(i, twp, &oldTaskWork);

                if (camera_mode_m[i] == 1)
                {
                    camera_mode_m[i] = 2;
                }
            }
        }

        cameraready = TRUE;
    }

    for (int i = 0; i < PLAYER_MAX; ++i)
    {
        ++cameraTimer_m[i];
        ++eventReleaseTimer_m[i];
    }

    camera_twp->pos = cameraLocations[0].pos;
    camera_twp->ang = cameraLocations[0].ang;

    tp->disp(tp);
}

void __cdecl InitFreeCamera_r()
{
    for (int i = 0; i < PLAYER_MAX; ++i)
    {
        fcwrk_m[i].timer = 60;
        free_camera_mode_m[i] |= MODE_UPDATE | MODE_TIMER | MODE_AUTHORIZED;
    }
}

void __cdecl ResetFreeCamera_r()
{
    for (int i = 0; i < PLAYER_MAX; ++i)
    {
        fcwrk_m[i].timer = 60;
        free_camera_mode_m[i] |= MODE_UPDATE | MODE_TIMER;
    }
}

void InitCamera()
{
    Camera_t = new Trampoline(0x438090, 0x438097, Camera_r);
    CameraPause_t = new Trampoline(0x4373D0, 0x4373D7, CameraPause_r);
    cameraDisplay_t = new Trampoline(0x4370F0, 0x4370F5, cameraDisplay_r);

    WriteJump((void*)0x434870, InitFreeCamera_r);
    WriteJump((void*)0x434880, ResetFreeCamera_r);

    CameraAdjust[CAMADJ_FORFREECAMERA].fnAdjust = AdjustForFreeCamera_m;
    for (int i = CAMADJ_THREE1; i <= CAMADJ_RELATIVE6C; ++i)
        CameraAdjust[i].fnAdjust = AdjustThreePoint_m;
}

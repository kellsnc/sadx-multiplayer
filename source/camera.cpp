#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "VariableHook.hpp"
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
VariableHook<NJS_CACTION*, 0x3B2C950> pCameraAction_m;
VariableHook<Float, 0x3B2CA6C> fActionFrame_m;
VariableHook<Bool, 0x3B2CAC0> dword_3B2CAC0_m;
VariableHook<Bool, 0x3B2C6C0> flagCameraNoUnderWater_m;

static taskwk* backup_pl;
static motionwk2* backup_mtn;
static playerwk* backup_pwp;
static PDS_PERIPHERAL backup_per;
static _camcontwk backup_work;
static CAM_ANYPARAM backup_any;

static CameraLocation cameraLocations[PLAYER_MAX];
static Angle ds_perspective_value_m[PLAYER_MAX];

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

void SetFreeCameraMode_m(int pnum, Sint32 sw)
{
    auto& fcmode = free_camera_mode_m[pnum];
    if (sw)
    {
        if (!(fcmode & MODE_AUTHORIZED))
        {
            fcmode |= MODE_AUTHORIZED | MODE_UPDATE;
        }
    }
    else
    {
        fcmode = fcmode & ~MODE_AUTHORIZED | MODE_UPDATE;
    }
}

Bool GetFreeCameraMode_m(int pnum)
{
    return !!(free_camera_mode_m[pnum] & MODE_AUTHORIZED);
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
        dword_3B2CAC0_m[pnum] = TRUE;
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
        dword_3B2CAC0_m[pnum] = TRUE;
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

void CameraSetNormalCamera_m(int pnum, Sint16 ssCameraMode, Uint8 ucAdjustType)
{
    auto& system = cameraSystemWork_m[pnum];

    if (!system.G_scCameraLevel)
    {
        Sint16 cameraLevel = system.G_ssCameraEntry;

        system.G_scRestoreAttribute[0] = system.G_scCameraAttribute;
        system.G_ssRestoreLevel[0] = 0;
        system.G_ssRestoreEntry[0] = cameraLevel;
        system.G_scRestoreCameraMode[0] = system.G_scCameraMode;
        system.G_pfnRestoreCamera[0] = system.G_pfnCamera;

        Sint8 adjust = cameraLevel == -1 ? 21 : pObjCameraEntry[cameraLevel].ucAdjType;

        system.G_scRestoreCameraAdjust[cameraLevel] = adjust;
        system.G_pfnRestoreAdjust[0] = pObjCameraAdjust[adjust].fnAdjust;

        system.G_scCameraLevel = CLEVEL_NORMAL;

        SetCameraMode_m(pnum, ssCameraMode);
        SetAdjustMode_m(pnum, ucAdjustType);

        if (pObjCameraAdjust[ucAdjustType].slAttribute)
        {
            system.G_scCameraAttribute |= 1u;
        }

        system.G_scCameraDirect = pObjCameraMode[ssCameraMode].scCameraDirectMode;
        system.G_boolSwitched = 1;
    }
}

void CameraReleaseCollisionCamera_m(int pnum)
{
    auto& system = cameraSystemWork_m[pnum];

    if (system.G_scCameraLevel == CLEVEL_COLLISION)
    {
        system.G_ssCameraEntry = system.G_ssRestoreEntry[CLEVEL_COLLISION];
        system.G_scCameraMode = system.G_scRestoreCameraMode[CLEVEL_COLLISION];
        system.G_scCameraLevel = system.G_ssRestoreLevel[CLEVEL_COLLISION];
        system.G_scCameraAttribute = system.G_scRestoreAttribute[CLEVEL_COLLISION];
        system.G_pfnCamera = system.G_pfnRestoreCamera[CLEVEL_COLLISION];
        system.G_scCameraAdjust = system.G_scRestoreCameraAdjust[CLEVEL_COLLISION];
        system.G_pfnAdjust = system.G_pfnRestoreAdjust[CLEVEL_COLLISION];
        system.G_scCameraDirect = pObjCameraMode[system.G_scRestoreCameraMode[CLEVEL_COLLISION]].scCameraDirectMode;
        system.G_boolSwitched = 1;
        system.G_ulTimer = 0;
        objAdjustParam_m[pnum].counter = 0;

        if (!(system.G_ssRestoreLevel[CLEVEL_COLLISION] >> 8) && default_camera_mode != -1)
        {
            CameraSetNormalCamera_m(pnum, default_camera_mode, default_camera_adjust);
        }
    }
}

void CameraSetCollisionCamera(int pnum, Sint16 ssCameraMode, Uint8 ucAdjustType)
{
    auto& system = cameraSystemWork_m[pnum];

    if (system.G_scCameraLevel <= CLEVEL_COLLISION)
    {
        if (system.G_scCameraLevel != CLEVEL_COLLISION)
        {
            system.G_scRestoreAttribute[CLEVEL_COLLISION] = system.G_scCameraAttribute;
            system.G_ssRestoreLevel[CLEVEL_COLLISION] = system.G_scCameraLevel;
            system.G_scRestoreCameraMode[CLEVEL_COLLISION] = system.G_scCameraMode;
            system.G_ssRestoreEntry[CLEVEL_COLLISION] = system.G_ssCameraEntry;
            system.G_pfnRestoreCamera[CLEVEL_COLLISION] = system.G_pfnCamera;

            Sint8 adjust = system.G_ssCameraEntry == -1 ? 21 : pObjCameraEntry[system.G_ssCameraEntry].ucAdjType;
            system.G_scRestoreCameraAdjust[CLEVEL_COLLISION] = adjust;
            system.G_pfnRestoreAdjust[CLEVEL_COLLISION] = pObjCameraAdjust[adjust].fnAdjust;
            system.G_scCameraLevel = CLEVEL_COLLISION;
        }

        SetCameraMode_m(pnum, ssCameraMode);
        SetAdjustMode_m(pnum, ucAdjustType);

        system.G_scCameraAttribute = 2;
        if (pObjCameraAdjust[ucAdjustType].slAttribute)
        {
            system.G_scCameraAttribute = 3;
        }

        system.G_scCameraDirect = pObjCameraMode[ssCameraMode].scCameraDirectMode;
        system.G_boolSwitched = 1;
    }
}

void CameraSetCollisionCameraFunc_m(int pnum, void(__cdecl* fnCamera)(_OBJ_CAMERAPARAM*), Uint8 ucAdjustType, Sint8 scCameraDirect)
{
    auto& system = cameraSystemWork_m[pnum];

    if (system.G_scCameraLevel <= CLEVEL_COLLISION)
    {
        if (system.G_scCameraLevel != CLEVEL_COLLISION)
        {
            system.G_ssRestoreLevel[CLEVEL_COLLISION] = system.G_scCameraLevel;
            system.G_scRestoreAttribute[CLEVEL_COLLISION] = system.G_scCameraAttribute;
            system.G_ssRestoreEntry[CLEVEL_COLLISION] = system.G_ssCameraEntry;
            system.G_scRestoreCameraMode[CLEVEL_COLLISION] = system.G_scCameraMode;
            system.G_pfnRestoreCamera[CLEVEL_COLLISION] = system.G_pfnCamera;

            Sint8 adjust = system.G_ssCameraEntry == -1 ? 21 : pObjCameraEntry[system.G_ssCameraEntry].ucAdjType;
            system.G_scRestoreCameraAdjust[CLEVEL_COLLISION] = adjust;
            system.G_pfnRestoreAdjust[CLEVEL_COLLISION] = pObjCameraAdjust[adjust].fnAdjust;
            system.G_scCameraLevel = CLEVEL_COLLISION;
        }

        system.G_ssCameraEntry = -1;
        system.G_scCameraMode = -1;
        system.G_pfnCamera = fnCamera;

        SetAdjustMode_m(pnum, ucAdjustType);

        system.G_scCameraAttribute = 2;

        if (pObjCameraAdjust[ucAdjustType].slAttribute)
        {
            system.G_scCameraAttribute = 3;
        }

        system.G_scCameraDirect = scCameraDirect;
        system.G_boolSwitched = 1;
    }
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

        if (!(system.G_ssRestoreLevel[CLEVEL_EVENT] >> 8) && default_camera_mode != -1)
        {
            CameraSetNormalCamera_m(pnum, default_camera_mode, default_camera_adjust);
        }
    }

    eventReleaseTimer_m[pnum] = 0;
}

void CanselCAMSURVEY_m(int pnum)
{
    if (cameraSystemWork_m[pnum].G_scCameraMode == CAMMD_SURVEY || cameraSystemWork_m[pnum].G_scCameraMode == CAMMD_KOSCAM)
    {
        CameraReleaseCollisionCamera_m(pnum);
        SetAdjustMode_m(pnum, CAMADJ_TIME);
    }
}

void CameraSetEventCamera_m(int pnum, Sint16 ssCameraMode, Uint8 ucAdjustType)
{
    auto& system = cameraSystemWork_m[pnum];

    CanselCAMSURVEY_m(pnum);

    if (system.G_scCameraLevel <= CLEVEL_EVENT)
    {
        if (system.G_scCameraLevel != CLEVEL_EVENT)
        {
            system.G_ssRestoreLevel[CLEVEL_EVENT] = system.G_scCameraLevel;
            system.G_scRestoreCameraMode[CLEVEL_EVENT] = system.G_scCameraMode;
            system.G_scRestoreAttribute[CLEVEL_EVENT] = system.G_scCameraAttribute;
            system.G_ssRestoreEntry[CLEVEL_EVENT] = system.G_ssCameraEntry;
            system.G_pfnRestoreCamera[CLEVEL_EVENT] = system.G_pfnCamera;

            Sint8 adjust = system.G_ssCameraEntry == -1 ? 21 : pObjCameraEntry[system.G_ssCameraEntry].ucAdjType;

            system.G_scRestoreCameraAdjust[CLEVEL_EVENT] = adjust;
            system.G_pfnRestoreAdjust[CLEVEL_EVENT] = pObjCameraAdjust[adjust].fnAdjust;
            system.G_scCameraLevel = CLEVEL_EVENT;
        }

        SetCameraMode_m(pnum, ssCameraMode);
        SetAdjustMode_m(pnum, ucAdjustType);

        system.G_scCameraAttribute = 2;
        if (pObjCameraAdjust[ucAdjustType].slAttribute)
        {
            system.G_scCameraAttribute = 3;
        }

        system.G_scCameraDirect = pObjCameraMode[ssCameraMode].scCameraDirectMode;
        system.G_boolSwitched = 1;
    }
}

void CameraSetEventCameraFunc_m(int pnum, void(__cdecl* fnCamera)(_OBJ_CAMERAPARAM*), Uint8 ucAdjustType, Sint8 scCameraDirect)
{
    auto& system = cameraSystemWork_m[pnum];

    CanselCAMSURVEY_m(pnum);

    if (system.G_scCameraLevel <= CLEVEL_EVENT)
    {
        if (system.G_scCameraLevel != CLEVEL_EVENT)
        {
            system.G_ssRestoreLevel[CLEVEL_EVENT] = system.G_scCameraLevel;
            system.G_scRestoreCameraMode[CLEVEL_EVENT] = system.G_scCameraMode;
            system.G_scRestoreAttribute[CLEVEL_EVENT] = system.G_scCameraAttribute;
            system.G_ssRestoreEntry[CLEVEL_EVENT] = system.G_ssCameraEntry;
            system.G_pfnRestoreCamera[CLEVEL_EVENT] = system.G_pfnCamera;

            Sint8 adjust = system.G_ssCameraEntry == -1 ? 21 : pObjCameraEntry[system.G_ssCameraEntry].ucAdjType;

            system.G_scRestoreCameraAdjust[CLEVEL_EVENT] = adjust;
            system.G_pfnRestoreAdjust[CLEVEL_EVENT] = pObjCameraAdjust[adjust].fnAdjust;
            system.G_scCameraLevel = CLEVEL_EVENT;
        }

        system.G_ssCameraEntry = -1;
        system.G_scCameraMode = -1;
        system.G_pfnCamera = fnCamera;

        SetAdjustMode_m(pnum, ucAdjustType);

        system.G_scCameraAttribute = 2;
        if (pObjCameraAdjust[ucAdjustType].slAttribute)
        {
            system.G_scCameraAttribute = 3;
        }

        system.G_scCameraDirect = scCameraDirect;
        system.G_boolSwitched = 1;
    }
}

Bool CameraCameraAction(int pnum, NJS_CACTION* caction, Float frame)
{
    switch (camera_mode_m[pnum])
    {
    case 0:
        CameraSetEventCameraFunc_m(pnum, (CamFuncPtr)nullsub, 0, 0);
    case 2:
        pCameraAction = caction;
        camera_mode_m[pnum] = 1;
        fActionFrame = frame;
        return FALSE;
    default:
        return TRUE;
    }
}

void ResetCameraTimer_m(int pnum)
{
    cameraTimer_m[pnum] = 0;
}

void CameraViewSet_m(int pnum)
{
    if (camera_mode_m[pnum])
    {
        /* Action camera (animation) */

        NJS_VECTOR pos, vec;
        Angle roll, fov;
        NJS_CMOTION_DATA mdata;

        mdata.pos = (Float*)&pos;
        mdata.vect = (Float*)&vec;
        mdata.roll = &roll;
        mdata.ang = &fov;
        njGetCameraMotion(pCameraAction[pnum].camera, pCameraAction[pnum].motion, &mdata, fActionFrame_m[pnum]);
        njSetPerspective(fov);

        NJS_CAMERA camera = View;
        camera.px = pos.x;
        camera.py = pos.y;
        camera.pz = pos.z;
        camera.vx = vec.x;
        camera.vy = vec.y;
        camera.vz = vec.z;
        camera.roll = roll;
        njSetCamera(&camera);

        /* Some hack for an Egg Carrier camera action */
        if (GetStageNumber() == 0x2000)
        {
            camera_twp->ang.y = 0;
        }
    }
    else
    {
        /* Normal camera */

        auto& loc = cameraLocations[pnum];
        auto& work = cameraSystemWork_m[pnum];

        NJS_VECTOR vs = { 0.0f, 0.0f, -1.0f };
        NJS_VECTOR vd;

        njPushMatrix(_nj_unit_matrix_);

        njRotateY_(loc.ang.y);
        njRotateX_(loc.ang.x);
        njRotateZ_(loc.ang.z);

        njCalcVector(0, &vs, &vd);
        njPopMatrix(1u);

        View.roll = -loc.ang.z;
        View.px = work.G_vecCameraOffset.x + loc.pos.x;
        View.py = work.G_vecCameraOffset.y + loc.pos.y;
        View.pz = work.G_vecCameraOffset.z + loc.pos.z;
        View.vx = vd.x;
        View.vy = vd.y;
        View.vz = vd.z;

        njSetPerspective(ds_GetPerspective_m(pnum));
        njSetCamera(&View);

        work.G_vecCameraOffset.x = 0.0f;
        work.G_vecCameraOffset.y = 0.0f;
        work.G_vecCameraOffset.z = 0.0f;

        camera_twp->pos = loc.pos;
        camera_twp->ang = loc.ang;
    }
}

void ApplyMultiCamera(int pnum)
{
    if (!camera_twp || !playertwp[pnum] || pnum >= PLAYER_MAX)
    {
        return;
    }

    CameraViewSet_m(pnum);
}

void CameraFilter_m(int pnum)
{
    // This function is restored by dc conversion
    if (DreamcastConversionEnabled == false)
    {
        return;
    }

    bool no_draw = flagCameraNoUnderWater_m[pnum] != FALSE;

    if (!ChkPause())
    {
        flagCameraNoUnderWater_m[pnum] = 0;
    }

    if (no_draw)
    {
        return;
    }

    auto& loc = cameraLocations[pnum];

    NJS_VECTOR v = { 0.0f, 0.0f, -1.5f };
    njPushMatrix(_nj_unit_matrix_);
    njTranslateV(0, &loc.pos);
    njRotateY_(loc.ang.y);
    njRotateX_(loc.ang.x);
    njCalcPoint(0, &v, &v);

    NJS_PLANE pl1;
    pl1.px = v.x;
    pl1.py = v.y;
    pl1.pz = v.z;
    pl1.vx = 0.0f;
    pl1.vy = 0.0f;
    pl1.vz = -1.0f;

    njCalcVector(0, (NJS_VECTOR*)&pl1.vx, (NJS_VECTOR*)&pl1.vx);
    njPopMatrix(1u);

    zxsdwstr carry;
    carry.pos = v;
    ListGroundForCollision(v.x, v.y, v.z, 200.0f);

    NJS_VECTOR upper_pos, lower_pos;
    upper_pos.y = 1000000.0f;
    lower_pos.y = -1000000.0f;

    if (numLandCollList > 0)
    {
        for (Int i = 0; i < numLandCollList; ++i)
        {
            NJS_OBJECT* pObject = LandCollList[i].pObject;

            if (LandCollList[i].pTask)
            {
                NJS_POINT3 center;
                center.x = pObject->pos[0] - v.x;
                center.y = 0.0;
                center.z = pObject->pos[2] - v.z;

                Float scale = pObject->scl[0];

                if (scale < pObject->scl[1])
                {
                    scale = pObject->scl[1];
                }

                if (scale < pObject->scl[2])
                {
                    scale = pObject->scl[2];
                }

                if (njScalor(&center) > scale * ((NJS_MODEL*)(pObject->model))->r)
                {
                    continue;
                }
            }

            if (LandCollList[i].slAttribute & (ColFlags_Water | 0x400000))
            {
                if (GetZxShadowOnFDPolygon(&carry, pObject))
                {
                    if (carry.upper.findflag == TRUE && carry.pos.y < carry.upper.onpos && carry.upper.onpos < upper_pos.y)
                    {
                        upper_pos.y = carry.upper.onpos;
                    }
                    else if (carry.lower.findflag == TRUE && carry.pos.y > carry.lower.onpos && carry.lower.onpos < lower_pos.y)
                    {
                        lower_pos.y = carry.lower.onpos;
                    }
                }
            }
        }

        if (upper_pos.y != 1000000.0f || lower_pos.y != -1000000.0f)
        {
            NJS_PLANE pl_upper, pl_lower;
            NJS_LINE ln;

            pl_upper.px = v.x;
            pl_lower.px = v.x;
            pl_upper.pz = v.z;
            pl_lower.pz = v.z;
            pl_upper.py = upper_pos.y;
            pl_upper.vx = 0.0f;
            pl_upper.vy = 1.0f;
            pl_upper.vz = 0.0f;
            pl_lower.py = upper_pos.y; /* should be lower_pos */
            pl_lower.vx = 0.0f;
            pl_lower.vy = 1.0f;
            pl_lower.vz = 0.0f;

            if (njDistancePL2PL(&pl1, &pl_upper, &ln) == 0.0f)
            {
                upper_pos.x = ln.px;
                upper_pos.y = ln.py;
                upper_pos.z = ln.pz;
            }
            else
            {
                upper_pos.y = -1000000.0f;
            }

            if (njDistancePL2PL(&pl1, &pl_lower, &ln) == 0.0f)
            {
                lower_pos.x = ln.px;
                lower_pos.y = ln.py;
                lower_pos.z = ln.pz;
            }
            else
            {
                upper_pos.y = -1000000.0f; /* should be lower_pos */
            }

            NJS_POINT2 pos;

            njPushMatrix(0);
            njProjectScreen(0, &upper_pos, &pos);
            njPopMatrix(1u);

            if (pos.y < 0.0f || pos.y > 480.0f)
            {
                njPushMatrix(0);
                njProjectScreen(0, &lower_pos, &pos);
                njPopMatrix(1u);

                if (auto ratio = SplitScreen::GetScreenRatio(pnum))
                {
                    NJS_POINT2 rectpos[4];
                    static NJS_COLOR rectcol[4] = { 0x40000040, 0x40000040, 0x40000040, 0x40000040 };
                    NJS_POINT2COL rectp2;

                    rectp2.p = rectpos;
                    rectp2.col = rectcol;
                    rectp2.tex = NULL;
                    rectp2.num = 2;

                    Float x = (float)HorizontalResolution * ratio->x;
                    Float y = (float)VerticalResolution * ratio->y;
                    Float w = (float)HorizontalResolution * ratio->w;
                    Float h = (float)VerticalResolution * ratio->h;

                    rectp2.p[0] = { x, y };
                    rectp2.p[1] = { x + w, y };
                    rectp2.p[2] = { x, y + h };
                    rectp2.p[3] = { x + w, y + h };

                    njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
                    njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);

                    /* Drawn twice, would need late_DrawPolygon2D in SADX */
                    late_DrawPolygon2D(&rectp2, 4, 0.01f, NJD_FILL | NJD_TRANSPARENT, LATE_MAT);
                    late_DrawPolygon2D(&rectp2, 4, 0.01f, NJD_FILL | NJD_TRANSPARENT, LATE_MAT);
                }
            }
        }
    }
}

void __cdecl cameraDisplay_r(task* tp)
{
    if (SplitScreen::IsActive())
    {
        ApplyMultiCamera(SplitScreen::numScreen);
        CameraFilter_m(SplitScreen::numScreen);
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

// To avoid having to rewrite every camera, and to be compatible with eventual camera mods
// we replace all references to player 0 with needed player
void PushPlayerSwap(int pnum)
{
    camera_twp->pos = cameraLocations[pnum].pos;
    camera_twp->ang = cameraLocations[pnum].ang;

    if (pnum != 0)
    {
        camcont_wp = &cameraControlWork_m[pnum];
        pCameraSystemWork = &cameraSystemWork_m[pnum];
        pObjAdjustParam = &objAdjustParam_m[pnum];

        backup_pl = playertwp[0];
        backup_mtn = playermwp[0];
        backup_pwp = playerpwp[0];
        backup_per = perG[0];
        backup_any = *GetCamAnyParam(0);

        playertwp[0] = playertwp[pnum];
        playermwp[0] = playermwp[pnum];
        playerpwp[0] = playerpwp[pnum];
        perG[0] = perG[pnum];
        *GetCamAnyParam(0) = *GetCamAnyParam(pnum);
    }
}

// Restore changes
void PopPlayerSwap(int pnum)
{
    if (pnum != 0)
    {
        camcont_wp = &cameraControlWork_m[0];
        pCameraSystemWork = &cameraSystemWork_m[0];
        pObjAdjustParam = &objAdjustParam_m[0];
        *GetCamAnyParam(pnum) = *GetCamAnyParam(0);

        playertwp[0] = backup_pl;
        playermwp[0] = backup_mtn;
        playerpwp[0] = backup_pwp;
        perG[0] = backup_per;
        *GetCamAnyParam(0) = backup_any;
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

        if ((perG[pnum].on & (Buttons_L | Buttons_R)) == (Buttons_L | Buttons_R) && (perG[pnum].press & Buttons_Y))
        {
            SetFreeCamera_m(pnum, !GetFreeCamera_m(pnum));
        }

        PushPlayerSwap(pnum);

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
            cameraSystemWork_m[pnum].G_pfnCamera(&cameraParam);
        }

        /* Apply processed auto camera data based on direct mode */
        switch (system.G_scCameraDirect)
        {
        case CDM_TGTOFST:
            CamcontSetCameraTGTOFST(camera_twp);
            break;
        case CDM_LOOKAT:
            CamcontSetCameraLOOKAT(camera_twp);
            break;
        case CDM_CAMSTATUS:
            CamcontSetCameraCAMSTATUS(camera_twp);
            break;
        }

        /* Run adjust function (interpolation between previous and current data) */
        if (fcmode & MODE_ACTIVE)
        {
            pObjCameraAdjust[CAMADJ_FORFREECAMERA].fnAdjust(camera_twp, &oldTaskWork, &objAdjustParam_m[pnum]);
        }
        else
        {
            if ((system.G_ulTimer || !(system.G_scCameraAttribute & 1)) && cameraTimer_m[pnum] > 0x3C)
            {
                cameraSystemWork_m[pnum].G_pfnAdjust(camera_twp, &oldTaskWork, &objAdjustParam_m[pnum]);
                ++objAdjustParam_m[pnum].counter;
            }
        }

        PopPlayerSwap(pnum);
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
            /* I have no idea what this does */
            if (dword_3B2CAC0_m[pnum] && playerpwp[pnum])
            {
                playerpwp[pnum]->nocontimer = 30;
            }
        }
    }
    /* otherwise run timer */
    else
    {
        ++system.G_ulTimer;
    }

    dword_3B2CAC0_m[pnum] = 0;
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
        if (DiffAngle(pTaskWork->ang.x, pOldTaskWork->ang.x) < 0x5B
            && DiffAngle(pTaskWork->ang.y, pOldTaskWork->ang.y) < 0x5B
            && DiffAngle(pTaskWork->ang.z, pOldTaskWork->ang.z) < 0x5B)
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
        for (int i = 0; i < PLAYER_MAX; ++i)
        {
            if (start_camera_mode != -1)
            {
                CameraSetNormalCamera_m(i, start_camera_mode, 0);
            }

            InitCameraParam_m(i);

            __PlayerStatus_last_pos_m[i] = playertwp[i] ? playertwp[i]->pos : playertwp[0]->pos;
            __CameraInertia_last_pos_m[i] = camera_twp->pos;

            if (SplitScreen::IsScreenEnabled(i))
            {
                CameraCameraMode_m(i);
            }
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
            if (SplitScreen::IsScreenEnabled(i))
            {
                if (camera_mode_m[i] == 2)
                {
                    camera_mode_m[i] = 0;
                    CameraReleaseEventCamera_m(i);
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

#pragma region Vanilla patches
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

void __cdecl SetAdjustMode_r(Sint32 AdjustType)
{
    pCameraSystemWork->G_scCameraAdjust = AdjustType;
    pCameraSystemWork->G_pfnAdjust = pObjCameraAdjust[AdjustType].fnAdjust;
    pObjAdjustParam->counter = 0;
}

Sint32 __cdecl GetAdjustMode_r()
{
    return pCameraSystemWork->G_scCameraAdjust;
}

Sint32 __cdecl GetCameraMode_r()
{
    return pCameraSystemWork->G_scCameraMode;
}

void __cdecl SetCameraMode_r(Sint32 mode)
{
    pCameraSystemWork->G_scCameraMode = mode;
    pCameraSystemWork->G_ssCameraEntry = -1;
    pCameraSystemWork->G_pfnCamera = pObjCameraMode[mode].fnCamera;
}

void __cdecl ChangeCamsetMode_r(Sint8 mode)
{
    pCameraSystemWork->G_scCameraDirect = mode;
}

void __cdecl CamcontSetCameraCAMSTATUS_r(taskwk* pTaskWork)
{
    pTaskWork->pos = *(NJS_VECTOR*)&camcont_wp->camxpos;
    pTaskWork->ang = *(Angle3*)&camcont_wp->angx;
}

void __cdecl CamcontSetCameraLOOKAT_r(taskwk* pTaskWork)
{
    pTaskWork->pos = *(NJS_VECTOR*)&camcont_wp->camxpos;

    Float x = camcont_wp->camxpos - camcont_wp->tgtxpos;
    Float z = camcont_wp->camzpos - camcont_wp->tgtzpos;
    pTaskWork->ang.x = njArcTan2(camcont_wp->tgtypos - camcont_wp->camypos, njSqrt(z * z + x * x));
    pTaskWork->ang.y = njArcTan2(x, z);
    pTaskWork->ang.z = camcont_wp->angz;
}

void __cdecl CamcontSetCameraTGTOFST_r(taskwk* pTaskWork)
{
    Float dist = njCos(camcont_wp->angx) * camcont_wp->tgtdist;
    pTaskWork->pos.y = camcont_wp->tgtypos - njSin(camcont_wp->angx) * camcont_wp->tgtdist;
    pTaskWork->pos.x = njSin(camcont_wp->angy) * dist + camcont_wp->tgtxpos;
    pTaskWork->pos.z = njCos(camcont_wp->angy) * dist + camcont_wp->tgtzpos;
    pTaskWork->ang = *(Angle3*)&camcont_wp->angx;
}

void __cdecl cameraModeInit_r()
{
    for (auto& system : cameraSystemWork_m)
    {
        system.G_scCameraLevel = 0;
        system.G_ssCameraEntry = 0;

        Sint8 cameraMode = pObjCameraEntry[0].scMode;
        system.G_scCameraMode = cameraMode;

        _OBJ_CAMERAMODE* objCameraMode = &pObjCameraMode[cameraMode];
        system.G_pfnCamera = objCameraMode->fnCamera;
        system.G_scCameraDirect = objCameraMode->scCameraDirectMode;

        system.G_scCameraAdjust = 0;
        system.G_pfnAdjust = pObjCameraAdjust[0].fnAdjust;

        system.G_boolSwitched = 1;
        system.G_scCameraAttribute = 2;
    }
    
    for (auto& param : objAdjustParam_m)
    {
        param.counter = 0;
    }
}

void __cdecl AddCameraStage_r(Sint16 ssStep)
{
    if (ChkGameMode())
    {
        ssAct += ssStep;

        pNumCameraEntry = objCameraEntryTable[ssAct];
        pObjCameraEntry = (_OBJ_CAMERAENTRY*)&pNumCameraEntry[8];

        cameraModeInit_r();

        for (int i = 0; i < PLAYER_MAX; ++i)
        {
            cameraTimer_m[i] = 0;
            eventReleaseTimer_m[i] = 0;
        }
    }
}
#pragma endregion

#pragma region FOV
Angle ds_GetPerspective_m(int pnum)
{
    return ds_perspective_value_m[pnum] == 0 ? 0x31C7 : ds_perspective_value_m[pnum];
};

void njSetPerspective_m(int pnum, Angle ang)
{
    ds_perspective_value_m[pnum] = ang;
};

void ResetPerspective_m(int pnum)
{
    ds_perspective_value_m[pnum] = 0;
};
#pragma endregion

void InitCamera()
{
    Camera_t = new Trampoline(0x438090, 0x438097, Camera_r);
    CameraPause_t = new Trampoline(0x4373D0, 0x4373D7, CameraPause_r);
    cameraDisplay_t = new Trampoline(0x4370F0, 0x4370F5, cameraDisplay_r);

    WriteJump((void*)0x434870, InitFreeCamera_r);
    WriteJump((void*)0x434880, ResetFreeCamera_r);
    WriteJump((void*)0x43656A, SetAdjustMode_r);
    WriteJump((void*)0x436590, GetAdjustMode_r);
    WriteJump((void*)0x4365A0, GetCameraMode_r);
    WriteJump((void*)0x4365B0, SetCameraMode_r);
    WriteJump((void*)0x4367A0, ChangeCamsetMode_r);
    WriteJump((void*)0x435C30, CamcontSetCameraCAMSTATUS_r);
    WriteJump((void*)0x435C70, CamcontSetCameraLOOKAT_r);
    WriteJump((void*)0x435D10, CamcontSetCameraTGTOFST_r);
    WriteJump((void*)0x434600, cameraModeInit_r);
    WriteJump((void*)0x434680, AddCameraStage_r);

    CameraMode[CAMMD_KLAMATH].fnCamera = CameraKlamath_m;
    CameraMode[CAMMD_A_KLAMATH].fnCamera = CameraKlamath_m;
    CameraMode[CAMMD_C_KLAMATH].fnCamera = CameraKlamath_m;
    CameraMode[CAMMD_RuinWaka1].fnCamera = CameraRuinWaka1_m;
    CameraAdjust[CAMADJ_NORMAL].fnAdjust = AdjustNormal_m;
    CameraAdjust[CAMADJ_NORMAL_S].fnAdjust = AdjustNormal_m;
    CameraAdjust[CAMADJ_FORFREECAMERA].fnAdjust = AdjustForFreeCamera_m;
    for (int i = CAMADJ_THREE1; i <= CAMADJ_RELATIVE6C; ++i)
        CameraAdjust[i].fnAdjust = AdjustThreePoint_m;
}

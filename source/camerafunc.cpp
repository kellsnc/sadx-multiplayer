#include "pch.h"
#include "SADXModLoader.h"
#include "sadx_utils.h"
#include "camera.h"
#include "camerafunc.h"

VariableHook<CAM_ANYPARAM, 0x3B2CA38> CamAnyParam_m;

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

CAM_ANYPARAM* GetCamAnyParam(int pnum)
{
    return &CamAnyParam_m[pnum];
}

// Make this use CamAnyParam as it should have...
void CameraRuinWaka1_m(_OBJ_CAMERAPARAM* pParam)
{
    auto param = GetCamAnyParam(TASKWK_PLAYERID(playertwp[0]));
    cameraControlWork.camxpos = param->camAnyParamPos.x;
    cameraControlWork.camypos = param->camAnyParamPos.y;
    cameraControlWork.camzpos = param->camAnyParamPos.z;
    cameraControlWork.tgtxpos = param->camAnyParamTgt.x;
    cameraControlWork.tgtypos = param->camAnyParamTgt.y;
    cameraControlWork.tgtzpos = param->camAnyParamTgt.z;
    cameraControlWork.angz = 0;
}

void AdjustNormal_m(taskwk* pTaskWork, taskwk* pOldTaskWork, _OBJ_ADJUSTPARAM* pCameraAdjustWork)
{
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

    if (cameraControlWork.ssFlag & 1)
    {
        cameraControlWork.ssFlag &= ~1;
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
            cameraControlWork.angy = pTaskWork->ang.y;
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
            cameraControlWork.angy = ang + pOldTaskWork->ang.y;
        }
    }

    Angle angx = SubAngle(pOldTaskWork->ang.x, pTaskWork->ang.x);

    if (angx & 0x8000)
    {
        angx |= 0xFFFF0000;
    }

    if (angx > -0x80 && angx < 0x80)
    {
        cameraControlWork.angx = pTaskWork->ang.x;
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
        cameraControlWork.angx = ang + pOldTaskWork->ang.x;
    }

    cameraControlWork.tgtxpos = ptwp->pos.x;
    cameraControlWork.tgtypos = ptwp->pos.y + 15.0f;
    cameraControlWork.tgtzpos = ptwp->pos.z;
    cameraControlWork.tgtdist = dist;
    cameraControlWork.angz = 0;
    CamcontSetCameraTGTOFST(pTaskWork);
}

void AdjustForFreeCamera_m(taskwk* pTaskWork, taskwk* pOldTaskWork, _OBJ_ADJUSTPARAM* pCameraAdjustWork)
{
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

    if (cameraControlWork.ssFlag & 1) // LR Flag carried on from some auto cameras
    {
        cameraControlWork.ssFlag &= ~1;
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
            cameraControlWork.angy = pTaskWork->ang.y;
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
            cameraControlWork.angy = spd + pOldTaskWork->ang.y;
        }
    }

    Angle angx = SubAngle(pOldTaskWork->ang.x, pTaskWork->ang.x);

    if (angx & 0x8000)
    {
        angx |= 0xFFFF0000;
    }

    if (angx > -0x200 && angx < 0x200)
    {
        cameraControlWork.angx = pTaskWork->ang.x;
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
        cameraControlWork.angx = spd + pOldTaskWork->ang.x;
    }

    cameraControlWork.tgtxpos = ptwp->pos.x;
    cameraControlWork.tgtypos = ppwp->p.center_height + ptwp->pos.y;
    cameraControlWork.tgtzpos = ptwp->pos.z;
    cameraControlWork.tgtdist = dist;
    cameraControlWork.angz = 0;
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

void AdjustThreePoint_m(taskwk* pTaskWork, taskwk* pOldTaskWork, _OBJ_ADJUSTPARAM* pCameraAdjustWork)
{
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

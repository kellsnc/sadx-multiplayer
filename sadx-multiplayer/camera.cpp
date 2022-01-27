#include "pch.h"
#include "splitscreen.h"
#include "camera.h"

Trampoline* CameraPause_t = nullptr;
Trampoline* CameraDisplay_t = nullptr;
Trampoline* Camera_t = nullptr;

struct MultiCam
{
    FCWRK wk;
    NJS_POINT3 pos;
    Angle3 ang;
    int timer;
    int mode;
};

MultiCam MultiCams[PLAYER_MAX];

NJS_VECTOR* GetCameraPosition(int pnum)
{
    if (SplitScreen::IsActive() && pnum < multiplayer::GetPlayerCount() && playertp[pnum])
    {
        return &MultiCams[pnum].pos;
    }
    else
    {
        return &camera_twp->pos;
    }
}

Angle3* GetCameraAngle(int pnum)
{
    if (SplitScreen::IsActive() && pnum < multiplayer::GetPlayerCount() && playertp[pnum])
    {
        return &MultiCams[pnum].ang;
    }
    else
    {
        return &camera_twp->ang;
    }
}

void ApplyMultiCamera(int pnum)
{
    if (!camera_twp || !playertwp[pnum] || pnum > multiplayer::GetPlayerCount())
    {
        return;
    }

    camera_twp->pos = MultiCams[pnum].pos;
    camera_twp->ang = MultiCams[pnum].ang;

    CameraSetView(camera_twp);
}

void __cdecl CameraDisplay_r(task* tp)
{
    if (SplitScreen::IsActive())
    {
        ApplyMultiCamera(SplitScreen::numScreen);
        //CameraFilter(tp); <- Works individually but crashes when draws more than once with DC Conv
    }
    else
    {
        TARGET_DYNAMIC(CameraDisplay)(tp);
    }
}

void __cdecl CameraPause_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        ApplyMultiCamera(SplitScreen::numScreen);
    }
    else
    {
        TARGET_DYNAMIC(CameraPause)(tp);
    }
}

void MultiCam_SetDistData(MultiCam* cam, taskwk* pltwp)
{
    if (CurrentLevel == LevelIDs_HotShelter)
    {
        cam->wk.dist1 = 15.0f;

        if (TASKWK_CHARID(pltwp) == 5)
        {
            cam->wk.dist2 = 30.0f;
        }
        else
        {
            cam->wk.dist2 = 50.0f;
        }
    }
    else
    {
        cam->wk.dist1 = 16.0f;

        if (CurrentLevel != LevelIDs_MysticRuins || CurrentAct != 2)
        {
            cam->wk.dist2 = 70.0f;
        }
        else
        {
            cam->wk.dist2 = 50.0f;
        }
    }
}

void MultiCam_CalcOrigPos(MultiCam* cam, taskwk* pltwp)
{
    NJS_VECTOR unitvector = { 0.0f, 0.0f, cam->wk.dist };
    Angle y = cam->wk._ang.y;
    Angle x = cam->wk._ang.x;

    njPushMatrix(_nj_unit_matrix_);

    if (y)
    {
        njRotateY(0, y);
    }

    if (x)
    {
        njRotateX(0, x);
    }

    njCalcPoint(0, &unitvector, &unitvector);
    njPopMatrixEx();

    cam->wk.pos.x = unitvector.x + pltwp->pos.x;
    cam->wk.pos.y = unitvector.y + pltwp->pos.y + 10.5f;
    cam->wk.pos.z = unitvector.z + pltwp->pos.z;
}

void RunMultiCamera(int num)
{
    auto cam = &MultiCams[num];
    auto pltwp = playertwp[num];
    auto plmwp = (motionwk2*)playermwp[num];
    auto plpwp = playerpwp[num];
    auto plper = &perG[num];

    if (multiplayer::IsFightMode() || CurrentLevel == LevelIDs_TwinkleCircuit)
    {
        cam->mode |= 0x80000008;

        // Force camera behind the player
        if (ControllerEnabled[num] == FALSE)
        {
            NJS_VECTOR dir = { -100, 0, 0 };
            PConvertVector_P2G(pltwp, &dir);
            cam->pos = pltwp->pos;
            njAddVector(&cam->pos, &dir);
        }
    }

    if (!pltwp)
    {
        return;
    }

    NJS_VECTOR vec;
    NJS_VECTOR dyncolpos;
    NJS_VECTOR freecampos;

    if (cam->wk.timer)
    {
        --cam->wk.timer;
    }
    else
    {
        cam->mode &= ~8u;
    }

    if ((cam->mode & 0x80000000))
    {
        MultiCam_SetDistData(cam, pltwp);
        vec.x = cam->pos.x - pltwp->pos.x;
        vec.y = cam->pos.y - pltwp->pos.y - 10.5;
        vec.z = cam->pos.z - pltwp->pos.z;
        float magnitude = fabsf(njScalor(&vec));
        njUnitVector(&vec);
        cam->wk._ang.y = (atan2(vec.x, vec.z) * 65536.0 * 0.1591549762031479);
        cam->wk._ang.x = (asin(vec.y) * 65536.0 * -0.1591549762031479);
        cam->wk._ang.z = 0;

        if (magnitude < cam->wk.dist1 || magnitude > cam->wk.dist2)
        {
            cam->wk.dist0 = cam->wk.dist1;
            cam->wk.dist = cam->wk.dist1;
        }
        else
        {
            cam->wk.dist0 = magnitude;
            cam->wk.dist = magnitude;
        }

        MultiCam_CalcOrigPos(cam, pltwp);
        cam->wk.campos = cam->wk.pos;
        cam->wk.counter = 0;
        cam->wk.pang.x = 0;
        cam->wk.pang.y = 0;
        cam->wk.pang.z = 0;
        cam->mode |= 0x40000000u;
    }

    if ((cam->mode & 0x40000000) != 0)
    {
        if ((cam->mode & 0x80000000) == 0)
        {
            MultiCam_SetDistData(cam, pltwp);
            cam->wk.counter = 0;
            cam->wk.dist0 = cam->wk.dist1;
            cam->wk.dist = cam->wk.dist1;
            MultiCam_CalcOrigPos(cam, pltwp);
            cam->wk.campos = cam->wk.pos;
        }

        int passes = 60;
        while (1)
        {
            cam->wk.dist0 = cam->wk.dist;
            cam->wk.dist *= 1.2f;

            if (cam->wk.dist >= cam->wk.dist2)
            {
                break;
            }

            MultiCam_CalcOrigPos(cam, pltwp);
            cam->wk.cammovepos = cam->wk.campos;
            cam->wk.camspd = { 0.0f, 0.0f, 0.0f };

            freecampos = cam->wk.campos;
            dyncolpos = { 0.0f, 0.0f, 0.0f };
            MSetPositionWIgnoreAttribute(&freecampos, &dyncolpos, 0, 0x400002 | ColFlags_Water, 10.0);

            cam->wk.campos.x = dyncolpos.x + freecampos.x;
            cam->wk.campos.y = dyncolpos.y + freecampos.y;
            cam->wk.campos.z = dyncolpos.z + freecampos.z;

            --passes;

            if (passes == 0 || cam->wk.dist0 == cam->wk.dist)
            {
                break;
            }

        }

        cam->mode &= ~0xC0000000;
    }

    if (cam->wk.pang.y > 0)
    {
        cam->wk.pang.y -= 32;

        if (cam->wk.pang.y < 0)
        {
            cam->wk.pang.y = 0;
        }
    }
    else if (cam->wk.pang.y < 0)
    {
        cam->wk.pang.y += 32;

        if (cam->wk.pang.y > 0)
        {
            cam->wk.pang.y = 0;
        }
    }

    if ((plper->x2 > 0 || (plper->r - 128) << 8 > 128) && cam->wk.pang.y < 256)
    {
        cam->wk.pang.y += 64;
    }
    else if ((plper->x2 < 0 || (plper->l - 128) << 8 > 128) && cam->wk.pang.y > -256)
    {
        cam->wk.pang.y -= 64;
    }

    cam->wk._ang.y += cam->wk.pang.y;

    if (cam->wk.pang.x > 0)
    {
        cam->wk.pang.x -= 32;

        if (cam->wk.pang.x < 0)
        {
            cam->wk.pang.x = 0;
        }
    }
    else if (cam->wk.pang.x < 0)
    {
        cam->wk.pang.x += 32;

        if (cam->wk.pang.x > 0)
        {
            cam->wk.pang.x = 0;
        }
    }

    if (plper->y2 > 0 && cam->wk.pang.x < 256)
    {
        cam->wk.pang.x += 64;
    }
    else if (plper->y2 < 0 && cam->wk.pang.x > -256)
    {
        cam->wk.pang.x -= 64;
    }

    cam->wk._ang.x += cam->wk.pang.x;

    if (cam->wk._ang.x >= -12288)
    {
        if (cam->wk._ang.x > -512)
        {
            cam->wk._ang.x = -513;
        }
    }
    else
    {
        cam->wk._ang.x = -12287;
    }

    cam->wk.dist = cam->wk.dist0;
   
    MultiCam_CalcOrigPos(cam, pltwp);
    cam->wk.cammovepos = cam->wk.pos;
    cam->wk.camspd.x = cam->wk.pos.x - cam->wk.campos.x;
    cam->wk.camspd.y = cam->wk.pos.y - cam->wk.campos.y;
    cam->wk.camspd.z = cam->wk.pos.z - cam->wk.campos.z;

    freecampos = cam->wk.campos;
    dyncolpos = cam->wk.camspd;
    int water_cdt = 0;

    if (MSetPositionWIgnoreAttribute(&freecampos, &dyncolpos, 0, 0x400002 | ColFlags_Water, 10.0))
    {
        freecampos = cam->wk.campos;
        dyncolpos = cam->wk.camspd;
        water_cdt = 1;

        if (MSetPositionWIgnoreAttribute(&freecampos, &dyncolpos, 0, 0x400002 | ColFlags_Water, 8.0))
        {
            water_cdt = 2;
        }
    }

    cam->wk.pos.x = dyncolpos.x + freecampos.x;
    cam->wk.pos.y = dyncolpos.y + freecampos.y;
    cam->wk.pos.z = dyncolpos.z + freecampos.z;
    cam->wk.campos = cam->wk.pos;
    vec.x = plmwp->spd.x;
    vec.y = plmwp->spd.y;
    vec.z = plmwp->spd.z;

    if (fabsf(vec.x) >= 1.0f)
    {
        vec.x *= 1.1f;
    }
    else
    {
        vec.x = 0.0f;
    }

    if (TASKWK_CHARID(pltwp) == Characters_Gamma && pltwp->mode == 31)
    {
        float y = fabsf(vec.y);
        if (y >= 2.0f && y <= 4.0f)
        {
            vec.y = 0.0f;
        }
    }

    if (fabsf(vec.y) >= 1.0f)
    {
        vec.y *= 1.1f;
    }
    else
    {
        vec.y = 0.0f;
    }

    if (fabsf(vec.z) >= 1.0f)
    {
        vec.z *= 1.1f;
    }
    else
    {
        vec.z = 0.0f;
    }

    vec.x = cam->wk.campos.x - pltwp->pos.x - vec.x;
    vec.y = cam->wk.campos.y - pltwp->pos.y - vec.y - 10.5f;
    vec.z = cam->wk.campos.z - pltwp->pos.z - vec.z;
    njUnitVector(&vec);

    cam->wk._ang.y = NJM_RAD_ANG(atan2f(vec.x, vec.z));
    cam->wk._ang.x = NJM_RAD_ANG(-asinf(vec.y));
    vec.x = cam->wk.campos.x - pltwp->pos.x;
    vec.y = cam->wk.campos.y - pltwp->pos.y - 10.5f;
    vec.z = cam->wk.campos.z - pltwp->pos.z;
    cam->wk.dist = fabsf(njScalor(&vec));

    if (cam->wk.dist <= cam->wk.dist2)
    {
        cam->wk.counter = 0;
    }
    else if (++cam->wk.counter > 20)
    {
        cam->mode |= 0x40000000u;
    }

    if (water_cdt)
    {
        if (cam->wk.dist <= cam->wk.dist1)
        {
            cam->wk.dist = cam->wk.dist1;
        }
    }
    else
    {
        cam->wk.dist *= 1.2f;

        if (cam->wk.dist >= cam->wk.dist2)
        {
            cam->wk.dist = cam->wk.dist2;
        }
    }

    cam->wk.dist0 = cam->wk.dist;

    if (!(plpwp->item & Powerups_Dead))
    {
        cam->pos = cam->wk.campos;
    }

    cam->ang = cam->wk._ang;
}

void __cdecl Camera_r(task* tp)
{
    // If multiplayer is enabled, run custom cameras
    if (SplitScreen::IsActive())
    {
        auto twp = tp->twp;

        if (twp->mode == 0)
        {
            twp->mode = 1;
            camera_twp = twp;
            tp->disp = CameraDisplay;

            // Initialize all cameras
            for (auto cam : MultiCams)
            {
                cam.wk.timer = 60;
                cam.mode = 0x8000000C;
                cam.wk = {};
                cam.pos = { 0.0f, 0.0f, 0.0f };
                cam.ang = { 0, 0, 0 };
                cam.mode = 0;
            }

            cameraready = FALSE;
        }
        else
        {
            for (int i = 0; i < multiplayer::GetPlayerCount(); ++i)
            {
                RunMultiCamera(i);
            }

            cameraready = TRUE;
            tp->disp(tp);
        }
    }
    else
    {
        TARGET_DYNAMIC(Camera)(tp);
    }
}

void __cdecl InitFreeCamera_r()
{
    if (SplitScreen::IsActive())
    {
        for (auto& cam : MultiCams)
        {
            cam.mode |= 0x8000000C;
        }
    }

    fcwrk.timer = 60;
    free_camera_mode |= 0x8000000C;
}

void __cdecl ResetFreeCamera_r()
{
    if (SplitScreen::IsActive())
    {
        for (auto& cam : MultiCams)
        {
            cam.mode |= 0x80000008;
        }
    }

    fcwrk.timer = 60;
    free_camera_mode |= 0x80000008;
}

void InitCamera()
{
    Camera_t = new Trampoline(0x438090, 0x438097, Camera_r);
    CameraPause_t = new Trampoline(0x4373D0, 0x4373D7, CameraPause_r);
    CameraDisplay_t = new Trampoline(0x4370F0, 0x4370F5, CameraDisplay_r);
    
    WriteJump((void*)0x434870, InitFreeCamera_r);
    WriteJump((void*)0x434880, ResetFreeCamera_r);
}
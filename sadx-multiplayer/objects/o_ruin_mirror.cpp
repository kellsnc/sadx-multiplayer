#include "pch.h"
#include "multiplayer.h"
#include "camera.h"

#define MIRROR_PNUM(twp) twp->btimer

enum : __int8
{
    MD_INIT,
    MD_1,
    MD_2,
    MD_3
};

DataPointer(CCL_INFO, c_colli_mirror, 0x2038C38);
DataPointer(NJS_POINT3, aim_position, 0x2038C28);
DataPointer(unsigned __int8, name_flag, 0x3C7512A);
DataPointer(unsigned __int8, ruin_m_flag, 0x3C75128);
DataPointer(unsigned __int8, other_flag, 0x3C7512B);
DataPointer(unsigned __int8, discovery, 0x3C75129);
DataPointer(int, nocontimer, 0x3C7512C);
DataPointer(float, max_dist, 0x2038CA0);

static void ObjectRuinMirror_m(task* tp)
{
    if (!CheckRangeOutWithR(tp, 25000000.0f))
    {
        auto twp = tp->twp;
        auto pnum = MIRROR_PNUM(twp);

        switch (twp->mode)
        {
        case MD_INIT:
            CCL_Init(tp, &c_colli_mirror, 1, 4u);
            tp->disp = (TaskFuncPtr)0x5E2380;
            twp->timer.w[0] = 0i16;
            twp->timer.w[1] = 0i16;
            twp->mode = MD_1;
            break;
        case MD_1:
            if (twp->btimer == 2i8)
            {
                twp->timer.w[1] = 1i16;
            }

            if (ruin_m_flag == 1ui8)
            {
                auto aim_id = static_cast<unsigned __int8>(twp->scl.y);

                if (other_flag == aim_id)
                {
                    twp->btimer = 2;
                    aim_position.x = twp->pos.x;
                    aim_position.y = twp->pos.y;
                    aim_position.z = twp->pos.z;
                }

                if (name_flag == aim_id && twp->timer.w[1] == 1i16)
                {
                    NJS_POINT3 velo = { 0.0f, 0.0f, 0.0f };
                    CreateSmoke(&twp->pos, &velo, 2.0f);
                }
            }

            if (twp->cwp->flag & 1)
            {
                twp->cwp->flag &= ~1;

                if (twp->smode != 1 && twp->cwp->hit_cwp->mytask)
                {
                    auto hit_tp = twp->cwp->hit_cwp->mytask;

                    if (hit_tp->twp)
                    {
                        pnum = TASKWK_PLAYERID(hit_tp->twp);
                        twp->mode = MD_2;

                        SetInputP(pnum, PL_OP_PLACEON);

                        //CamAnyParam.camAnyParamPos.x = twp->pos.x;
                        //CamAnyParam.camAnyParamPos.y = twp->pos.y + 10.0;
                        //CamAnyParam.camAnyParamPos.z = twp->pos.z;
                        //CamAnyParam.camAnyParamAng.x = 0;
                        //CamAnyParam.camAnyParamAng.y = twp->ang.y;
                        //CamAnyParam.camAnyParamAng.z = 0;
                        //SetFreeCameraMode(0);
                        // CameraSetEventCamera(65, 0);

                        PClearSpeed(playermwp[pnum], playerpwp[pnum]);
                        MIRROR_PNUM(twp) = pnum;
                    }
                }
            }
            else
            {
                EntryColliList(twp);
            }
            break;
        case MD_2:
            ruin_m_flag = 1ui8;
            name_flag = static_cast<unsigned __int8>(twp->scl.x);
            SetPositionP(pnum, twp->pos.x + njCos(twp->ang.y) * 10.0f, twp->pos.y, twp->pos.z - njSin(twp->ang.y) * 10.0f);

            {
                auto cam_rot = GetCameraAngle(pnum);
                twp->ang.y = cam_rot ? cam_rot->y + 0x8000 : 0;
            }

            if (discovery == 1ui8)
            {
                twp->smode = 1i8;
                twp->mode = MD_1;
                nocontimer = 120;
                ruin_m_flag = 0ui8;
                discovery = 0ui8;

                if (twp->scl.x == 5.0f)
                {
                    max_dist = -175.0f;
                }

                //CameraReleaseEventCamera();
                dsStop_num(197);
                SetInputP(pnum, PL_OP_LETITGO);
            }
            break;
        case MD_3:
            if (++twp->timer.w[0] > 20)
            {
                twp->timer.w[0] = 0i16;
                NJS_POINT3 velo = { 0.0f, 0.0f, 0.0f };
                CreateSmoke(&twp->pos, &velo, 2.0f);
            }
            break;
        }

        tp->disp(tp);
    }
}

static void __cdecl ObjectRuinMirror_r(task* tp);
Trampoline ObjectRuinMirror_t(0x5E2850, 0x5E2855, ObjectRuinMirror_r);
static void __cdecl ObjectRuinMirror_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        ObjectRuinMirror_m(tp);
    }
    else
    {
        TARGET_STATIC(ObjectRuinMirror)(tp);
    }
}
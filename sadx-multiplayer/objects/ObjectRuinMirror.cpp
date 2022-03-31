#include "pch.h"

TaskFunc(ORmirror, 0x5E2850);
DataPointer(CCL_INFO, c_colli_mirror, 0x02038C38);
DataPointer(char, ruin_m_flag, 0x03C75128);
DataPointer(char, other_flag, 0x03C7512B);
DataPointer(NJS_VECTOR, MirrorAim_position, 0x02038C28);
DataPointer(char, name_flag, 0x03C7512A);
DataPointer(int, nocontimer, 0x03C7512C);
DataPointer(char, discovery, 0x03C75129);

void __cdecl ORmirror_Main_r(task* task)
{
    colliwk* colInfo; 
    unsigned __int16 flag; 
    _BOOL1 timer; 
    float cosPosX; 
    float sinPosZ;
    NJS_VECTOR smokeRadius; 
    char pnum;

   taskwk* data = task->twp;

    if (!ClipObject((ObjectMaster*)task, 25000000.0))
    {
        switch (data->mode)
        {
        case 0:
            CCL_Init(task, &c_colli_mirror, 1, 4u);
            task->disp = (TaskFuncPtr)ORmirror_Display;
            data->timer.w[0] = 0;
            data->timer.w[1] = 0;
            data->mode = 1;
            break;
        case 1:
            if (data->btimer == 2)
            {
                data->timer.w[1] = 1;
            }

            if (ruin_m_flag == 1)
            {
                if ((double)(unsigned __int8)other_flag == data->scl.y)
                {
                    data->btimer = 2;
                    MirrorAim_position.x = data->pos.x;
                    MirrorAim_position.y = data->pos.y;
                    MirrorAim_position.z = data->pos.z;
                }

                if ((double)(unsigned __int8)name_flag == data->scl.y && data->timer.w[1] == 1)
                {
                    smokeRadius.x = 0.0;
                    smokeRadius.y = 0.0;
                    smokeRadius.z = 0.0;
                    CreateSmoke(&data->pos, &smokeRadius, 2.0);
                }
            }
            ORmirror_Display((ObjectMaster*)task);
            colInfo = data->cwp;
            flag = colInfo->flag;

            if ((flag & 1) != 0)
            {
                colInfo->flag = flag & 0xFFFE;

                if (data->smode != 1 && data->cwp->hit_cwp->mytask)
                {
                    if (data->cwp->hit_cwp->mytask->twp) {

                        pnum = data->cwp->hit_cwp->mytask->twp->counter.b[0];
                        data->mode = 2;
                        ORmirror_PositionThing.x = data->pos.x;
                        ORmirror_PositionThing.y = data->pos.y + 10.0;
                        ORmirror_PositionThing.z = data->pos.z;
                        ORmirror_RotationThing.x = 0;
                        ORmirror_RotationThing.y = data->ang.y;
                        ORmirror_RotationThing.z = 0;
                        ForcePlayerAction(pnum, 12);
                        SetCameraControlEnabled(0);
                       // CameraSetEventCamera(65, 0);
                        PClearSpeed(playermwp[pnum], playerpwp[pnum]);
                    }
                }
            }
            else
            {
                EntryColliList(data);
            }
            break;
        case 2:
            pnum = data->cwp->hit_cwp->mytask->twp->counter.b[0];
            ruin_m_flag = 1;
            ORmirror_Display((ObjectMaster*)task);
            name_flag = (unsigned __int64)data->scl.x;
            sinPosZ = data->pos.z - njSin(data->ang.y) * 10.0;
            cosPosX = njCos(data->ang.y) * 10.0 + data->pos.x;
            PositionPlayer(pnum, cosPosX, data->pos.y, sinPosZ);
            data->ang.y = Camera_Data1->Rotation.y + 0x8000;
            if (discovery == 1)
            {
 
                data->smode = 1;
                nocontimer = 120;
                ruin_m_flag = 0;
                data->mode = 1;
                discovery = 0;

                if (data->scl.x == 5.0f)
                {
                    LostWorldFogTarget = -175.0f;
                }

                CameraReleaseEventCamera();
                dsStop_num(197);
                SetInputP(pnum, 24);
            }
            break;
        case 3:
            timer = ++data->timer.w[0] <= 20;
            smokeRadius.x = 0.0;
            smokeRadius.y = 0.0;
            smokeRadius.z = 0.0;

            if (!timer)
            {
                data->timer.w[0] = 0;
                CreateSmoke(&data->pos, &smokeRadius, 2.0);
            }

            ORmirror_Display((ObjectMaster*)task);
            break;
        default:
            break;
        }

        if (!MissedFrames)
        {
            if (ObjectSelectedDebug((ObjectMaster*)task))
            {
                DisplayDebugString(1114131, "<-MY ID");
                DisplayDebugString(1114132, "<-AIM ID");
            }
        }
    }
}


static void __cdecl ORmirror_r(task* tp);
Trampoline ORmirror_t(0x5E2850, 0x5E2855, ORmirror_r);
static void __cdecl ORmirror_r(task* tp)
{

    if (multiplayer::IsActive())
    {
        ORmirror_Main_r(tp);
    }
    else
    {
        TARGET_STATIC(ORmirror)(tp);
    }
}
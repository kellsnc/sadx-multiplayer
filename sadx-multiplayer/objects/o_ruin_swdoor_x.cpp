#include "pch.h"


void SwDoorX_Main_r(task* tp);
Trampoline ObjSwDoorX_Main_t(0x5E7660, 0x5E7668, SwDoorX_Main_r);
void SwDoorX_Main_r(task* tp)
{
    taskwk* data = tp->twp;
    NJS_VECTOR a3;
    char pnum;

    if ( (CheckRangeOut(tp) ||ObjectSelectedDebug((ObjectMaster*)tp)))
        return;

    if (!data->mode)
    {
        if (CheckCollisionCylinderP(&data->pos, 26.0, 3.0))
        {
            pnum = GetClosestPlayerNum(&data->pos);

 
            GetBufferedPositionAndRotation(pnum, 0, &a3, 0);
            if ((a3.z - data->pos.z) * data->scl.z + (a3.x - data->pos.x) * data->scl.x > 0.0)
            {
                data->mode = 2;
                data->wtimer = 0;
                data->counter.l = 0;
                data->value.l = 1;
                PlaySound2(194, data, 1, 0);
            }
        }

        EntryColliList(data);
        tp->disp(tp);
    }
    else
    {
        TaskFunc(origin, ObjSwDoorX_Main_t.Target());
        origin(tp);
    }
}

void Init_SwDoorX(task* tp)
{
    if (multiplayer::IsActive())
    {
        SwDoorX_Main_r(tp);
    }
    else
    {
        TaskFunc(origin, ObjSwDoorX_Main_t.Target());
        origin(tp);
    }
}

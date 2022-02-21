#include "pch.h"
#include "UsercallFunctionHandler.h"

//imagine making array of 1
DataArray(CCL_INFO, palley_colli, 0x24C8C70, 1);
TaskFunc(PathKassha, 0x603640);

auto SetPath2Taskwk_0 = GenerateUsercallWrapper<void (*)(pathtag* a1, taskwk* a2, float a3)>(noret, 0x602A50, rEDX, rESI, noret);
auto DrawHuck = GenerateUsercallWrapper<void (*)(task* a1)>(noret, 0x602B10, rEAX);
auto DrawWireTarumi = GenerateUsercallWrapper<void (*)(task* a1, int a2, char a3)>(noret, 0x603330, rEAX, rECX, noret);
auto DrawWire = GenerateUsercallWrapper<void (*)(task* a1)>(noret, 0x602DF0, rEAX);
 
void PathKassha_r(task* tp);
Trampoline PathKassha_t(0x603640, 0x603647, PathKassha_r);
void PathKassha_r(task* tp)
{

    if (!multiplayer::IsActive())
    {
        return TARGET_STATIC(PathKassha)(tp);
    }

    taskwk* data = tp->twp;
    pathtag* path;
    NJS_VECTOR pos;
    float result;
    int v29[4];
    float resultScaleZ;
    float cosRes;
    float sinRes;

    int lvlActThing = ssStageNumber << 8;

    if ((lvlActThing | ssActNumber) != data->timer.w[0])
    {
        FreeTask(tp);
        return;
    }

    unsigned __int8 mode = data->mode;
    char pnum;

    switch (mode)
    {
    case 0:
        tp->disp = (TaskFuncPtr)0x603590;

        CCL_Init(tp, palley_colli, 1, 4u);
        path = (pathtag*)data->value.l;
        SetPath2Taskwk_0(path, data, data->counter.f);
        data->wtimer = 0;
        data->mode++;
        data->counter.f = 0.0;
        data->scl.z = path->totallen;
        break;
    case 1:
        if ((data->cwp->flag & 1) != 0)
        {
            auto ptwp = CCL_IsHitPlayer(data);

            if (!ptwp)
            {
                ptwp = playertwp[GetTheNearestPlayerNumber(&data->pos)];
            }

            data->btimer = TASKWK_PLAYERID(ptwp);

            SetInputP(data->btimer, 16);
            data->mode++;
        }
        EntryColliList(data);

        pnum = data->btimer;
        pos = { data->pos.x - playertwp[pnum]->pos.x, data->pos.y - playertwp[pnum]->pos.y, data->pos.z - playertwp[pnum]->pos.z };
        result = pos.x * pos.x + pos.y * pos.y + pos.z * pos.z;
        if (squareroot(result) < 100.0f)
        {
            path = (pathtag*)data->value.l;
            SetPath2Taskwk_0(path, data, 0.0f);
            DrawHuck(tp);
            SCPathOnposToPntnmb(path, data->counter.f, v29);
            DrawWireTarumi(tp, v29[0], 0);
            DrawWireTarumi(tp, v29[0] +1, 0);
            return;
        }

        break;
    case 2:
        pnum = data->btimer;
        SCPathOnposToPntnmb((pathtag*)data->value.l, data->counter.f, v29);
        DrawWireTarumi(tp, v29[0] + 1, 0);
        DrawWireTarumi(tp, v29[0] + 2, 0);
        DrawWire(tp);
        resultScaleZ = (float)(data->counter.f + (float)8.0f);
        data->counter.f += 8.0f;
        if (resultScaleZ > data->scl.z)
        {
            data->mode++;
            SetInputP(pnum, 24);
            cosRes = njCos(-playertwp[pnum]->ang.y);
            sinRes = njSin(-playertwp[pnum]->ang.y);
            SetVelocityP(pnum, cosRes * 2.2f, 1.0f, sinRes);
            SCPathOnposToPntnmb((pathtag*)data->value.l, data->scl.z, v29);
            data->btimer = v29[0];
        }
        QueueSound_DualEntity(132, data, 1, 0, 10);
        return;
    case 3:
        pnum = data->btimer;
        pos = { data->pos.x - playertwp[pnum]->pos.x, data->pos.y - playertwp[pnum]->pos.y, data->pos.z - playertwp[pnum]->pos.z };
        result = pos.x * pos.x + pos.y * pos.y + pos.z * pos.z;
        if (squareroot(result) < 100.0f)
        {
            DrawWireTarumi(tp, (unsigned __int8)data->btimer, 1);
        }
        break;
    default:
        return;
    }
}
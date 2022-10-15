#include "pch.h"

UsercallFunc(signed int, Amy_CheckInput_t, (playerwk* a1, motionwk2* a2, taskwk* a3), (a1, a2, a3), 0x487810, rEAX, rECX, rEDI, rESI);
TaskHook AmyExec_t((intptr_t)Amy_Main);
static FunctionHook<void, taskwk*, motionwk2*, playerwk*> Amy_RunsActions_t((intptr_t)0x488880);

signed int Amy_CheckInput_r(playerwk* co2, motionwk2* data2, taskwk* data)
{
    auto even = data->ewp;

    if (even->move.mode || even->path.list || ((data->flag & Status_DoNextAction) == 0))
    {
        return Amy_CheckInput_t.Original(co2, data2, data);
    }

    switch (data->smode)
    {
    case 5:
        data->mode = SDCannonMode;
        co2->mj.reqaction = 18; //falling
        return 1;
    }

    return Amy_CheckInput_t.Original(co2, data2, data);
}

void Amy_RunsActions_r(taskwk* data1, motionwk2* data2, playerwk* co2) {

    switch (data1->mode)
    {
    case SDCannonMode:
        if (!AmyCheckInput(co2, data2, data1) && (data1->flag & 3) != 0)
        {
            if (PCheckBreak(data1) && co2->spd.x > 0.0f)
            {
                data1->mode = 9;
            }
            if (!AmyCheckStop(data1, co2))
            {
                data1->mode = 2;
            }

            data1->ang.x = data2->ang_aim.x;
            data1->ang.z = data2->ang_aim.z;
            co2->mj.reqaction = 2;
        }
        return;
    }

    Amy_RunsActions_t.Original(data1, data2, co2);
}

void AmyExec_r(task* obj)
{
    auto data = obj->twp;
    motionwk2* data2 = (motionwk2*)obj->mwp;
    playerwk* co2 = (playerwk*)obj->mwp->work.l;

    switch (data->mode)
    {
    case SDCannonMode:
        CannonModePhysics(data, data2, co2);
        break;
    }

    AmyExec_t.Original(obj);
}

void Init_AmyPatches()
{
    AmyExec_t.Hook(AmyExec_r);
    Amy_CheckInput_t.Hook(Amy_CheckInput_r);
    Amy_RunsActions_t.Hook(Amy_RunsActions_r);
}
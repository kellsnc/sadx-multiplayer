#include "pch.h"

UsercallFunc(signed int, Knux_CheckNact_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x476970, rEAX, rEDI, rESI, stack4);
TaskHook KnuxExec_t((intptr_t)Knuckles_Main);
static FunctionHook<void, taskwk*, motionwk2*, playerwk*> Knux_RunsActions_t(Knux_RunsActions);

signed int Knux_CheckNAct_r(playerwk* co2, taskwk* data, motionwk2* data2)
{
    auto even = data->ewp;

    if (even->move.mode || even->path.list || ( (data->flag & Status_DoNextAction) == 0))
    {
        return Knux_CheckNact_t.Original(co2, data, data2);
    }

    switch (data->smode)
    {
    case 5:
       data->mode = 101;
       co2->mj.reqaction = 19;
       return 1;
    }

    return Knux_CheckNact_t.Original(co2, data, data2);
}

void Knux_RunsActions_r(taskwk* data1, motionwk2* data2, playerwk* co2) {

    switch (data1->mode)
    {
    case 101:
        if (!KnucklesCheckInput(data1, data2, co2) && (data1->flag & 3) != 0)
        {
            if (PCheckBreak(data1) && co2->spd.x > 0.0f)
            {
                data1->mode = 10;
            }
            if (!KnucklesCheckStop(data1, co2))
            {
                data1->mode = 2;
            }

            data1->ang.x = data2->ang_aim.x;
            data1->ang.z = data2->ang_aim.z;
            co2->mj.reqaction = 2;
        }
        return;
    }

    Knux_RunsActions_t.Original(data1, data2, co2);
}

void KnuxExec_r(task* obj)
{ 
    auto data = obj->twp;
    motionwk2* data2 = (motionwk2*)obj->mwp;
    playerwk* co2 = (playerwk*)obj->mwp->work.l;
    chaoswk* cwk = (chaoswk*)obj->awp;

    switch (data->mode)
    {
    case 101:
        PGetGravity(data, data2, co2);
        PGetSpeed(data, data2, co2);
        PSetPosition(data, data2, co2);
        PResetPosition(data, data2, co2);
        break;
    }

    KnuxExec_t.Original(obj);
}

void Init_KnuxPatches()
{
    KnuxExec_t.Hook(KnuxExec_r);
    Knux_CheckNact_t.Hook(Knux_CheckNAct_r);
    Knux_RunsActions_t.Hook(Knux_RunsActions_r);
}
#include "pch.h"
#include "deathzones.h"

void __cdecl sub_440C80_r(task* obj)
{
    anywk* dz; // eax
    task* parent; // eax

    dz = obj->awp;
    if (dz->work.ul[0]++ > 0x78)
    {
        if (GetDebugMode())
        {
            CheckThingButThenDeleteObject((ObjectMaster*)obj);
        }
    }
    else
    {

        StartLevelCutscene(2);
        parent = obj->ptp;
        if (parent)
        {
            parent->twp->flag |= 4u;
        }
        FreeQueueSound();
        CheckThingButThenDeleteObject((ObjectMaster*)obj);
    }
}


void __cdecl KillPlayer_r(unsigned __int8 pID)
{
    EntityData1* data; // esi

    data = EntityData1Ptrs[pID];
    if (!GetDebugMode())
    {
        data->Status | 0x1000;
        data->NextAction = 50;
        CharObj2Ptrs[pID]->Powerups |= 0x4000u;
        task* dz = CreateElementalTask(8u, 0, sub_440C80_r);
        dz->twp->scl.y = pID;
    }
}

void init_DeathPatches() {
    WriteJump((void*)0x440CD0, KillPlayer_r);

}
#include "pch.h"

VoidFunc(sub_464DF0, 0x464DF0);

void __cdecl sub_440C80_r(task* obj)
{
    anywk* dz; // eax
    task* parent; // eax

    char pNum = obj->twp->scl.y;
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
        if (pNum <= 0)
            StartLevelCutscene(2);

        parent = obj->ptp;
        if (parent)
        {
            parent->twp->flag |= 4u;
        }

        EntityData1* player = EntityData1Ptrs[pNum];
        SetLives_r(pNum, -1);
        MovePlayerToStartPoint(player);
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

void PlayCharacterDeathSound_r(task* obj, int pNum)
{
    int pNumCopy; // esi
    EntityData1* data1; // edi

    pNumCopy = (unsigned __int8)pNum;
    EntityData1* player = EntityData1Ptrs[pNumCopy];
    CharObj2* co2 = CharObj2Ptrs[pNumCopy];

    CreateChildTask(LoadObj_UnknownB, sub_440C80_r, obj);

    data1 = EntityData1Ptrs[pNumCopy];

    CameraSetEventCameraFunc((CamFuncPtr)sub_464DF0, 0, 0);

    if (!co2 || (co2->Powerups & 0x4000) == 0)
    {
        switch (pNumCopy)
        {
        case Characters_Sonic:
            if (MetalSonicFlag)
            {
                PlayVoice(2046);
            }
            else
            {
                PlaySound(1503, 0, 0, 0);
            }
            break;
        case Characters_Tails:
            PlaySound(1465, 0, 0, 0);

            if (player)
            {
                player->Status |= 0x10u;
                player->NextAction = 24;
            }
            break;
        case Characters_Knuckles:
            PlaySound(1453, 0, 0, 0);

            if (player)
            {
                player->Status |= 0x10u;
                player->NextAction = 24;
            }
            break;
        case Characters_Amy:
            PlaySound(1396, 0, 0, 0);
            break;
        case Characters_Gamma:
            PlaySound(1433, 0, 0, 0);
            break;
        case Characters_Big:
            PlaySound(1412, 0, 0, 0);
            break;
        default:
            return;
        }
    }
}

void init_DeathPatches() {
    WriteJump((void*)0x440CD0, KillPlayer_r);


}
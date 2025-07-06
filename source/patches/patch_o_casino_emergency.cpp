#include "pch.h"

FastFunctionHook<void, task*> ObjectEmergencyPExec_h(0x5D50C0);

static void ObjectEmergencyPExec(task* tp)
{
    if (multiplayer::IsActive() == false)
    {
        return ObjectEmergencyPExec_h.Original(tp);
    }

    taskwk* twp = tp->twp;


    if (!(twp->cwp->flag & CWK_FLAG_HIT))
    {
        //Not being touched so reset it.
        twp->counter.l = 0;
    }
    else if (twp->counter.f >= twp->scl.z)
    {
        //Touched and ready to bounce the player (Counter is higher than Interval)
        NJS_POINT3 v = { twp->scl.y, 0.0f, 0.0f };
        Angle3 ang = { 0, twp->ang.y, 0 };
        auto player = CCL_IsHitPlayer(twp);
        if (player)
        {
            auto pnum = player->counter.b[0];
            SetVelocityAndRotationAndNoconTimeP(pnum, &v, &ang, 0);
            twp->counter.l = 0;
        }
    }
    else {
        //Touched but not ready yet, increment counter.
        ++twp->counter.f;
    }
    EntryColliList(twp);
    ObjectSetupInput(twp, 0);

    //Visible in Edit Mode/Debug
    if (OnEdit(tp))
    {
        ObjectEmergencyPExec_h.Original(tp);
    }
}

void patch_emergency_init()
{
    ObjectEmergencyPExec_h.Hook(ObjectEmergencyPExec);
}

RegisterPatch patch_emergency(patch_emergency_init);
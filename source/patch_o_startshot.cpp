#include "pch.h"

FastFunctionHook<void, task*> ObjectStartShotNormal_h(0x5DCC20);
#define PINBALL_FLAG_STOPER_CAM		0x100
DataPointer(int, PinballFlag, 0x3C748F8);

static void Normal_r(task* tp)
{
    if (multiplayer::IsActive() == false)
    {
        return ObjectStartShotNormal_h.Original(tp);
    }

    taskwk* twp = tp->twp;

    auto pnum = GetTheNearestPlayerNumber(&twp->pos);
    auto pwp = playerpwp[pnum];


    if (twp->cwp->flag & CWK_FLAG_HIT)
    {
        NJS_VECTOR v = { njRandom() * twp->scl.y + twp->scl.z, 0.0f, 0.0f };
        Angle3 ang = { 0, 0x8000, 0 };
        SetVelocityAndRotationAndNoconTimeP(pnum, &v, &ang, 0);
        dsPlay_oneshot(SE_CA_SHOT, 0, 0, 0);
        VibShot(pnum, 0);
        PinballFlag &= ~PINBALL_FLAG_STOPER_CAM;
    }
    EntryColliList(twp);
    ObjectSetupInput(twp, 0);
}

void patch_startshot_init()
{
    ObjectStartShotNormal_h.Hook(Normal_r);
}

RegisterPatch patch_startshot(patch_startshot_init);
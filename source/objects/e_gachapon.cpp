#include "pch.h"
#include "SADXModLoader.h"
#include "UsercallFunctionHandler.h"
#include "FunctionHook.h"

// Crabs in Final Egg 3

UsercallFuncVoid(chk_mode_Hook, (task* tp, taskwk* twp, gachamotionwk* mwp), (tp, twp, mwp), 0x5B00A0, stack4, rECX, rEAX);
UsercallFuncVoid(GachaCheckColli_Hook, (taskwk* twp, gachamotionwk* mwp), (twp, mwp), 0x5AFA20, rESI, rEDI);

void chk_mode_r(task* tp, taskwk* twp, gachamotionwk* mwp)
{
    if ((twp->flag & 4) && twp->mode != 11 && twp->mode != 7)
    {
        auto pltwp = CCL_IsHitPlayerWithNum(twp, 1);
        if (pltwp)
        {
            twp->mode = 7;
            twp->wtimer = 30;
            NJS_POINT3 p = twp->pos;
            njAddVector(&p, &pltwp->pos);
            p.x *= 0.5f;
            p.y *= 0.5f;
            p.z *= 0.5f;
            CreateHitmark(&p, 0.5f);
            twp->scl.z = 0.35f;
            SetVelocityP(TASKWK_PLAYERID(pltwp), 0.0f, 1.2f, 0.0f);
        }
    }

    chk_mode_Hook.Original(tp, twp, mwp);
}

void GachaCheckColli_r(taskwk* twp, gachamotionwk* mwp)
{
    if (multiplayer::IsActive())
    {
        twp->flag &= ~0x1;

        if (twp->pos.y < mwp->height + 0.1f)
        {
            twp->pos.y = mwp->height;
            twp->flag |= 1;
        }

        auto pnum = GetClosestPlayerNum(&twp->pos);
        auto ptwp = playertwp[pnum];

        if (ptwp)
        {
            mwp->playerang = -0x4000 - -njArcTan(ptwp->pos.x - twp->pos.x, ptwp->pos.z - twp->pos.z);
            mwp->playerangdiff = DiffAngle(mwp->playerang, twp->ang.y);
        }

    }
    else
    {
        GachaCheckColli_Hook.Original(twp, mwp);
    }
}

void PatchGachapon()
{
    chk_mode_Hook.Hook(chk_mode_r);
    GachaCheckColli_Hook.Hook(GachaCheckColli_r);
}

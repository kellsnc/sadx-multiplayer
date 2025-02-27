#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"

// Crabs in Final Egg 3

FastUsercallHookPtr<void(*)(taskwk* twp, gachamotionwk* mwp, task* tp), noret, rECX, rEAX, stack4> chk_mode_Hook(0x5B00A0);
FastUsercallHookPtr<void(*)(taskwk* twp, gachamotionwk* mwp), noret, rESI, rEDI> GachaCheckColli_Hook(0x5AFA20);

void chk_mode_r(taskwk* twp, gachamotionwk* mwp, task* tp)
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

	chk_mode_Hook.Original(twp, mwp, tp);
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

void patch_gachapon_init()
{
	chk_mode_Hook.Hook(chk_mode_r);
	GachaCheckColli_Hook.Hook(GachaCheckColli_r);
}

RegisterPatch patch_gachapon(patch_gachapon_init);
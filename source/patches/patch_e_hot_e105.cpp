#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "result.h"

FastUsercallHookPtr<void(*)(task*), noret, rEAX> e105_calcSomeValue_t(0x5A3860);
FastFunctionHook<void, task*> e105_moveBattery_t(0x5A40B0);
FastFunctionHook<Bool, task*> e105_chkPlayerRangeIn_t(0x5A3670);

// Unknown main struct so no full rewrite for now

static void __cdecl e105_calcSomeValue_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto pltp = playertp[0];
		auto plmwp = playermwp[0];
		auto pnum = GetClosestPlayerNum(&tp->twp->pos);
		playertp[0] = playertp[pnum];
		playermwp[0] = playermwp[pnum];
		e105_calcSomeValue_t.Original(tp);
		playertp[0] = pltp;
		playermwp[0] = plmwp;
	}
	else
	{
		e105_calcSomeValue_t.Original(tp);
	}
}

static void __cdecl e105_moveBattery_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (twp->mode == 1)
		{
			auto hit_twp = CCL_IsHitBullet(twp);

			if (hit_twp)
			{
				SetWinnerMulti(hit_twp->btimer); // player number is stored in btimer thanks to patch in E102.cpp
			}
		}

		auto pltp = playertp[0];
		playertp[0] = playertp[GetClosestPlayerNum(&tp->twp->pos)];
		e105_moveBattery_t.Original(tp);
		playertp[0] = pltp;
	}
	else
	{
		e105_moveBattery_t.Original(tp);
	}
}

static Bool __cdecl e105_chkPlayerRangeIn_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (IsPlayerInSphere(&tp->twp->pos, 150.0f))
		{
			twp->smode = 2;
			ccsi_flag = 1ui8;
		}

		return FALSE;
	}
	else
	{
		return e105_chkPlayerRangeIn_t.Original(tp);
	}
}

void patch_hot_e105_init()
{
	e105_calcSomeValue_t.Hook(e105_calcSomeValue_r);
	e105_moveBattery_t.Hook(e105_moveBattery_r);
	e105_chkPlayerRangeIn_t.Hook(e105_chkPlayerRangeIn_r);
}

RegisterPatch patch_hot_e105(patch_hot_e105_init);
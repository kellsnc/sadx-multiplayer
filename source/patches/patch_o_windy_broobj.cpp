#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "utils.h"

static void __cdecl ObjectWindyBrokenObj_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectWindyBrokenObj_Exec_r)> ObjectWindyBrokenObj_Exec_t(0x4E2970);

static void Exec_m(task* tp)
{
	auto twp = tp->twp;

	if (CheckRangeOut(tp))
	{
		return;
	}

	twp->timer.f += twp->scl.x;

	if (twp->mode == 0i8)
	{
		if (EnemyCheckDamage(twp, (enemywk*)tp->mwp))
		{
			dsPlay_oneshot(64, 0, 0, 0);
			NJS_POINT3 pos = twp->pos;
			pos.y += 10.0f;
			CreateBomb(&pos, 0.4f);
			twp->mode = 1i8;
			auto bomb_twp = CreateElementalTask(3u, 3, (TaskFuncPtr)0x4E2530)->twp;
			bomb_twp->btimer = 9;
			bomb_twp->smode = 0;
			bomb_twp->mode = 0;
			bomb_twp->ang.y = twp->ang.y;
			bomb_twp->pos = twp->pos;

			if (twp->cwp && twp->cwp->hit_cwp && twp->cwp->mytask)
			{
				task* hit_tp = twp->cwp->hit_cwp->mytask;
				if (IsThisTaskPlayer(hit_tp) >= 0)
					SetVelocityP(TASKWK_PLAYERID(hit_tp->twp), 0.0f, 2.5f, 0.0f);
			}
		}
		else
		{
			EntryColliList(twp);
		}
	}
	else if (twp->mode == 1i8)
	{
		DeadOut(tp);
		return;
	}

	tp->disp(tp);
}

static void __cdecl ObjectWindyBrokenObj_Exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		Exec_m(tp);
	}
	else
	{
		ObjectWindyBrokenObj_Exec_t.Original(tp);
	}
}

void patch_windy_broobj_init()
{
	ObjectWindyBrokenObj_Exec_t.Hook(ObjectWindyBrokenObj_Exec_r);
}

RegisterPatch patch_windy_broobj(patch_windy_broobj_init);
#include "pch.h"
#include "SADXModLoader.h"
#include "multiplayer.h"

// Fans in Final Egg Act 3

DataPointer(float, FunAcc, 0x1AC4854);
DataPointer(float, FunAccRate, 0x1AC4858);

static void __cdecl Fun_ExecATask_r(task* tp);
FastUsercallHookPtr<decltype(&Fun_ExecATask_r), noret, rEDI> Fun_ExecATask_h(0x5B7480);

static void ExecATask_m(task* tp)
{
	auto twp = tp->twp;

	bool sound_played = false;
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (!ptwp)
			continue;

		if (twp->pos.y - 3.0f < ptwp->pos.y)
		{
			auto x = ptwp->pos.x - twp->pos.x;
			auto z = ptwp->pos.z - twp->pos.z;
			if (z * z + x * x < 576.0f)
			{
				auto power = FunAcc - (ptwp->pos.y - twp->pos.y) * FunAccRate;

				if (power < 0.0f)
				{
					power = 0.0f;
				}

				SetAscendPowerP(i, 0, power, 0);

				if (!sound_played && twp->counter.f == 0.0f)
				{
					dsPlay_timer(289, (int)twp, 1, 0, 20);
					sound_played = true;
				}
			}
		}
	}

	twp->counter.f += 1.0f;
	if (twp->counter.f >= action_fun_funflot.motion->nbFrame)
	{
		twp->counter.f = 0.0f;
	}
	EntryColliList(twp);
}

static void __cdecl Fun_ExecATask_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ExecATask_m(tp);
	}
	else
	{
		Fun_ExecATask_h.Original(tp);
	}
}

void patch_finalegg_fun_init()
{
	Fun_ExecATask_h.Hook(Fun_ExecATask_r);
}

RegisterPatch patch_finalegg_fun(patch_finalegg_fun_init);
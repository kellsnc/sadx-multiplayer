#include "pch.h"
#include "multiplayer.h"

// Complete rework of the Windy Valley trampoline
// Allows several players on the same trampoline

enum : Sint8
{
	MODE_INITIAL,
	MODE_NORMAL,
	MODE_ABOVE = 3,
	MODE_ON,
	MODE_5,
	MODE_END
};

static void __cdecl ObjectWindyPoline_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectWindyPoline_r)> ObjectWindyPoline_t(0x4E3200);

/*
 * Custom task to run one logic per player simultaneously
 */
#pragma region BounceTask

 // Reset run once task
static void BounceTaskFree(task* tp)
{
	auto& flag = BYTEn(tp->ptp->twp->timer.l, tp->awp->work.sb[0]);
	if (flag == 2)
	{
		flag = 1;
	}
}

// Run bounce logic
static void BounceTaskExec(task* tp)
{
	auto awp = tp->awp;
	auto pnum = awp->work.sb[0];
	auto jump = awp->work.sb[1] - 1;
	auto& timer = awp->work.sw[1];
	auto& height = awp->work.f[1];

	auto player = playertwp[pnum];

	++timer;

	if (timer > 6)
	{
		if (timer == 7)
		{
			if (jump == 1)
			{
				dsPlay_oneshot(55, 0, 0, 0);
				SetTrampolineVelocityP(pnum, 2, 0.0f, 5.0f, 0.0f);
			}
			else if (jump == 2)
			{
				dsPlay_oneshot(55, 0, 0, 0);
				SetTrampolineVelocityP(pnum, 3, 0.0f, 7.0f, 0.0f);
			}
			else
			{
				player->pos.y = height = height + 2.0f;
			}
		}
		else if (timer == 8 && jump == 0)
		{
			dsPlay_oneshot(55, 0, 0, 0);
			SetTrampolineVelocityP(pnum, 1, 0.0f, 3.0f, 0.0f);
		}
	}
	else
	{
		if (jump)
		{
			height -= 1.4f;
		}
		else
		{
			height -= 1.2f;
		}

		player->pos.y = height;

		auto x = tp->ptp->twp->pos.x - player->pos.x;
		auto z = tp->ptp->twp->pos.z - player->pos.z;
		auto len = njSqrt(z * z + x * x);

		if (len > 3.0f)
		{
			len = 1.0f / len;
			player->pos.x = x * len * 1.5f + player->pos.x;
			player->pos.z = z * len * 1.5f + player->pos.z;
		}
	}

	if (timer > 40)
	{
		FreeTask(tp);
	}
}

// Create bounce task for a specific player, also sets the ON mode
static void CreateBounceTask(task* tp, Sint8 pnum, Sint8 time, Float height)
{
	// Only run once
	if (BYTEn(tp->twp->timer.l, pnum) == 1)
	{
		auto ctp = CreateChildTask(LoadObj_UnknownB, BounceTaskExec, tp);
		ctp->dest = BounceTaskFree;
		ctp->awp->work.sb[0] = pnum;
		ctp->awp->work.sb[1] = time;
		ctp->awp->work.sw[1] = 0;
		ctp->awp->work.f[1] = 0;

		tp->twp->mode = MODE_ON;
		tp->twp->counter.w[0] = 0;
		playertwp[pnum]->pos.y = tp->twp->value.f = ctp->awp->work.f[1] = tp->twp->pos.y + height;
		ChangeModePlaceWithTrampolineP(pnum, time);

		BYTEn(tp->twp->timer.l, pnum) = 2;
	}
}

#pragma endregion

// Check for players
static void Normal_m(taskwk* twp)
{
	for (auto& player : playertwp)
	{
		if (!player)
		{
			continue;
		}

		auto x = player->pos.x - twp->pos.x;
		auto z = player->pos.z - twp->pos.z;

		if (njSqrt(x * x + z * z) < 31.0f && twp->pos.y + 19.0f < player->pos.y && twp->pos.y + 95.0f > player->pos.y)
		{
			if (twp->mode == MODE_NORMAL) twp->mode = MODE_ABOVE;

			twp->smode = TASKWK_PLAYERID(player);

			auto& flag = BYTEn(twp->timer.l, twp->smode);

			if (flag == 0)
			{
				flag = 1;
			}
		}
	}
}

// Run while players are bouncing, resets if no players are bouncing anymore
static void Above_m(taskwk* twp)
{
	auto& jump = twp->counter.w[1];
	bool player_above = false;

	for (auto& player : playertwp)
	{
		if (!player)
		{
			continue;
		}

		auto pnum = TASKWK_PLAYERID(player);

		if (BYTEn(twp->timer.l, pnum) == 0)
		{
			continue;
		}

		auto x = player->pos.x - twp->pos.x;
		auto z = player->pos.z - twp->pos.z;

		if (njSqrt(x * x + z * z) < 29.0f)
		{
			player_above = true;

			if (jump == 0)
			{
				if (twp->pos.y + 19.0f >= player->pos.y)
				{
					CreateBounceTask(twp->cwp->mytask, pnum, 1, 19.0f);
				}
			}
			else if (jump == 1)
			{
				if (twp->pos.y + 22.0f >= player->pos.y)
				{
					CreateBounceTask(twp->cwp->mytask, pnum, 2, 20.0f);
				}
			}
			else
			{
				if (twp->pos.y + 25.0f >= player->pos.y)
				{
					CreateBounceTask(twp->cwp->mytask, pnum, 3, 21.0f);
				}
			}
		}
		else
		{
			BYTEn(twp->timer.l, pnum) = 0;
		}
	}

	if (!player_above)
	{
		twp->mode = MODE_NORMAL;
		jump = 0;
		twp->smode = 0;
		twp->timer.l = 0;
	}
}

// Stripped of the player logic, only the animation logic now
static void On_m(taskwk* twp)
{
	auto& timer = twp->counter.w[0];
	auto& jump = twp->counter.w[1];

	++timer;

	if (timer > 6)
	{
		if (timer > 40)
		{
			twp->scl.y = 0.0f;
			twp->ang.z = 0;
			twp->mode = MODE_5;
			++jump;
			timer = 0;
		}
		else
		{
			twp->ang.z += 0x2000;
			twp->scl.y = njCos(twp->ang.z) * ((Float)(60 - timer) * 0.02f);
		}
	}
	else
	{
		twp->scl.y = twp->scl.y + 0.1f;
	}
}

static void ObjectWindyPoline_m(task* tp)
{
	if (!CheckRangeOut(tp))
	{
		auto twp = tp->twp;

		switch (twp->mode)
		{
		case MODE_NORMAL:
			Normal_m(twp);
			EntryColliList(twp);
			break;
		case MODE_ABOVE:
			Normal_m(twp);
			Above_m(twp);
			break;
		case MODE_ON:
			Normal_m(twp);
			Above_m(twp);
			On_m(twp);
			break;
		case MODE_5:
			twp->mode = MODE_ABOVE;
			break;
		case MODE_END:
			DestroyTask(tp);
			return;
		default:
			twp->mode = MODE_INITIAL;
			break;
		}

		LoopTaskC(tp);
		tp->disp(tp);
	}
}

static void __cdecl ObjectWindyPoline_r(task* tp)
{
	if (multiplayer::IsEnabled() && tp->twp->mode != MODE_INITIAL)
	{
		ObjectWindyPoline_m(tp);
	}
	else
	{
		ObjectWindyPoline_t.Original(tp);
	}
}

void patch_windy_poline_init()
{
	ObjectWindyPoline_t.Hook(ObjectWindyPoline_r);
}

RegisterPatch patch_windy_poline(patch_windy_poline_init);
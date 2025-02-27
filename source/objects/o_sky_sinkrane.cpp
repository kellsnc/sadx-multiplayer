#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl execSkySinkRane_r(task* tp);
static void __cdecl AirLiftSleep_r(task* tp);

FastUsercallHookPtr<decltype(&execSkySinkRane_r), noret, rEAX> execSkySinkRane_t(0x5F5E50);
FastUsercallHookPtr<decltype(&AirLiftSleep_r), noret, rEDI> AirLiftSleep_t(0x4AA430);

static void __cdecl execSkySinkRane_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 1)
	{
		auto twp = tp->twp;
		checkSkySinkRaneColli(twp);

		if (twp->scl.y < 0.0f)
		{
			auto pnum = GetClosestPlayerNumRange(&twp->pos, 300.0f);

			if (pnum >= 0)
			{
				dsPlay_oneshot_v(160, 0, 0, 64, twp->pos.x, twp->pos.y, twp->pos.z);
				twp->mode = 2i8;
			}
		}

		LoopTaskC(tp);
		tp->disp(tp);
	}
	else
	{
		execSkySinkRane_t.Original(tp);
	}
}

static void __declspec(naked) execSkySinkRane_w()
{
	__asm
	{
		push eax
		call execSkySinkRane_r
		pop eax
		retn
	}
}

static void AirLiftSleep_m(task* tp)
{
	auto twp = tp->twp;
	if ((twp->flag & 0x100) == 0 && !tp->ctp)
	{
		if (IsPlayerInSphere(&twp->pos, 1000.0f))
		{
			// This objet has multiple nested display-less child tasks, including task creation inbetween draw calls
			auto ctp = CreateChildTask(2u, EnemyAir, tp);

			auto ctwp = ctp->twp;
			ctwp->pos = twp->pos;
			ctwp->pos.y -= 21.0f;
			ctwp->ang.y = twp->ang.y - 0x4000;

			if (twp->scl.z <= 0.0f)
			{
				if (twp->scl.z <= -10.0f)
				{
					ctwp->smode = 2i8;
					ctwp->ang.y += 0x8000;
				}
				else
				{
					ctwp->smode = 0i8;
				}
			}
			else
			{
				ctwp->smode = 1i8;
			}

			twp->flag |= 0x100;
			twp->mode = 1i8;
			twp->wtimer = 0ui16;
		}
	}
	tp->disp(tp);
}

static void __cdecl AirLiftSleep_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		AirLiftSleep_m(tp);
	}
	else
	{
		AirLiftSleep_t.Original(tp);
	}
}

void patch_sky_sinkrane_init()
{
	execSkySinkRane_t.Hook(execSkySinkRane_r);
	AirLiftSleep_t.Hook(AirLiftSleep_r);
}

RegisterPatch patch_sky_sinkrane(patch_sky_sinkrane_init);
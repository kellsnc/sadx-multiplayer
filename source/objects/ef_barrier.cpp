#include "pch.h"
#include "multiplayer.h"

static void __cdecl TBarrier_r(task* tp);
static void __cdecl NBarrier_r(task* tp);
static void __cdecl Muteki_r(task* tp);

Trampoline TBarrier_t (0x4BA2A0, 0x4BA2A5, TBarrier_r);
Trampoline NBarrier_t (0x4BA380, 0x4BA385, NBarrier_r);
Trampoline Muteki_t   (0x4BA440, 0x4BA446, Muteki_r);

static void __cdecl TBarrier_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pnum = TASKWK_PLAYERID(twp);
		auto pltwp = playertwp[pnum];
		auto plpwp = playerpwp[pnum];

		if (pltwp && plpwp && (plpwp->item & Powerups_MagneticBarrier) != 0)
		{
			EffBarrierPosSet(twp, pltwp);

			if (njRandom() > 0.70)
			{
				auto ctp = CreateChildTask(LoadObj_Data1, ThunderB, tp);
				auto ctwp = ctp->twp;

				if (ctwp)
				{
					ctwp->ang.x = (Angle)(njRandom() * 65536.0);
					ctwp->ang.y = (Angle)(njRandom() * 65536.0);
					ctwp->value.f = 1.0f;
					ctp->disp = TBarrierDisp;
				}
			}
			LoopTaskC(tp);
		}
		else
		{
			FreeTask(tp);
		}
	}
	else
	{
		TARGET_STATIC(TBarrier)(tp);
	}
}

static void __cdecl NBarrier_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pnum = TASKWK_PLAYERID(twp);
		auto pltwp = playertwp[pnum];
		auto plpwp = playerpwp[pnum];

		if (pltwp && plpwp && (plpwp->item & Powerups_Barrier) != 0)
		{
			EffBarrierPosSet(twp, pltwp);

			auto ctp = CreateChildTask(LoadObj_Data1, (TaskFuncPtr)0x4BA1E0, tp);
			auto ctwp = ctp->twp;

			if (ctwp)
			{
				ctwp->ang.x = (Angle)(njRandom() * 65536.0);
				ctwp->ang.y = (Angle)(njRandom() * 65536.0);
				ctp->disp = (TaskFuncPtr)0x4B9F40;
			}

			LoopTaskC(tp);
		}
		else
		{
			FreeTask(tp);
		}
	}
	else
	{
		TARGET_STATIC(NBarrier)(tp);
	}
}

static void __cdecl Muteki_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pnum = TASKWK_PLAYERID(twp);
		auto pltwp = playertwp[pnum];
		auto plpwp = playerpwp[pnum];

		if (!tp->disp)
			tp->disp = (TaskFuncPtr)0x4B9FF0; // stop forgetting displays sadx

		if (pltwp && plpwp && ++twp->timer.l <= 1260)
		{
			plpwp->item |= Powerups_Invincibility;
			EffBarrierPosSet(twp, pltwp);

			for (int i = 0; i < 3; ++i)
			{
				if (njRandom() > 0.70)
				{
					auto ctp = CreateChildTask(LoadObj_Data1, (TaskFuncPtr)0x4BA240, tp);
					auto ctwp = ctp->twp;

					if (ctwp)
					{
						ctwp->ang.x = (Angle)(njRandom() * 65536.0);
						ctwp->ang.y = (Angle)(njRandom() * 65536.0);
						ctp->disp = (TaskFuncPtr)0x4BA070;
					}
				}
			}
			
			LoopTaskC(tp);
		}
		else
		{
			FreeTask(tp);
		}

		tp->disp(tp);
	}
	else
	{
		TARGET_STATIC(Muteki)(tp);
	}
}
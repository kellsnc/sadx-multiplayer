#include "pch.h"
#include "multiplayer.h"

static void __cdecl TBarrier_r(task* tp);
static void __cdecl NBarrier_r(task* tp);
static void __cdecl Muteki_r(task* tp);

FastFunctionHookPtr<decltype(&TBarrier_r)> TBarrier_t(0x4BA2A0);
FastFunctionHookPtr<decltype(&NBarrier_r)> NBarrier_t(0x4BA380);
FastFunctionHookPtr<decltype(&Muteki_r)> Muteki_t(0x4BA440);

static void EffBarrierPosSet_m(taskwk* twp, taskwk* ptwp)
{
	twp->scl.x = ptwp->scl.x;
	twp->scl.y = (playerpwp[TASKWK_PLAYERID(ptwp)]->p.height * twp->scl.x) * 0.6f;

	njPushMatrix(_nj_unit_matrix_);
	njTranslateV(0, &ptwp->pos);
	ROTATEZ(0, ptwp->ang.z);
	ROTATEX(0, ptwp->ang.x);
	njTranslate(0, 0.0f, twp->scl.y, 0.0f);
	njGetTranslation(0, &twp->pos);
	njPopMatrixEx();
}

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
			EffBarrierPosSet_m(twp, pltwp);

			if (njRandom() > 0.70)
			{
				auto ctp = CreateChildTask(LoadObj_Data1, ThunderB, tp);
				auto ctwp = ctp->twp;

				if (ctwp)
				{
					ctwp->ang.x = (Angle)(njRandom() * 65536.0f);
					ctwp->ang.y = (Angle)(njRandom() * 65536.0f);
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
		TBarrier_t.Original(tp);
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
			EffBarrierPosSet_m(twp, pltwp);

			auto ctp = CreateChildTask(LoadObj_Data1, (TaskFuncPtr)0x4BA1E0, tp);
			auto ctwp = ctp->twp;

			if (ctwp)
			{
				ctwp->ang.x = (Angle)(njRandom() * 65536.0f);
				ctwp->ang.y = (Angle)(njRandom() * 65536.0f);
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
		NBarrier_t.Original(tp);
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
			EffBarrierPosSet_m(twp, pltwp);

			for (int i = 0; i < 3; ++i)
			{
				if (njRandom() > 0.70)
				{
					auto ctp = CreateChildTask(LoadObj_Data1, (TaskFuncPtr)0x4BA240, tp);
					auto ctwp = ctp->twp;

					if (ctwp)
					{
						ctwp->ang.x = (Angle)(njRandom() * 65536.0f);
						ctwp->ang.y = (Angle)(njRandom() * 65536.0f);
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
		Muteki_t.Original(tp);
	}
}

void patch_ef_barrier_init()
{
	TBarrier_t.Hook(TBarrier_r);
	NBarrier_t.Hook(NBarrier_r);
	Muteki_t.Hook(Muteki_r);
}

RegisterPatch patch_ef_barrier(patch_ef_barrier_init);
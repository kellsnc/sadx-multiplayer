#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

FastFunctionHook<void, task*> Ring_h(0x450370);
FastFunctionHook<void, task*> ObjectTPRing_h(0x61F4A0);
FastFunctionHook<void, task*> Tobitiri_h(0x44FD10);
FastUsercallHookPtr<Bool(*)(taskwk* twp), rEAX, rEDI> PlayerVacumedRing_h(0x44FA90);
FastFunctionHook<void, char> DamegeRingScatter_h(DamegeRingScatter);

bool GrabRingMulti(taskwk* twp, task* tp)
{
	auto player = CCL_IsHitPlayer(twp);

	if (player)
	{
		int pID = TASKWK_PLAYERID(player);

		if (!(playerpwp[pID]->item & 0x4000))
		{
			twp->mode = 2;
			AddNumRingM(pID, 1);
			dsPlay_oneshot(7, 0, 0, 0);
			tp->disp = RingDoneDisplayer;
			return true;
		}
	}

	return false;
}

// Patch for other players to collect rings
void __cdecl Ring_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1)
		{
			if (GrabRingMulti(twp, tp))
				return;
		}
	}

	Ring_h.Original(tp);
}

void __cdecl ObjectTPRing_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1)
		{
			if (GrabRingMulti(twp, tp))
				return;
		}
	}

	ObjectTPRing_h.Original(tp);
}

// Patch for other players to collect scattered rings
void __cdecl Tobitiri_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1 || twp->mode == 2)
		{
			auto player = CCL_IsHitPlayer(twp);

			if (player)
			{
				twp->mode = 4;
				twp->pos.y += 3.44f;
				twp->scl.x = -2.0f;
				twp->scl.z = -4.0f;
				twp->counter.l = 0;

				ResetParticle((EntityData1*)twp, (NJS_SPRITE*)0x3B42FC0);

				int pID = TASKWK_PLAYERID(player);

				if (!(playerpwp[pID]->item & 0x4000))
				{
					AddNumRingM(pID, 1);
					dsPlay_oneshot(7, 0, 0, 0);
					tp->disp = RingDoneDisplayer;
					return;
				}
			}
		}
	}

	Tobitiri_h.Original(tp);
}

static Bool PlayerVacumedRing_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		// Get closest players with magnetic field
		taskwk* pltwp_ = nullptr;
		playerwk* plpwp = nullptr;
		float dist = 10000000.0f;

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto pltwp = playertwp[i];
			plpwp = playerpwp[i];

			if (pltwp && plpwp && plpwp->item & Powerups_MagneticBarrier)
			{
				NJS_VECTOR v
				{
					twp->pos.x - pltwp->pos.x,
					twp->pos.y - pltwp->pos.y,
					twp->pos.z - pltwp->pos.z
				};

				auto curdist = njScalor(&v);

				if (curdist < dist)
				{
					dist = curdist;
					pltwp_ = pltwp;
				}
			}
		}

		// found one
		if (pltwp_ && (dist < 50.0f || twp->wtimer))
		{
			NJS_VECTOR dir = { 0.0f, 7.0f, 0.0f };
			njPushMatrix(_nj_unit_matrix_);
			if (pltwp_->ang.z) njRotateZ(0, pltwp_->ang.z);
			if (pltwp_->ang.x) njRotateX(0, pltwp_->ang.x);
			if (pltwp_->ang.y) njRotateY(0, pltwp_->ang.y);
			njCalcPoint(0, &dir, &dir);
			njPopMatrixEx();

			dir.x += pltwp_->pos.x;
			dir.y += pltwp_->pos.y;
			dir.z += pltwp_->pos.z;

			// clamp
			if (dist > 50.0f)
			{
				dist = 50.0f;
			}

			dist = min(5.0f, max(0.85f, dist * 0.026f));

			if (plpwp)
			{
				dist *= (njScalor(&plpwp->spd) * 0.5f + 1.0f);
			}

			CalcAdvanceAsPossible(&twp->pos, &dir, dist, &twp->pos);
			++twp->wtimer;

			twp->counter.f = twp->counter.f + 3.0f;
			EntryColliList(twp);
			return TRUE;
		}

		return FALSE;
	}
	else
	{
		return PlayerVacumedRing_h.Original(twp);
	}
}

void DamegeRingScatter_r(char pno)
{
	if (multiplayer::IsActive())
	{
		auto rings = GetNumRingM(pno);

		if (rings > 0)
		{
			ResetNumRingP(pno);

			for (int i = 0; i < min(20, rings); ++i)
			{
				auto tp = CreateElementalTask(LoadObj_UnknownB | LoadObj_Data1, 2, (TaskFuncPtr)0x44FD10);
				tp->twp->pos = playertwp[pno]->pos;
				tp->twp->ang.y = NJM_DEG_ANG(((double)(i * 350.0) / (double)rings) + (njRandom() * 360.0));
			}

			dsPlay_oneshot(0, 0, 0, 0);
		}
		else if (playertwp[pno] && playertwp[pno]->id != 3)
		{	
			KillHimP(pno);

			if (TASKWK_CHARID(playertwp[pno]) == Characters_Gamma)
			{
				dsPlay_oneshot(1431, 0, 0, 0);
			}
			else
			{
				dsPlay_oneshot(23, 0, 0, 0);
			}
		}
	}
	else
	{
		return DamegeRingScatter_h.Original(pno);
	}
}

void patch_ring_init()
{
	Ring_h.Hook(Ring_r);
	ObjectTPRing_h.Hook(ObjectTPRing_r);
	Tobitiri_h.Hook(Tobitiri_r);
	PlayerVacumedRing_h.Hook(PlayerVacumedRing_r);
	DamegeRingScatter_h.Hook(DamegeRingScatter_r);
}

RegisterPatch patch_ring(patch_ring_init);
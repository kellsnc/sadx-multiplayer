#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

FastFunctionHook<void, task*> Ring_t(0x450370);
FastFunctionHook<void, task*> ObjectTPRing_t(0x61F4A0);
FastFunctionHook<void, task*> Tobitiri_t(0x44FD10);
FastUsercallHookPtr<Bool(*)(taskwk* twp), rEAX, rEDI> PlayerVacumedRing_t(0x44FA90);

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

	Ring_t.Original(tp);
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

	ObjectTPRing_t.Original(tp);
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

	Tobitiri_t.Original(tp);
}

static BOOL PlayerVacumedRing_r(taskwk* twp)
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
		return PlayerVacumedRing_t.Original(twp);
	}
}

void patch_ring_init()
{
	Ring_t.Hook(Ring_r);
	ObjectTPRing_t.Hook(ObjectTPRing_r);
	Tobitiri_t.Hook(Tobitiri_r);
	PlayerVacumedRing_t.Hook(PlayerVacumedRing_r);
}

RegisterPatch patch_ring(patch_ring_init);
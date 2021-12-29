#include "pch.h"
#include "levels.h"

/*

Level-related adjustements for multiplayer

*/

Trampoline* FogColorChange_t    = nullptr;
Trampoline* PlayerVacumedRing_t = nullptr;
Trampoline* SetPlayerInitialPosition_t = nullptr;

// Put players side by side
void __cdecl SetPlayerInitialPosition_r(taskwk* twp)
{
	TARGET_DYNAMIC(SetPlayerInitialPosition)(twp);

	if (IsMultiplayerEnabled())
	{
		static const int dists[]
		{
			-5.0f,
			5.0f,
			-10.0f,
			10.0f
		};

		twp->pos.x += njCos(twp->ang.y + 0x4000) * dists[TASKWK_PLAYERID(twp)];
		twp->pos.z += njSin(twp->ang.y + 0x4000) * dists[TASKWK_PLAYERID(twp)];
	}
}

static void FogColorChange_r(task* tp)
{
	if (!IsMultiplayerEnabled())
	{
		auto target = TARGET_DYNAMIC(FogColorChange);

		__asm
		{
			mov eax, [tp]
			call target
		}
	}
}

static void __declspec(naked) FogColorChange_w()
{
	__asm
	{
		push eax
		call FogColorChange_r
		pop eax
		retn
	}
}

static BOOL PlayerVacumedRing_r(taskwk* twp)
{
	if (IsMultiplayerEnabled())
	{
		// Get closest players with magnetic field
		taskwk* pltwp_ = nullptr;
		playerwk* plpwp = nullptr;
		float dist = 10000000.0f;

		for (int i = 0; i < player_count; ++i)
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
		auto target = TARGET_DYNAMIC(PlayerVacumedRing);
		BOOL result;
		__asm
		{
			mov esi, [twp]
			call target
			mov result, eax
		}
		return result;
	}
}

static void __declspec(naked) PlayerVacumedRing_w()
{
	__asm
	{
		push esi
		call PlayerVacumedRing_r
		pop esi
		retn
	}
}
void InitLevels()
{
	// Windy Valley tornade effects
	FogColorChange_t    = new Trampoline(0x4DD240, 0x4DD246, FogColorChange_w);
	PlayerVacumedRing_t = new Trampoline(0x44FA90, 0x44FA96, PlayerVacumedRing_w);
	SetPlayerInitialPosition_t = new Trampoline(0x414810, 0x414815, SetPlayerInitialPosition_r);

	// Patch Skyboxes (display function managing mode)
	WriteData((void**)0x4F723E, (void*)0x4F71A0); // Emerald Coast
	WriteData((void**)0x4DDBFE, (void*)0x4DDB60); // Windy Valley
	WriteData((void**)0x61D57E, (void*)0x61D4E0); // Twinkle Park
	WriteData((void**)0x610A7E, (void*)0x6109E0); // Speed Highway
	WriteData((void**)0x5E1FCE, (void*)0x5E1F30); // Lost World
	WriteData((void**)0x4EA26E, (void*)0x4EA1D0); // Ice Cap
}
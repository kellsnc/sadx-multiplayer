#include "pch.h"
#include "gravity.h"

// Allow gravity to be configured per player
// Once SetUserGravity is called, the player is no longer using global gravity. Calling ResetUserGravity reverts this.
// Mods may use multi_set_gravity, multi_get_gravity and multi_reset_gravity

FastFunctionHook<void> SetDefaultGravity_h(0x43B490);
FastFunctionHook<void, Angle, Angle> SetUserGravityXZ_h(0x43B4C0);

namespace gravity
{
	struct GravityInfo
	{
		bool enabled = true;
		Angle angGx, angGz = 0;
		NJS_POINT3 vG = { 0.0f, -1.0f, 0.0f };
	} static playersGravity[PLAYER_MAX];

	GravityInfo globalGravity;

	// Backup global gravity
	void SaveGlobalGravity()
	{
		globalGravity.angGx = GravityAngle_Z;
		globalGravity.angGz = GravityAngle_X;
		globalGravity.vG = Gravity;
	}

	// Replace global gravity with player's gravity
	void SwapGlobalToUserGravity(int pnum)
	{
		if (GetUserGravity(pnum, &Gravity, &GravityAngle_Z, &GravityAngle_X))
		{
			globalGravity.enabled = true;
		}
	}

	// Restore backed up global gravity
	void RestoreGlobalGravity()
	{
		if (globalGravity.enabled)
		{
			GravityAngle_Z = globalGravity.angGx;
			GravityAngle_X = globalGravity.angGz;
			Gravity = globalGravity.vG;
		}
	}

	// Get custom gravity for a player if it exists, returns false if it doesn't.
	bool GetUserGravity(int pnum, NJS_POINT3* v, Angle* angx, Angle* angz)
	{
		if (pnum >= 0 && pnum < PLAYER_MAX)
		{
			auto& g = playersGravity[pnum];

			if (!g.enabled)
			{
				return false;
			}

			if (v) *v = g.vG;
			if (angx) *angx = g.angGx;
			if (angz) *angz = g.angGz;

			return true;
		}

		return false;
	}

	// Set custom gravity data for a specific player, no longer using global gravity.
	void SetUserGravity(Angle angx, Angle angz, int pnum)
	{
		if (pnum >= 0 && pnum < PLAYER_MAX)
		{
			auto& g = playersGravity[pnum];

			float x = -(njCos(-angx) * njSin(-angz));
			float y = -(njCos(-angx) * njCos(-angz));
			float z = njSin(-angx);

			g.vG = { x, y, z };
			g.angGx = NJM_RAD_ANG(asin(-z));
			g.angGz = NJM_RAD_ANG(-atan2(-x, -y));

			g.enabled = true;
		}
	}

	// Undo custom gravity data for a player, reverting to global gravity.
	void ResetUserGravity(int pnum)
	{
		if (pnum >= 0 && pnum < PLAYER_MAX)
		{
			playersGravity[pnum].enabled = false;
		}
	}

	static void Disable()
	{
		for (auto& i : playersGravity)
		{
			i.enabled = false;
		}
	}
}

static void __cdecl SetDefaultGravity_r()
{
	SetDefaultGravity_h.Original();
	gravity::Disable();
}

// Changing global gravity overwrites player specific ones
static void __cdecl SetUserGravityXZ_r(Angle angx, Angle angz)
{
	if (angx == 0 && angz == 0)
	{
		SetDefaultGravity_r();
	}
	else
	{
		SetUserGravityXZ_h.Original(angx, angz);
		gravity::Disable();
	}
}

void InitGravityPatches()
{
	SetDefaultGravity_h.Hook(SetDefaultGravity_r);
	SetUserGravityXZ_h.Hook(SetUserGravityXZ_r);
}
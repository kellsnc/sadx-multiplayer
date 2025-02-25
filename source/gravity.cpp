#include "pch.h"
#include "gravity.h"

// Allow per-player gravity

FastFunctionHook<void> SetDefaultGravity_t(0x43B490);
FastFunctionHook<void, Angle, Angle> SetUserGravityXZ_t(0x43B4C0);

namespace gravity
{
	struct GravityInfo
	{
		bool enabled = true;
		Angle angGx, angGz = 0;
		NJS_POINT3 vG = { 0.0f, -1.0f, 0.0f };
	} static playersGravity[PLAYER_MAX];

	GravityInfo globalGravity;

	void SaveGlobalGravity()
	{
		globalGravity.angGx = GravityAngle_Z;
		globalGravity.angGz = GravityAngle_X;
		globalGravity.vG = Gravity;
	}

	void SwapGlobalToUserGravity(int pnum)
	{
		if (GetUserGravity(pnum, &Gravity, &GravityAngle_Z, &GravityAngle_X))
		{
			globalGravity.enabled = true;
		}
	}

	void RestoreGlobalGravity()
	{
		if (globalGravity.enabled)
		{
			GravityAngle_Z = globalGravity.angGx;
			GravityAngle_X = globalGravity.angGz;
			Gravity = globalGravity.vG;
		}
	}

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
	SetDefaultGravity_t.Original();
	gravity::Disable();
}

static void __cdecl SetUserGravityXZ_r(Angle angx, Angle angz)
{
	if (angx == 0 && angz == 0)
	{
		SetDefaultGravity_r();
	}
	else
	{
		SetUserGravityXZ_t.Original(angx, angz);
		gravity::Disable();
	}
}

void InitGravityPatches()
{
	SetDefaultGravity_t.Hook(SetDefaultGravity_r);
	SetUserGravityXZ_t.Hook(SetUserGravityXZ_r);
}
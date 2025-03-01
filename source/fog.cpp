#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "VariableHook.hpp"
#include "splitscreen.h"
#include "fog.h"

// Allow fog to be configured per player
// Once SetUserFog is called, the player is no longer using global fog. Calling ResetUserFog reverts this.
// Mods may use multi_set_fog, multi_get_fog and multi_reset_fog

FastFunctionHook<void> ___njFogEnable_h(0x411AF0);

namespace fog
{
	struct {
		bool enabled = false;
		___stcFog data;
	} static playersFog[PLAYER_MAX];

	// Get custom fog for a player if it exists, returns false if it doesn't.
	bool GetUserFog(int pnum, ___stcFog* pFog)
	{
		if (pnum >= 0 && pnum < PLAYER_MAX)
		{
			auto fog = &playersFog[pnum];
			if (fog->enabled)
			{
				if (pFog)
				{
					*pFog = fog->data;
				}
				return true;
			}
		}
		return false;
	}

	// Set custom fog data for a specific player, no longer using global fog.
	void SetUserFog(int pnum, ___stcFog* pFog)
	{
		if (pnum >= 0 && pnum < PLAYER_MAX)
		{
			playersFog[pnum].data = *pFog;
			playersFog[pnum].enabled = true;
		}
	}

	// Undo custom fog data for a player, reverting to global fog.
	void ResetUserFog(int pnum)
	{
		if (pnum >= 0 && pnum < PLAYER_MAX)
		{
			playersFog[pnum].enabled = false;
		}
	}
}

// Same as setfog but with a fog data argument
static bool setfog_m(int pnum, ___stcFog* pFog)
{
	if (!(gFogEmu.u8Emulation & 1))
	{
		auto n = pFog->f32StartZ;
		auto f = pFog->f32EndZ;

		if (n > 0.0f)
			n = -n;

		if (f > 0.0f)
			f = -f;

		njSetFogColor(pFog->Col);

		if (n != oldn && f != oldf)
		{
			njGenerateFogTable3((float*)&FogTable, n, f);
			njSetFogTable_((float*)&FogTable);
		}

		oldn = n;
		oldf = f;
	}

	return true;
}

// In multiplayer mode, check if a player has custom fog data and use it instead if so
static void ___njFogEnable_m()
{
	if (!loop_count)
	{
		auto num = SplitScreen::numViewPort;

		___stcFog fog = gFog;
		fog::GetUserFog(num, &fog); // Overwrite fog if custom data exists

		if (fog.u8Enable)
		{
			if (setfog_m(num, &fog))
			{
				if (!(fogemulation & 1))
				{
					njEnableFog();
				}
				else
				{
					njDisableFog();
				}
			}
			gu8FogEnbale = TRUE;
		}
		else
		{
			ItDisablesFog();
			njDisableFog();
		}
	}
}

static void __cdecl ___njFogEnable_r()
{
	if (SplitScreen::IsActive())
	{
		___njFogEnable_m();
	}
	else
	{
		___njFogEnable_h.Original();
	}
}

void InitFogPatches()
{
	___njFogEnable_h.Hook(___njFogEnable_r);
}
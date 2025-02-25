#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "VariableHook.hpp"
#include "splitscreen.h"
#include "fog.h"

FastFunctionHook<void> ___njFogEnable_t(0x411AF0);

namespace fog
{
	struct {
		bool enabled = false;
		___stcFog data;
	} static playersFog[PLAYER_MAX];

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

	void SetUserFog(int pnum, ___stcFog* pFog)
	{
		if (pnum >= 0 && pnum < PLAYER_MAX)
		{
			playersFog[pnum].data = *pFog;
			playersFog[pnum].enabled = true;
		}
	}

	void ResetUserFog(int pnum)
	{
		if (pnum >= 0 && pnum < PLAYER_MAX)
		{
			playersFog[pnum].enabled = false;
		}
	}
}

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

static void ___njFogEnable_m()
{
	if (!loop_count)
	{
		auto num = SplitScreen::numViewPort;

		___stcFog fog = gFog;
		fog::GetUserFog(num, &fog);

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
		___njFogEnable_t.Original();
	}
}

void InitFogPatches()
{
	___njFogEnable_t.Hook(___njFogEnable_r);
}

#include "pch.h"
#include "splitscreen.h"
#include "fog.h"

Trampoline* ___njFogEnable_t = nullptr;

MAKEVARMULTI(___stcFog, gFog, 0x3ABDC60);

___stcFog* GetScreenFog(int num)
{
	if (num >= 0 || num < PLAYER_MAX)
	{
		auto fog = gFog_m[num];

		if (num != 0 && fog->u8Enable == 0i8)
		{
			fog->f32StartZ = gFog.f32StartZ;
			fog->f32EndZ = gFog.f32EndZ;
			fog->Col = gFog.Col;
		}

		return fog;
	}
	else
	{
		return nullptr;
	}
}

static bool setfog_m(___stcFog* fog)
{
	if (!(gFogEmu.u8Emulation & 1))
	{
		auto start = fog->f32StartZ;
		auto end = fog->f32EndZ;

		if (start > 0.0f)
			start = -start;

		if (end > 0.0f)
			end = -end;

		njSetFogColor(fog->Col);

		if (start != LastFogLayer || end != LastFogDistance)
		{
			njGenerateFogTable3((float*)&FogTable, fog->f32StartZ, fog->f32EndZ);
			njSetFogTable((float*)&FogTable);
		}

		LastFogLayer = start;
		LastFogDistance = end;
	}

	return true;
}

static void ___njFogEnable_m()
{
	if (!loop_count)
	{
		auto num = SplitScreen::numViewPort;

		// Use default fog for invalid screen ids
		if (num < 0 || num >= PLAYER_MAX)
		{
			num = 0;
		}

		// Use first screen fog if no fog is set for the current screen
		auto fog = gFog_m[num]->u8Enable ? gFog_m[num] : gFog_m[0];

		if (fog->u8Enable)
		{
			if (setfog_m(fog))
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
			gu8FogEnbale = 1i8;
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
		TARGET_DYNAMIC(___njFogEnable)();
	}
}

void InitFogPatches()
{
	___njFogEnable_t = new Trampoline(0x411AF0, 0x411AF5, ___njFogEnable_r);
}
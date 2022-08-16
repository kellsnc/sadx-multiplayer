#include "pch.h"
#include "splitscreen.h"
#include "fog.h"

Trampoline* ___njFogEnable_t = nullptr;

VariableHook<___stcFog, 0x3ABDC60> gFog_m;
VariableHook<Float, 0x90A098> oldn_m;
VariableHook<Float, 0x90A09C> oldf_m;

___stcFog* GetScreenFog(int num)
{
	if (num >= 0 || num < PLAYER_MAX)
	{
		auto fog = &gFog_m[num];

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

static bool setfog_m(int pnum)
{
	if (!(gFogEmu.u8Emulation & 1))
	{
		auto& fog = gFog_m[pnum];
		auto& _oldn = oldn_m[pnum];
		auto& _oldf = oldf_m[pnum];

		auto n = fog.f32StartZ;
		auto f = fog.f32EndZ;

		if (n > 0.0f)
			n = -n;

		if (f > 0.0f)
			f = -f;

		njSetFogColor(fog.Col);

		if (n != _oldn || f != _oldf)
		{
			njGenerateFogTable3((float*)&FogTable, n, f);
			njSetFogTable_((float*)&FogTable);
		}

		_oldn = n;
		_oldf = f;
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

		if (gFog_m[num].u8Enable == 0)
		{
			num = 0;
		}

		if (gFog_m[num].u8Enable)
		{
			if (setfog_m(num))
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
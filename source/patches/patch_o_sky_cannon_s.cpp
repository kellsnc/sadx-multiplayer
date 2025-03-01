#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"

FastFunctionHook<void, task*> ObjectSkydeck_cannon_s_Exec_h(0x5FC7A0);

static char PInCannon[PLAYER_MAX];

void isPlayerinCannon(taskwk* twp)
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (playertwp[i] && GetDistance(&twp->pos, &playertwp[i]->pos) <= 100.0f)
		{
			PInCannon[i] = 1;
		}
		else
		{
			PInCannon[i] = 0;
		}
	}
}

static void SDSetPlayerPos(taskwk* twp)
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (PInCannon[i])
		{
			PositionPlayer(i, twp->pos.x, twp->pos.y, twp->pos.z);
		}
	}
}

static void SDSetPlayerPosDiff(taskwk* twp)
{
	NJS_VECTOR pos = twp->pos;
	pos.z += 90.0f;

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (PInCannon[i])
		{
			if (i > 0)
			{
				if (playertwp[i] && playertwp[i]->counter.b[1] < Characters_Gamma)
					pos.z -= 10 * i;
			}

			PositionPlayer(i, pos.x, pos.y, pos.z);
		}
	}
}

void ObjectSkydeck_cannon_s_Exec_r(task* a1)
{
	if (!multiplayer::IsActive())
	{
		return ObjectSkydeck_cannon_s_Exec_h.Original(a1);
	}

	int timer = 0;
	Angle3 ang = { 0 };
	NJS_VECTOR vector = { 0.0f, 2.5f, 5.0f };
	auto twp = a1->twp;

	isPlayerinCannon(twp);

	switch (twp->mode)
	{
	case 0:
	case 1:
		ObjectSkydeck_cannon_s_Exec_h.Original(a1);
		SDSetPlayerPos(twp);
		return;
	case 2:
		twp->mode++;
		twp->wtimer = 0;
		PadReadOffP(-1);
		SDSetPlayerPosDiff(twp);
		break;
	case 3:
		timer = 10 * 1;
		ang.x = 0;
		ang.z = 0;
		ang.y = (unsigned __int64)NJM_DEG_ANG(njSin((unsigned __int64)NJM_DEG_ANG(timer)) * 70.0f);
		PadReadOffP(-1);
		if (twp->wtimer <= 0x36u)
		{
			for (int i = 0; i < PLAYER_MAX; i++)
			{
				if (PInCannon[i])
				{
					SetAutoPilotForBreak(i);
					SetLookingAngleP(i, &ang);
				}
			}
		}
		else
		{
			for (int i = 0; i < PLAYER_MAX; i++)
			{
				if (playertwp[i] && TASKWK_CHARID(playertwp[i]) == Characters_Sonic)
				{
					if (!MetalSonicFlag)
					{
						if (GetCurrentCharacterID())
						{
							PlaySound(1473, 0, 0, 0);
						}
						else
						{
							PlaySound(1509, 0, 0, 0);
						}
					}
				}
			}

			twp->mode++;
			twp->wtimer = 0;
			PlaySound(184, 0, 0, 0);
		}
		break;
	case 4:
		PadReadOffP(-1);

		if (twp->wtimer == 1)
		{
			for (int i = 0; i < PLAYER_MAX; i++)
			{
				if (PInCannon[i])
				{
					vector = { 0.0f, 2.5f, 5.0f };

					if (i & 1)
					{
						vector.x = 0.5;
					}
					else
					{
						vector.x = 0.0f;
					}

					njUnitVector(&vector);
					SetParabolicMotionP(i, 6.0f, &vector);
				}
			}

			twp->value.f = 40.0f;
		}
		else if ((twp->cwp->flag & 1) == 0)
		{
			twp->mode++;
			twp->wtimer = 0;
			PadReadOnP(0xFFu);
		}
		break;
	case 5:
	default:
		if (twp->cwp && (twp->cwp->flag & 1) && !twp->cwp->hit_cwp->id)
		{
			twp->mode = 2;
			return;
		}
		break;
	}

	ObjectSkydeck_cannon_s_Exec_h.Original(a1);
}

void __cdecl SDIntroPatch(Uint8 charIndex, float x, float y, float z)
{
	if (multiplayer::IsActive())
	{
		if (!PInCannon[charIndex])
			return;
	}

	return PositionPlayer(charIndex, x, y, z);
}

void __cdecl SDIntroPatch2(Uint8 charIndex)
{
	if (multiplayer::IsActive())
	{
		if (!PInCannon[charIndex])
			return;
	}

	return SetAutoPilotForBreak(charIndex);
}

void __cdecl SDIntroPatch3(Uint8 charIndex, float spd, NJS_VECTOR* a3)
{
	if (multiplayer::IsActive())
	{
		if (!PInCannon[charIndex])
			return;
	}

	return SetParabolicMotionP(charIndex, spd, a3);
}

void patch_sky_cannon_s_init()
{
	ObjectSkydeck_cannon_s_Exec_h.Hook(ObjectSkydeck_cannon_s_Exec_r);

	WriteCall((void*)0x5FC82D, SDIntroPatch);
	WriteCall((void*)0x5FC84C, SDIntroPatch);
	WriteCall((void*)0x5FC8C2, SDIntroPatch);
	WriteCall((void*)0x5FC8E5, SDIntroPatch);

	WriteCall((void*)0x5FC9F6, SDIntroPatch2);
	WriteCall((void*)0x5FCA18, SDIntroPatch2);

	WriteCall((void*)0x5FCAB9, SDIntroPatch3);
	WriteCall((void*)0x5FCA78, SDIntroPatch3);
}

RegisterPatch patch_sky_cannon_s(patch_sky_cannon_s_init);
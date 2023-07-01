#include "pch.h"

void ObjectSkydeck_cannon_s_Exec_r(task* a1);
TaskHook ObjectSkydeck_cannon_s_Exec_t(0x5FC7A0, ObjectSkydeck_cannon_s_Exec_r);
static char PInCannon[PLAYER_MAX];

void isPlayerinCannon(taskwk* data)
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (playertwp[i] && GetDistance(&data->pos, &playertwp[i]->pos) <= 100.0f)
		{
			PInCannon[i] = 1;
		}
		else
		{
			PInCannon[i] = 0;
		}
	}
}

static void SDSetPlayerPos(taskwk* data)
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (PInCannon[i])
		{
			PositionPlayer(i, data->pos.x, data->pos.y, data->pos.z);
		}
	}
}

static void SDSetPlayerPosDiff(taskwk* data)
{
	NJS_VECTOR pos = data->pos;
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
		return ObjectSkydeck_cannon_s_Exec_t.Original(a1);
	}

	int timer = 0;
	Angle3 ang = { 0 };
	NJS_VECTOR vector = { 0.0f, 2.5f, 5.0f };
	auto data = a1->twp;

	isPlayerinCannon(data);

	switch (data->mode)
	{
	case 0:
	case 1:
		ObjectSkydeck_cannon_s_Exec_t.Original(a1);
		SDSetPlayerPos(data);
		return;
	case 2:
		data->mode++;
		data->wtimer = 0;
		PadReadOffP(-1);
		SDSetPlayerPosDiff(data);
		break;
	case 3:
		timer = 10 * 1;
		ang.x = 0;
		ang.z = 0;
		ang.y = (unsigned __int64)NJM_DEG_ANG(njSin((unsigned __int64)NJM_DEG_ANG(timer)) * 70.0f);
		PadReadOffP(-1);
		if (data->wtimer <= 0x36u)
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

			data->mode++;
			data->wtimer = 0;
			PlaySound(184, 0, 0, 0);
		}
		break;
	case 4:
		PadReadOffP(-1);

		if (data->wtimer == 1)
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

			data->value.f = 40.0f;
		}
		else if ((data->cwp->flag & 1) == 0)
		{
			data->mode++;
			data->wtimer = 0;
			PadReadOnP(0xFFu);
		}
		break;
	case 5:
	default:
		if (data->cwp && (data->cwp->flag & 1) && !data->cwp->hit_cwp->id)
		{
			data->mode = 2;
			return;
		}
		break;
	}

	ObjectSkydeck_cannon_s_Exec_t.Original(a1);
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

void initSDIntroPatches()
{
	WriteCall((void*)0x5FC82D, SDIntroPatch);
	WriteCall((void*)0x5FC84C, SDIntroPatch);
	WriteCall((void*)0x5FC8C2, SDIntroPatch);
	WriteCall((void*)0x5FC8E5, SDIntroPatch);

	WriteCall((void*)0x5FC9F6, SDIntroPatch2);
	WriteCall((void*)0x5FCA18, SDIntroPatch2);

	WriteCall((void*)0x5FCAB9, SDIntroPatch3);
	WriteCall((void*)0x5FCA78, SDIntroPatch3);
}

void CannonModePhysics(taskwk* data, motionwk2* data2, playerwk* co2)
{
	PGetGravity(data, data2, co2);
	PGetSpeed(data, data2, co2);
	PSetPosition(data, data2, co2);
	PResetPosition(data, data2, co2);
}
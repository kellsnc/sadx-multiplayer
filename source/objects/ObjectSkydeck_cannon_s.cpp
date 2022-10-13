#include "pch.h"

void ObjectSkydeck_cannon_s_Exec_r(task* a1);
TaskHook ObjectSkydeck_cannon_s_Exec_t(0x5FC7A0, ObjectSkydeck_cannon_s_Exec_r);

static void SDSetPlayerPos(taskwk* data)
{
	for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
	{
		if (playertwp[i])
		{
			PositionPlayer(i, data->pos.x, data->pos.y, data->pos.z);
		}
	}
}

static void SDSetPlayerPosDiff(taskwk* data)
{
	NJS_VECTOR pos = data->pos;
	pos.z += 90.0f;

	for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
	{
		if (i > 0)
		{
			pos.z -= 10 * i;
		}

		PositionPlayer(i, pos.x, pos.y, pos.z);
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
		ang.y = NJM_DEG_ANG(njSin(NJM_DEG_ANG(timer)) * 70.0f);
		PadReadOffP(-1);
		if (data->wtimer <= 0x36u)
		{
			for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
			{
				SetAutoPilotForBreak(i);
				SetLookingAngleP(i, &ang);
			}
		}
		else
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
			data->mode++;
			data->wtimer = 0;
			PlaySound(184, 0, 0, 0);
		}
		break;
	case 4:

		PadReadOffP(-1);

		if (data->wtimer == 1)
		{
			for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
			{
				vector = { 0.0f, 2.5f, 5.0f };

				if (i & 1)
				{
					vector.x += 0.5;
				}
				else
				{
					vector.x = 0.0f;
				}

				njUnitVector(&vector);
				SetParabolicMotionP(i, 6.0f, &vector);	
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
		SetFreeCameraMode(1);
		break;
	default:
		data->wtimer = 0;
		break;
	}


	ObjectSkydeck_cannon_s_Exec_t.Original(a1);
}


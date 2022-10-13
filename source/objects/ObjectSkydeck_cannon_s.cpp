#include "pch.h"

void ObjectSkydeck_cannon_s_Exec_r(task* a1);
//TaskHook ObjectSkydeck_cannon_s_Exec_t(0x5FC7A0, ObjectSkydeck_cannon_s_Exec_r);
TaskHook ObjectSkydeck_cannon_s_Exec_t(0x5FC7A0);

static void SDSetPlayerPos(taskwk* data)
{
	for (int i = 0; i < multiplayer::GetPlayerCount(); i++)
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

	for (int i = 0; i < multiplayer::GetPlayerCount(); i++)
	{
		PositionPlayer(i, pos.x, pos.y, pos.z);
		pos.z += 80.0f;
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
		SDSetPlayerPos(data);
		ObjectSkydeck_cannon_s_Exec_t.Original(a1);
		return;
	case 2:
		data->mode++;
		data->wtimer = 0;
		PadReadOffP(-1);
		SDSetPlayerPosDiff(data);
		break;
	case 3:
		timer = 10 * data->wtimer;
		ang.y = njSin(timer) * 65536.0f
			* 0.002777777777777778f
			* 70.0f
			* 65536.0f
			* 0.002777777777777778f;
		PadReadOffP(-1);
		if (data->wtimer <= 0x36u)
		{
			for (int i = 0; i < multiplayer::GetPlayerCount(); i++)
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
			vector = { 0.0f, 2.5f, 5.0f };

			for (int i = 0; i < multiplayer::GetPlayerCount(); i++)
			{
				njUnitVector(&vector);
				SetParabolicMotionP(i, 6.0f, &vector);
				vector.x += 0.5;
			}

			data->value.f = 40.0f;
		}
		else if ((data->cwp->flag & 1) == 0)
		{
			data->mode++;
			data->wtimer = 0;
			EnableController(0xFFu);
		}
		break;
	case 5:
		SetFreeCameraMode(1);
		break;
	default:
		data->wtimer = 0;
		break;
	}

	auto timeA = (data->timer.f - data->value.f * 0.30000001f) * 0.94999999f;
	data->timer.f = timeA;
	auto resultTime = timeA + data->value.f;
	if (resultTime > 0.0f)
	{
		data->value.f = resultTime;
	}
	else
	{
		data->value.f = 0;
		data->timer.f *= -0.64999998f;
	}
	EntryColliList(data);
	if (data->counter.l)
	{

	}


}


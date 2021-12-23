#include "pch.h"
#include "camera.h"

Trampoline* PGetRotation_t = nullptr;
Trampoline* GetPlayersInputData_t = nullptr;
Trampoline* PInitialize_t = nullptr;
Trampoline* NpcMilesSet_t = nullptr;

void __cdecl PGetRotation_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (IsMultiplayerEnabled() && camera_twp)
	{
		auto backup = camera_twp->ang;
		camera_twp->ang = *GetCameraAngle(TASKWK_PLAYERID(twp));
		TARGET_DYNAMIC(PGetRotation)(twp, mwp, pwp);
		camera_twp->ang = backup;
	}
	else
	{
		TARGET_DYNAMIC(PGetRotation)(twp, mwp, pwp);
	}
}

void __cdecl GetPlayersInputData_r()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto controller = per[i];
		float lx = (controller->x1 << 8); // left stick x
		float ly = (controller->y1 << 8); // left stick y

		int ang;
		float strk;

		if (lx > 3072.0f || lx < -3072.0f || ly > 3072.0f || ly < -3072.0f)
		{
			lx = lx <= 3072.0f ? (lx >= -3072.0f ? 0.0f : lx + 3072.0f) : lx - 3072.0f;
			ly = ly <= 3072.0f ? (ly >= -3072.0f ? 0.0f : ly + 3072.0f) : ly - 3072.0f;

			strk = atan2f(ly, lx) * 65536.0f;

			if (camera_twp)
			{
				ang = -(GetCameraAngle(i)->y) - (strk * -0.1591549762031479);
			}
			else
			{
				ang = (strk * 0.1591549762031479);
			}

			float magnitude = ly * ly + lx * lx;
			strk = sqrtf(magnitude) * magnitude * 3.9187027e-14;
			if (strk > 1.0f)
			{
				strk = 1.0f;
			}
		}
		else
		{
			strk = 0.0f;
			ang = 0;
		}

		input_data[i] = { ang, strk };

		if (ucInputStatus == 1 && (i >= 4 || ucInputStatusForEachPlayer[i] == 1))
		{
			input_dataG[i] = input_data[i];
		}
		else
		{
			input_dataG[i] = { 0, 0.0f };
		}
	}
}

void RemovePlayersDamage(int no)
{
	auto twp = playertwp[no];

	if (twp && twp->cwp)
	{
		for (int i = 0; i < twp->cwp->nbInfo; i++) {

			twp->cwp->info[i].damage &= ~0x20u;
		}
	}
}

void PInitialize_r(int no, ObjectMaster* tp)
{
	TARGET_DYNAMIC(PInitialize)(no, tp);

	if (IsMultiplayerEnabled())
	{
		RemovePlayersDamage(no);
	}
}

void __cdecl NpcMilesSet_r(ObjectMaster* obj)
{
	if (!IsMultiplayerEnabled())
	{
		TARGET_DYNAMIC(NpcMilesSet);
	}
}

void InitPlayerPatches()
{
	PGetRotation_t = new Trampoline(0x44BB60, 0x44BB68, PGetRotation_r);
	WriteJump((void*)0x40F170, GetPlayersInputData_r);
	PInitialize_t = new Trampoline(0x442750, 0x442755, PInitialize_r);

	//To do: enable this when mod is ready
	//NpcMilesSet_t = new Trampoline(0x47ED60, 0x47ED65, NpcMilesSet_r);
}
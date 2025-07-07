#include "pch.h"

//Elevator on the Egg Carrier

FunctionPointer(int, sub_5217A0, (), 0x5217A0);

FastUsercallHookPtr<Sint32(*)(taskwk* twp), rEAX, rESI> sub_529180_h(0x529180);
Sint32 sub_529180_r(taskwk* twp)
{
	if (multiplayer::IsActive() == false)
	{
		return sub_529180_h.Original(twp);
	}

	auto pnum = twp->id;
	SetAutoPilotForXZPositionP(pnum, twp->pos.x, twp->pos.z);
	Float z = twp->pos.z - playertwp[pnum]->pos.z;
	Float x = twp->pos.x - playertwp[pnum]->pos.x;
	return x * x + z * z <= 1.0;
}

FastUsercallHookPtr<Sint32(*)(taskwk* twp), rEAX, rESI> SetAutoPilotForXZPositionP_0_h(0x521700);
Sint32 SetAutoPilotForXZPositionP_0_r(taskwk* twp)
{
	if (multiplayer::IsActive() == false)
	{
		return SetAutoPilotForXZPositionP_0_h.Original(twp);
	}

	auto pnum = twp->id;
	SetAutoPilotForXZPositionP(pnum, twp->pos.x, twp->pos.z);
	Float z = twp->pos.z - playertwp[pnum]->pos.z;
	Float x = twp->pos.x - playertwp[pnum]->pos.x;
	return x * x + z * z <= 10.0f;
}

FastUsercallHookPtr<int(*)(task* tp, taskwk* twp), rEAX, rEAX, rESI> CheckPlayerOnLift_h(0x521630);

int CheckPlayerOnLift_r(task* tp, taskwk* twp)
{

	if (multiplayer::IsActive() == false)
	{
		return CheckPlayerOnLift_h.Original(tp, twp);
	}

	for (uint8_t i = 0; i < PLAYER_MAX; i++)
	{
		if (!playertwp[i])
			continue;

		auto first = CheckPlayerRideOnMobileLandObjectP(i, tp);
		auto second = CheckCollisionCylinderP(&twp->pos, 10.0f, 20.0f);
		if (first > 0 && second
			&& playertwp[i]->mode <= 2)
		{
			if ((twp->flag & 0x400) == 0)
			{
				twp->id = i;
				PadReadOffP(-1);
				SetAutoPilotForBreak(i);
				return TRUE;
			}
		}
		else
		{

			twp->flag &= 0xFBFFu;
			if (!EV_Check(1))
			{
				Float z = playertwp[i]->pos.z - twp->pos.z;
				Float y = playertwp[i]->pos.y - twp->pos.y;
				Float x = playertwp[i]->pos.x - twp->pos.x;
				Float res = x * x + y * y + z * z;
				if (njSqrt(res) < 40.0f)
				{
					PadReadOnP(-1);
					CancelAutoPilotP(i);
				}
			}
		}
	}

	return FALSE;
}

FastUsercallHookPtr<int(*)(task* tp, taskwk* twp), rEAX, rEAX, rESI> CheckPlayerOnLift2_h(0x5290C0);

int CheckPlayerOnLift2_r(task* tp, taskwk* twp)
{

	if (multiplayer::IsActive() == false)
	{
		return CheckPlayerOnLift2_h.Original(tp, twp);
	}

	for (uint8_t i = 0; i < PLAYER_MAX; i++)
	{
		if (!playertwp[i])
			continue;

		if (CheckPlayerRideOnMobileLandObjectP(i, tp) > 0 && CheckCollisionCylinderP(&twp->pos, 10.0f, 20.0f)
			&& playertwp[i]->mode <= 2)
		{
			twp->id = i;
			PadReadOffP(-1);
			SetAutoPilotForBreak(i);
			return TRUE;
		}
		else
		{
			if (!EV_Check(1))
			{
				Float z = playertwp[i]->pos.z - twp->pos.z;
				Float y = playertwp[i]->pos.y - twp->pos.y;
				Float x = playertwp[i]->pos.x - twp->pos.x;
				Float res = x * x + y * y + z * z;
				if (njSqrt(res) < 40.0f)
				{
					PadReadOnP(-1);
					CancelAutoPilotP(i);
				}
			}
		}
	}

	return FALSE;
}

FastFunctionHook<void, task*> LiftRegular_h(0x5218E0);

void LiftRegular_r(task* tp)
{

	LiftRegular_h.Original(tp);

	if (!CheckRangeOut(tp))
	{
		if (sub_5217A0())
		{

			if (multiplayer::IsActive())
			{
				taskwk* twp = tp->twp;
				for (uint8_t i = 1; i < multiplayer::GetPlayerCount(); i++)
				{
					Float z = playertwp[i]->pos.z - twp->pos.z;
					Float y = playertwp[i]->pos.y - twp->pos.y;
					Float x = playertwp[i]->pos.x - twp->pos.x;
					Float res = z * z + x * x + y * y;
					if (njSqrt(res) < 40.0f)
					{
						twp->flag |= 0x100;
					}
				}
			}
		}

	}
}

void patch_ec_egglift_init()
{
	CheckPlayerOnLift_h.Hook(CheckPlayerOnLift_r);
	CheckPlayerOnLift2_h.Hook(CheckPlayerOnLift2_r);
	LiftRegular_h.Hook(LiftRegular_r);
	sub_529180_h.Hook(sub_529180_r);
	SetAutoPilotForXZPositionP_0_h.Hook(SetAutoPilotForXZPositionP_0_r);
}

RegisterPatch patch_ec_egglift(patch_ec_egglift_init);
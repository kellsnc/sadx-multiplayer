#include "pch.h"


FastFunctionHook<void, task*> ObjectBumperExec1_h(0x5DBD70);
FastFunctionHook<void, task*> ObjectBumperExec2_h(0x5DBF50);
FastFunctionHook<void, task*> ObjectBumperExecChild2_h(0x5DBEC0);
TaskFunc(sub_5DBCD0, 0x5DBCD0);

/// <summary>
/// Caps Pinball Sonic's speed.
/// </summary>
/// <param name="NowSpeed"></param>
/// <returns>8.0f if it's less than NowSpeed</returns>
Float CheckBallSpeed(Float NowSpeed)
{
	if (NowSpeed <= 8.0f)
		return NowSpeed;
	else
		return 8.0f;
}

Float GetBallSpeedM(const uint8_t pnum)
{
	Float result = playerpwp[pnum]->spd.x;
	if (result < 0.0f)
	{
		result = -result;
	}
	return result;
}

static void BumperExec1(task* tp)
{
	Angle3 ang;
	NJS_POINT3 v;
	taskwk* twp = tp->twp;
	if (twp->cwp->flag & 1)
	{
		auto player = CCL_IsHitPlayer(twp);
		auto pnum = player->counter.b[0];

		if (!(twp->flag & 0x8000))
		{
			ang.x = 0;
			ang.y = SHORT_ANG(0x4000 - -njArcTan2(twp->pos.z - playertwp[0]->pos.z, playertwp[0]->pos.x - twp->pos.x));
			ang.z = 0;
			v.x = CheckBallSpeed(GetBallSpeedM(pnum) + (njRandom() * twp->scl.y) + twp->scl.z);
			v.y = 0.0f;
			v.z = 0.0f;
			SetVelocityAndRotationAndNoconTimeP(pnum, &v, &ang, 0);
			twp->mode = 2;
			dsPlay_oneshot(SE_CA_BUMPER, 0, 0, 0);
			VibShot(pnum, 0);
			if (++twp->btimer == 20)
			{
				++twp->wtimer;
				twp->btimer = 0;
				SetGetRingM(10 * twp->wtimer + 1, pnum);
			}
			else
			{
				SetGetRingM(1, pnum);
			}
			twp->flag |= 0x8000;
		}
	}
	else
	{
		twp->flag &= ~0x8000;
	}
	tp->disp(tp);
	EntryColliList(twp);
	ObjectSetupInput(twp, 0);
}

static void ObjectBumperExecChild2(task* tp)
{
	taskwk* ptwp = tp->ptp->twp;
	taskwk* twp = tp->twp;
	if (twp->cwp->flag & 1)
	{
		auto player = CCL_IsHitPlayer(twp);
		auto pnum = player->counter.b[0];

		if (!(twp->flag & 0x8000))
		{
			SetAccelerationP(pnum, -playermwp[pnum]->spd.x, 2.0f - playermwp[pnum]->spd.y, 0.0f);
			ptwp->flag |= 0x200;
			ptwp->mode = 2;
			dsPlay_oneshot(SE_CA_BUMPER, 0, 0, 0);
			VibShot(pnum, 0);
			ptwp->flag |= 0x8000;
		}
	}
	else
	{
		ptwp->flag &= ~0x8000;
	}
	sub_5DBCD0(tp);
	EntryColliList(twp);
	ObjectSetupInput(twp, 0);
}

static void BumperExec2(task* tp)
{
	taskwk* twp = tp->twp;
	LoopTaskC(tp);

	auto pCount = multiplayer::GetPlayerCount();
	if ((twp->flag & 0x200) != 0)
	{
		for (uint8_t i = 0; i < pCount; i++)
		{
			if (twp->pos.y <= playertwp[i]->pos.y)
			{
				if (!(playertwp[i]->flag & 0x1))
				{
					Float analogX = (Float)(per[0]->x1 << 8);
					if (analogX >= -3072.0f) //Left
					{
						if (analogX > 3072.0f) //Right
						{
							SetAccelerationP(i, 0.0f, 0.0f, 0.05f);
						}
					}
					else
					{
						SetAccelerationP(i, 0.0f, 0.0f, -0.05f);
					}
					break;
				}

			}
			else
			{
				twp->flag &= ~0x200;
			}
		}
	}
}

void patch_bumper_init()
{
	ObjectBumperExec1_h.Hook(BumperExec1);
	ObjectBumperExec2_h.Hook(BumperExec2);
	ObjectBumperExecChild2_h.Hook(ObjectBumperExecChild2);
}

RegisterPatch patch_bumper(patch_bumper_init);
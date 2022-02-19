#include "pch.h"
#include "multiplayer.h"

DataPointer(NJS_POINT3, heli_pos, 0x3C8107C);
DataPointer(Angle, heli_angy_spd, 0x3C8108C);
DataPointer(Angle, heli_angy, 0x3C81088);
DataPointer(Angle, HeliStackA, 0x3C81078);
DataPointer(Angle, heli_AngTmp, 0x3C81090);
DataPointer(Float, heli_PosTmp, 0x3C81094);
DataPointer(Float, heli_PosTmpZ, 0x3C81098);
DataPointer(Float, heli_PosTmpX, 0x3C8109C);

enum MD_HELI // made up
{
	MD_HELI_GROUND,
	MD_HELI_TAKEOFF,
	MD_HELI_HOVER,
	MD_HELI_MOVE,
	MD_HELI_STOP
};

enum MD_HELILIGHT
{
	MD_HELILIGHT_CUSTOM,
	MD_HELILIGHT_RIGHT,
	MD_HELILIGHT_LEFT,
	MD_HELILIGHT_TARGET
};

static void HeliWriteSub_m(task* tp, taskwk* twp)
{
	if (!IsCameraInSphere(&twp->pos, 1000.0f))
	{
		return;
	}

	auto ptwp = playertwp[GetTheNearestPlayerNumber(&twp->pos)];

	switch (twp->smode)
	{
	case MD_HELILIGHT_RIGHT:
		twp->timer.l += 384;
		twp->value.l += 256;
		if (twp->timer.l >= 6144)
		{
			twp->smode = MD_HELILIGHT_LEFT;
		}
		break;
	case MD_HELILIGHT_LEFT:
		twp->timer.l -= 384;
		twp->value.l -= 256;
		if (twp->timer.l <= -6144)
		{
			twp->smode = MD_HELILIGHT_RIGHT;
		}
		break;
	case MD_HELILIGHT_TARGET:
		heli_PosTmpZ = twp->pos.z - ptwp->pos.z;
		heli_PosTmpX = twp->pos.x - ptwp->pos.x - -28.884884f;
		heli_PosTmp = squareroot(heli_PosTmpZ * heli_PosTmpZ + heli_PosTmpX * heli_PosTmpX);
		
		if (heli_PosTmp > 50.0f)
		{
			heli_PosTmp = 50.0f;
		}

		heli_AngTmp = NJM_DEG_ANG(heli_PosTmp);

		if (twp->timer.l > heli_AngTmp + 64 || twp->timer.l < heli_AngTmp - 64)
		{
			if (twp->timer.l <= heli_AngTmp)
				twp->timer.l += 64;
			else
				twp->timer.l -= 64;
		}

		heli_AngTmp = static_cast<int16_t>(twp->value.l + twp->ang.y - NJM_RAD_ANG(atan2f(heli_PosTmpX, heli_PosTmpZ)));

		if (heli_AngTmp > 0x180)
		{
			if (heli_AngTmp >= 0x8000)
				twp->value.l += 0x180;
			else
				twp->value.l -= 0x180;
		}

		break;
	}

	if (twp->flag & 0x2000)
	{
		if (twp->flag & 0x4000)
		{
			twp->scl.z += 0.2f;
			if (twp->scl.z >= 2.0f)
			{
				twp->scl.z = 2.0f;
				twp->flag &= ~0x4000;
			}
		}
		else
		{
			twp->scl.z -= 0.2f;
			if (twp->scl.z <= -2.0f)
			{
				twp->scl.z = -2.0f;
				twp->flag |= 0x4000;
			}
		}
	}
	
	twp->pos.y += twp->scl.z;
	twp->counter.l += twp->wtimer;

	tp->disp(tp);
}

static void ObjectHeli_Stop(taskwk* twp)
{
	if (twp->scl.x < 2200.0f)
	{
		if (twp->scl.y < 4.0f)
		{
			twp->ang.z += 64;
			twp->scl.y += 0.12f;
		}
	}
	else
	{
		twp->scl.y = 0.0f;
		twp->scl.x = 30.0f;
		twp->ang.z = 0;
		twp->mode = MD_HELI_HOVER;
	}
}

static void ObjectHeli_Move(taskwk* twp)
{
	auto pnum = twp->btimer;
	auto ptwp = playertwp[pnum];

	if (ptwp && twp->scl.x > 1100.0f)
	{
		twp->flag &= ~0xF00u;
		ptwp->pos.y = twp->pos.y - 30.0f;
		SetInputP(pnum, 24);
		twp->mode = MD_HELI_STOP;
		twp->scl.y = 1.0f;
		return;
	}

	if (twp->scl.x <= 940.0)
	{
		if (twp->scl.y < 6.0f)
		{
			twp->ang.z += 32;
			twp->scl.y += 0.06f;
		}
	}
	else
	{
		twp->scl.y -= 0.1f;

		if (twp->ang.z <= 0)
		{
			twp->ang.z = 0;
		}
		else
		{
			twp->ang.z -= 96;
		}
	}

	if (ptwp)
	{
		HeliPosCopyPlayer(ptwp, twp);
	}
}

static void ObjectHeli_Hover(task* tp, taskwk* twp)
{
	twp->flag |= 0x2000;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp == nullptr)
		{
			continue;
		}

		auto x = twp->pos.x - ptwp->pos.x;
		auto z = twp->pos.z - ptwp->pos.z;

		if (twp->pos.y - 10.0f <= ptwp->pos.y || twp->pos.y - 32.0f >= ptwp->pos.y || sqrtf(x * x + z * z) >= 20.0f)
		{
			LoopTaskC(tp);
		}
		else
		{
			if ((twp->ang.y - NJM_RAD_ANG(-atan2(twp->pos.z - ptwp->pos.z, twp->pos.x - ptwp->pos.x))) >= 0x8000u)
			{
				if ((twp->ang.y + ptwp->ang.y) <= 0x4000u || (twp->ang.y + ptwp->ang.y) >= 0xC000u)
				{
					twp->flag |= 0x800;
				}
				else
				{
					twp->flag |= 0x400;
				}
			}
			else if ((twp->ang.y + ptwp->ang.y) <= 0x4000u || (twp->ang.y + ptwp->ang.y) >= 0xC000u)
			{
				twp->flag |= 0x200;
			}
			else
			{
				twp->flag |= 0x100;
			}

			twp->btimer = TASKWK_PLAYERID(ptwp);
			SetInputP(twp->btimer, 16);
			twp->mode = MD_HELI_MOVE;
			twp->flag &= ~0x2000;
		}
	}
}

static void ObjectHeli_TakeOff(task* tp, taskwk* twp)
{
	if (twp->wtimer >= 0xF00)
	{
		if (twp->scl.x >= 30.0f)
		{
			twp->flag |= 0x3000u;
			LoopTaskC(tp);
			twp->flag &= ~0x3000u;

			twp->mode = MD_HELI_HOVER;
		}
		else
		{
			twp->scl.x += 0.5f;
		}
	}
	else
	{
		twp->wtimer += 64;
	}
}

static void ObjectHeli_Ground(taskwk* twp)
{
	if (IsPlayerInSphere(&twp->pos, 320.0f))
	{
		twp->mode = MD_HELI_TAKEOFF;
	}
}

static void ObjectHeliExec_m(task* tp)
{
	auto twp = tp->twp;

	switch (twp->mode)
	{
	case MD_HELI_GROUND:
		ObjectHeli_Ground(twp);
		break;
	case MD_HELI_TAKEOFF:
		ObjectHeli_TakeOff(tp, twp);
		break;
	case MD_HELI_HOVER:
		ObjectHeli_Hover(tp, twp);
		break;
	case MD_HELI_MOVE:
		ObjectHeli_Move(twp);
		break;
	case MD_HELI_STOP:
		ObjectHeli_Stop(twp);
		break;
	}

	twp->scl.x += twp->scl.y;
	
	EntryColliList(twp);
	ObjectSetupInput(twp, 0);

	pathinfo info;
	HeliPath(twp, &pathtag_hw1_heli0807, &info);
	HeliWriteSub_m(tp, twp);

	twp->pos = heli_pos;
	twp->ang.y = HeliStackA;

	if (twp->mode != MD_HELI_GROUND)
	{
		dsPlay_Dolby_time(96, (int)twp, 1, 0, 2, twp);
	}
}

static void __cdecl ObjectHeliExec_r(task* tp);
Trampoline ObjectHeliExec_t(0x6139F0, 0x6139FA, ObjectHeliExec_r);
static void __cdecl ObjectHeliExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectHeliExec_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjectHeliExec)(tp);
	}
}
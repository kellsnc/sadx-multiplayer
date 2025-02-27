#include "pch.h"
#include "UsercallFunctionHandler.h"
#include "multiplayer.h"

// Rolling rock in Lost World

static auto SmokeInit = GenerateUsercallWrapper<void (*)(taskwk* twp, int AA, float FF)>(noret, 0x5E3C40, rEAX, rESI, stack4);
static auto cSmokeInit = GenerateUsercallWrapper<void (*)(taskwk* cTaskWork, int AA, float FF)>(noret, 0x5E3C10, rESI, rEDI, stack4);

TaskFunc(C_Smoke, 0x5E3B90);
TaskFunc(C_Fire, 0x5E3AC0);
DataArray(int, FirePosi, 0x2038D40, 8);

static void __cdecl ObjectRockRoll_Normal_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectRockRoll_Normal_r)> ObjectRockRoll_Normal_t(0x5E3CB0);

static void Normal_m(task* tp)
{
	auto twp = tp->twp;

	if (GetStageNumber() != twp->timer.w[0])
	{
		//SetFreeCameraMode(1);
		FreeTask(tp);
		return;
	}

	auto pnum = GetTheNearestPlayerNumber(&twp->pos);
	auto ptwp = playertwp[pnum];

	if (!ptwp)
	{
		return;
	}

	if ((twp->cwp->flag & 1) && twp->cwp->hit_cwp == ptwp->cwp)
	{
		NJS_POINT3 v = { 5.0f, 1.0f, 0.0f };
		Angle3 ang = { 0, 0x4000, 0 };
		SetVelocityAndRotationAndNoconTimeP(pnum, &v, &ang, 60);
	}

	twp->scl.y += twp->scl.x;

	if (twp->scl.y >= 5755.0f)
	{
		FreeTask(tp);
		return;
	}

	twp->ang.z -= twp->counter.l;

	pathinfo pi;
	pi.onpathpos = twp->scl.y;
	GetStatusOnPath((pathtag*)0x20338F4, &pi);
	twp->pos.x = pi.xpos;
	twp->pos.y = pi.ypos;
	twp->pos.z = pi.zpos;

	if (!(twp->flag & 0x100))
	{
		if (twp->pos.x + 200.0f < ptwp->pos.x)
		{
			twp->flag |= 0x100;
		}
	}
	else
	{
		dsPlay_timer_v(199, (int)twp, 1, 96, 2, twp->pos.x, twp->pos.y, twp->pos.z);
		if (twp->pos.x + 200.0f < ptwp->pos.x)
		{
			//SetFreeCameraMode(0);
			twp->scl.x += 0.15f;
			twp->counter.l += 24;
		}
		else if (twp->scl.x > 1.5f)
		{
			twp->scl.x -= 0.3f;
			twp->counter.l -= 48;
		}
	}

	LoopTaskC(tp);

	if (!(twp->flag & 0x200) && twp->scl.y >= 150.0f)
	{
		twp->flag |= 0x200;

		SmokeInit(twp, -9102, -35.0f);
		SmokeInit(twp, -7281, -30.0f);
		SmokeInit(twp, -5461, -25.0f);
		SmokeInit(twp, -3640, -20.0f);
		SmokeInit(twp, -1820, -15.0f);
		SmokeInit(twp, 0, 0.0f);
		SmokeInit(twp, 1820, 15.0f);
		SmokeInit(twp, 3640, 20.0f);
		SmokeInit(twp, 5461, 25.0f);
		SmokeInit(twp, 7281, 30.0f);
		SmokeInit(twp, 9102, 35.0f);

		auto ctp = CreateChildTask(2u, C_Smoke, tp);
		cSmokeInit(ctp->twp, -9102, -25.0f);
		ctp->twp->btimer = 0;
		ctp = CreateChildTask(2u, C_Smoke, tp);
		cSmokeInit(ctp->twp, -15473, 5.0f);
		ctp->twp->btimer = 4;
		ctp = CreateChildTask(2u, C_Smoke, tp);
		cSmokeInit(ctp->twp, 15473, -5.0f);
		ctp->twp->btimer = 2;
		ctp = CreateChildTask(2u, C_Smoke, tp);
		cSmokeInit(ctp->twp, 9102, 25.0f);
		ctp->twp->btimer = 6;
	}

	if ((float)FirePosi[twp->btimer] < twp->scl.y)
	{
		twp->btimer += 1i8;
		auto ctp = CreateChildTask(2u, C_Fire, tp);
		ctp->twp->ang.z = twp->ang.z;
		ctp->twp->btimer = 0i8;
		ctp->twp->scl.y = 0.0f;
		ctp->twp->scl.z = 0.0f;
		ctp->twp->timer.f = (float)(njRandom() * 50.0 - 25.0);
	}

	EntryColliList(twp);
	ObjectSetupInput(twp, 0);

	tp->disp(tp);
}

static void __cdecl ObjectRockRoll_Normal_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		Normal_m(tp);
	}
	else
	{
		ObjectRockRoll_Normal_t.Original(tp);
	}
}

void patch_rockroll_init()
{
	ObjectRockRoll_Normal_t.Hook(ObjectRockRoll_Normal_r);
}

RegisterPatch patch_rockroll(patch_rockroll_init);
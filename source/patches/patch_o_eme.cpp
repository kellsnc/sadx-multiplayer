#include "pch.h"
#include "multiplayer.h"
#include "emeraldhunt.h"

auto ObjectEmeraldDisplay = GenerateUsercallWrapper<void (*)(Angle* angy, NJS_POINT3* pos)>(noret, 0x4A2C70, rEAX, stack4);

static void __cdecl ObjectEmeraldPRegular_r(task* tp);
static void __cdecl FragmEmeraldDigDisplay_r(task* tp);

FastFunctionHookPtr<decltype(&ObjectEmeraldPRegular_r)> ObjectEmeraldPRegular_h(0x4A2FD0);
FastFunctionHookPtr<decltype(&FragmEmeraldDigDisplay_r)> FragmEmeraldDigDisplay_h(0x4A31D0);

static void ObjectEmeraldPRegular_m(task* tp)
{
	if (CheckRangeOut(tp))
	{
		return;
	}

	auto twp = tp->twp;

	if (GetPlayerNumber() != 3 || !Knuckles_KakeraGame_Set_CheckEme(twp->ang.z, &twp->pos))
	{
		DeadOut(tp);
		return;
	}

	if ((twp->ang.z & 0xF0) == 32)
	{
		auto pnum = IsPlayerInSphere(&twp->pos, 30.0f) - 1;

		if (pnum >= 0 && Knuckles_Status(pnum) == 2)
		{
			Knuckles_KakeraGame_Set_PutEme_m(pnum, twp->ang.z, &twp->pos);
			DeadOut(tp);
			return;
		}

		tp->disp = nullptr;
		return;
	}
	else
	{
		auto pnum = IsPlayerInSphere(twp->pos.x, twp->pos.y - 5.0f, twp->pos.z, 10.0f) - 1;

		if (pnum >= 0)
		{
			Knuckles_KakeraGame_Set_PutEme_m(pnum, twp->ang.z, &twp->pos);
			DeadOut(tp);
			return;
		}

		tp->disp = CallObjectEmeraldDisplay;
		tp->disp(tp);
	}
}

static void __cdecl ObjectEmeraldPRegular_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectEmeraldPRegular_m(tp);
	}
	else
	{
		ObjectEmeraldPRegular_h.Original(tp);
	}
}

// No display so we add one:
static void __cdecl FragmEmeraldDigDisplayDisp(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 2i8 && twp->wtimer > 0x3Cu || (twp->wtimer & 2))
	{
		return;
	}

	ObjectEmeraldDisplay(&twp->ang.y, &twp->pos);
}

static void FragmEmeraldDigDisplay_m(task* tp)
{
	auto twp = tp->twp;
	auto ptwp = playertwp[twp->btimer];

	if (!ptwp)
	{
		DeadOut(tp);
		return;
	}

	if (!tp->disp)
	{
		tp->disp = FragmEmeraldDigDisplayDisp;
	}

	switch (twp->mode)
	{
	case 0i8:
		twp->scl.x = 0.0;
		twp->scl.y = 2.0;
		twp->scl.z = 0.0;
		twp->mode = 1i8;
		break;
	case 1i8:
		twp->scl.y -= 0.1f;
		if (twp->scl.y > 0.0f)
		{
			twp->pos.y += twp->scl.y;
		}
		else
		{
			twp->scl.x = twp->pos.x - ptwp->pos.x;
			twp->scl.y = twp->pos.y - ptwp->pos.y + 20.0f;
			twp->scl.z = twp->pos.z - ptwp->pos.z;
			twp->mode = 2i8;
		}
		break;
	case 2i8:
		twp->scl.x *= 0.95f;
		twp->scl.y *= 0.95f;
		twp->scl.z *= 0.95f;

		twp->pos.x = twp->scl.x + ptwp->pos.x;
		twp->pos.y = twp->scl.y + ptwp->pos.y + 20.0f;
		twp->pos.z = twp->scl.z + ptwp->pos.z;

		if (++twp->wtimer > 0x78u)
		{
			FreeTask(tp);
			return;
		}

		if (twp->wtimer > 0x3Cu || (twp->wtimer & 2))
		{
			NJS_VECTOR velo;
			velo.x = (float)(njRandom() - 0.5);
			velo.y = (float)(njRandom() - 0.5);
			velo.z = (float)(njRandom() - 0.5);
			if (njUnitVector(&velo) == 0.0f)
			{
				velo = { 0.0f, 0.0f, 0.0f };
			}
			NJS_VECTOR pos;
			pos.x = ptwp->pos.x - velo.x * 5.0f;
			pos.y = ptwp->pos.x - velo.y * 5.0f;
			pos.z = ptwp->pos.x - velo.z * 5.0f;
			CreateKiranR(&pos, &velo, 0.08f, 910);
		}
		break;
	}

	tp->disp(tp);
}

static void __cdecl FragmEmeraldDigDisplay_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		FragmEmeraldDigDisplay_m(tp);
	}
	else
	{
		FragmEmeraldDigDisplay_h.Original(tp);
	}
}

void patch_eme_init()
{
	ObjectEmeraldPRegular_h.Hook(ObjectEmeraldPRegular_r);
	FragmEmeraldDigDisplay_h.Hook(FragmEmeraldDigDisplay_r);
}

RegisterPatch patch_eme(patch_eme_init);
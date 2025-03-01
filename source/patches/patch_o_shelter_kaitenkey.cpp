#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "VariableHook.hpp"
#include "multiplayer.h"

// Amy key handles in Final Egg
// It's done properly in the player's task, but the object itself is hardcoded to p1

FastUsercallHookPtr<void(*)(taskwk*), noret, rEAX> HandleTest_h(0x5A2840);
FastFunctionHook<Bool, Angle*> AmyHndlGetHandleAngle_h(0x4C56D0);

VariableHook<amyhndlstr, 0x3C5B300> amyhndlstatus_m; // Add handle information for all players

static auto KeitenCount = GenerateUsercallWrapper<void (*)(int, int)>(noret, 0x5A26C0, rEAX, rECX);

amyhndlstr* __cdecl AmyHndlGetInfoStrP_r() // Get handle information from the player task
{
	if (multiplayer::IsActive())
	{
		return &amyhndlstatus_m[TASKWK_PLAYERID(gpCharTwp)];
	}
	else
	{
		return &amyhndlstatus_m[0];
	}
}

static bool AmyHndlGetHandleAngle_m(int pnum, Angle* ang)
{
	auto x = (double)(per[pnum]->x1 << 8);
	auto y = (double)(-256 * per[pnum]->y1);

	if (x * x + y * y <= 225000000.0)
	{
		return false;
	}

	*ang = NJM_RAD_ANG(atan2(x, y));

	return true;
}

static Bool AmyHndlGetHandleAngle_r(Angle* ang)
{
	if (multiplayer::IsActive())
	{
		return AmyHndlGetHandleAngle_m(TASKWK_PLAYERID(gpCharTwp), ang) ? TRUE : FALSE;
	}
	else
	{
		return AmyHndlGetHandleAngle_h.Original(ang);
	}
}

static amyhndlstr* AmyHndlEntryTouchHandle_m(int pnum, NJS_POINT3* hndlpos, int hndlangy)
{
	auto ptwp = playertwp[pnum];

	if (ptwp && TASKWK_CHARID(ptwp) == Characters_Amy)
	{
		auto& hndl = amyhndlstatus_m[TASKWK_PLAYERID(ptwp)];

		if (hndlpos)
		{
			hndl.hndlpos = *hndlpos;
		}

		hndl.hndlangy = hndlangy;
		hndl.hndlmode = 0;
		hndl.touchflag = 0;

		if (ptwp->mode == 15)
		{
			hndl.hndlmode = 1; // grab
			Angle turnang;
			if (AmyHndlGetHandleAngle_m(pnum, &turnang))
			{
				hndl.touchflag = 1;
				hndl.turnang = turnang;
			}
		}

		auto ppwp = playerpwp[pnum];

		if (ppwp)
		{
			ppwp->free.sb[6] |= 1u;
			return &hndl;
		}
		else
		{
			hndl.hndlmode = 0;
			hndl.touchflag = 0;
		}
	}

	return nullptr;
}

static int PlayerRange_m(taskwk* twp) // Get if player is in front of the handle, modified for all players
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			NJS_POINT3 v = ptwp->pos;
			njSubVector(&v, &twp->pos);
			njPushMatrix(_nj_unit_matrix_);
			ROTATEY(0, -twp->ang.y);
			njCalcVector(0, &v, &v);
			njPopMatrixEx();

			if (v.x > -5.0f && v.x < 5.0f && v.z > 3.0f && v.z < 10.0f)
			{
				return i + 1;
			}
		}
	}

	return 0;
}

static void HandleTest_m(taskwk* twp)
{
	auto pnum = PlayerRange_m(twp) - 1;

	if (pnum >= 0)
	{
		auto hndl = AmyHndlEntryTouchHandle_m(pnum, &twp->pos, twp->ang.y);

		if (hndl)
		{
			auto keybuff = &KeyBuff[twp->value.b[0]];

			if (hndl->touchflag == 1)
			{
				twp->counter.l = hndl->turnang;

				KeitenCount(hndl->turnang, twp->value.b[0]);

				if (keybuff->RightCount > twp->value.b[1])
					keybuff->RightCount = 0;
				if (keybuff->LeftCount > twp->value.b[2])
					keybuff->LeftCount = 0;
			}
			else
			{
				keybuff->Buff[keybuff->Point] = 0;
				keybuff->Point = (keybuff->Point + 1) & 3;
			}
		}
	}
}

static void __cdecl HandleTest_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		HandleTest_m(twp);
	}
	else
	{
		HandleTest_h.Original(twp);
	}
}

void patch_shelter_kaitenkey_init()
{
	WriteJump((void*)0x4C5800, AmyHndlGetInfoStrP_r); // Too small to hook
	AmyHndlGetHandleAngle_h.Hook(AmyHndlGetHandleAngle_r);
	HandleTest_h.Hook(HandleTest_r);
}

RegisterPatch patch_shelter_kaitenkey(patch_shelter_kaitenkey_init);
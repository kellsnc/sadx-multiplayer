#include "pch.h"
#include "multiplayer.h"

static void __cdecl ERSC_CommonExec_r(task* tp);
static void __cdecl AmyERobApperChecker_r(task* tp);
static BOOL __cdecl ERobStart_r(erctrlstr* cmd);
static void __cdecl EggRob_r(task* tp);

Trampoline ERSC_CommonExec_t(0x7B0640, 0x7B064A, ERSC_CommonExec_r);
Trampoline AmyERobApperChecker_t(0x486980, 0x486985, AmyERobApperChecker_r);
Trampoline ERobStart_t(0x4B3EB0, 0x4B3EB5, ERobStart_r);
Trampoline EggRob_t(0x4D2960, 0x4D296A, EggRob_r);

DataPointer(task*, er_tp, 0x3C5815C);
DataPointer(int, er_syorikaru, 0x3C5C838);

static void __cdecl EggRob_r(task* tp) // no way in hell I'm rewriting that
{
	if (multiplayer::IsActive())
	{
		auto backup_ptr = playertwp[0];
		playertwp[0] = playertwp[GetClosestPlayerNum(&tp->twp->pos)];
		TARGET_STATIC(EggRob)(tp);
		playertwp[0] = backup_ptr;
	}
	else
	{
		TARGET_STATIC(EggRob)(tp);
	}
}

static task* Eggrob_GenerateEggrob_m(erctrlstr* cmd)
{
	auto tp = CreateElementalTask(2u, LEV_3, Eggrob_Init);

	if (!tp)
	{
		return nullptr;
	}

	er_syorikaru = GetStageNumber() >= LevelAndActIDs_TwinklePark2 && GetStageNumber() <= LevelAndActIDs_TwinklePark3 ? 1 : 0;

	auto twp = tp->twp;

	switch (cmd->command)
	{
	case 0:
	{
		twp->pos = cmd->pos0;
		twp->pos.y += 5.0f;
		float x = cmd->pos1.x - cmd->pos0.x;
		float z = cmd->pos1.z - cmd->pos0.z;
		twp->ang.y = NJM_RAD_ANG(atan2(z, -x));
		twp->timer.f = sqrtf(x * x + z * z) * 0.33333334f;

		twp->scl.y = 0.0f;
		twp->scl.z = 0.0f;
		twp->mode = 11i8;
		break;
	}
	case 1:
	{
		twp->pos = cmd->pos0;
		auto ptwp = playertwp[GetTheNearestPlayerNumber(&twp->pos)];

		if (ptwp)
		{
			twp->ang.y = NJM_RAD_ANG(atan2(ptwp->pos.z - cmd->pos0.z, -(ptwp->pos.x - cmd->pos0.x)));
		}

		twp->mode = 1i8;
		break;
	}
	case 2:
		twp->pos = cmd->pos0;
		twp->pos.y += 5.0f;
		float x = cmd->pos1.x - cmd->pos0.x;
		float z = cmd->pos1.z - cmd->pos0.z;
		twp->ang.y = NJM_RAD_ANG(atan2(z, -x));
		twp->timer.f = sqrtf(x * x + z * z) * 0.33333334f;

		twp->scl = { 1.0f, 4.0f, 0.0f };
		twp->mode = 5i8;
		break;
	}

	// Rumble nearby players
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp && GetDistance(&ptwp->pos, &twp->pos) < 100.0f)
		{
			RumbleA(i, 0);
		}
	}

	return tp;
}

static bool ERobStart_m(erctrlstr* cmd)
{
	if (er_tp)
	{
		FreeTask(er_tp);
		er_tp = nullptr;
	}
	er_tp = Eggrob_GenerateEggrob_m(cmd);
	return er_tp != nullptr;
}

static BOOL __cdecl ERobStart_r(erctrlstr* cmd)
{
	if (multiplayer::IsActive())
	{
		return ERobStart_m(cmd) ? TRUE : FALSE;
	}
	else
	{
		return TARGET_STATIC(ERobStart)(cmd);
	}
}

static void __cdecl AmyERobApperChecker_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (IsPlayerInSphere(&twp->pos, twp->value.f))
		{
			erctrlstr cmd{};

			if (twp->wtimer == 2)
			{
				cmd.command = 2;
				cmd.pos0 = twp->scl;
				cmd.pos1 = twp->pos;
			}

			ERobStart_m(&cmd);
			FreeTask(tp);
		}
	}
	else
	{
		TARGET_STATIC(AmyERobApperChecker)(tp);
	}
}

static void __cdecl ERSC_CommonExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto backup_ptr = playertwp[0];
		playertwp[0] = playertwp[GetTheNearestPlayerNumber(&tp->twp->pos)];
		TARGET_STATIC(ERSC_CommonExec)(tp);
		playertwp[0] = backup_ptr;
	}
	else
	{
		TARGET_STATIC(ERSC_CommonExec)(tp);
	}
}
#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"

#define EGGROB_PNUM(twp) twp->smode

static void __cdecl ERSC_CommonExec_r(task* tp);
static void __cdecl AmyERobApperChecker_r(task* tp);
static BOOL __cdecl ERobStart_r(erctrlstr* cmd);
static void __cdecl EggRob_r(task* tp);
static void __cdecl Eggrob_Display_r(task* tp);
static void __cdecl Eggrob_LockOnCursor_r(task* tp);
static void __cdecl Eggrob_BeamZanzo_r(task* tp);
static void __cdecl Eggrob_BeamKonseki_r(task* tp);

Trampoline ERSC_CommonExec_t(0x7B0640, 0x7B064A, ERSC_CommonExec_r);
Trampoline AmyERobApperChecker_t(0x486980, 0x486985, AmyERobApperChecker_r);
Trampoline ERobStart_t(0x4B3EB0, 0x4B3EB5, ERobStart_r);
Trampoline EggRob_t(0x4D2960, 0x4D296A, EggRob_r);
Trampoline Eggrob_Display_t(0x4CFB70, 0x4CFB75, Eggrob_Display_r);
Trampoline Eggrob_LockOnCursor_t(0x4D01B0, 0x4D01B8, Eggrob_LockOnCursor_r);
Trampoline Eggrob_BeamZanzo_t(0x4CFEE0, 0x4CFEE8, Eggrob_BeamZanzo_r);
Trampoline Eggrob_BeamKonseki_t(0x4D00E0, 0x4D00E6, Eggrob_BeamKonseki_r);

DataPointer(task*, er_tp, 0x3C5815C);
DataPointer(int, er_syorikaru, 0x3C5C838);
DataArray(NJS_POINT3, konseki_p, 0x3C5C810, 2);
DataArray(NJS_COLOR, konseki_c, 0x9BE5A0, 2);
DataArray(NJS_POINT3, zanzo_p, 0x3C5C6AC, 4);
DataArray(NJS_COLOR, zanzo_c, 0x9BE580, 4);

static void __cdecl Eggrob_BeamKonseki_disp(task* tp)
{
	if (!MissedFrames && tp->twp->wtimer < 45ui16 && SplitScreen::IsActive() && SplitScreen::GetCurrentScreenNum() != 0)
	{
		auto twp = tp->twp;
		konseki_p[0] = twp->pos;
		konseki_p[1] = twp->scl;

		float idk = (float)(45 - twp->wtimer) / 45.0f;
		konseki_c[0].argb.a = (uint8_t)(255.0f * idk);
		konseki_c[1].argb.a = (uint8_t)((idk - (1.0f / 45.0f)) * 255.0f);

		DrawLine3D_Queue((NJS_POINT3COL*)0x9BE5A8, 4, NJD_TRANSPARENT | NJD_FILL, QueuedModelFlagsB_EnableZWrite);
	}
}

static void __cdecl Eggrob_BeamKonseki_r(task* tp)
{
	if (!tp->disp && multiplayer::IsActive())
	{
		tp->disp = Eggrob_BeamKonseki_disp;
	}

	TARGET_STATIC(Eggrob_BeamKonseki)(tp);
}

static void __cdecl Eggrob_BeamZanzo_disp(task* tp)
{
	if (!MissedFrames && tp->twp->wtimer < 9ui16 && SplitScreen::IsActive() && SplitScreen::GetCurrentScreenNum() != 0)
	{
		auto twp = tp->twp;

		zanzo_p[0] = twp->pos;
		zanzo_p[1] = twp->scl;
		zanzo_p[2] = { twp->counter.f, twp->timer.f, twp->value.f };
		zanzo_p[3].x = (float)twp->ang.x * 0.000015258789f + twp->pos.x;
		zanzo_p[3].y = (float)twp->ang.y * 0.000015258789f + twp->pos.y;
		zanzo_p[3].z = (float)twp->ang.z * 0.000015258789f + twp->pos.z;

		float v4 = (float)(9 - twp->wtimer) / 9.0f;
		zanzo_c[0].argb.a = v4 * 192.0f;
		zanzo_c[1].argb.a = (uint8_t)((1.0f - GetDistance(&twp->pos, &twp->scl) * 0.0066666668f) * (float)zanzo_c[0].argb.a);
		zanzo_c[3].argb.a = (uint8_t)((v4 - (1.0f / 9.0f)) * 192.0f);
		zanzo_c[2].argb.a = (uint8_t)((1.0f - GetDistance(&twp->pos, &zanzo_p[2]) * 0.0066666668f) * (float)zanzo_c[3].argb.a);

		njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
		DrawTriFanThing_Queue((NJS_POINT3COL*)0x9BE590, 4, NJD_TRANSPARENT, QueuedModelFlagsB_EnableZWrite);
		ghDefaultBlendingMode();
	}
}

static void __cdecl Eggrob_BeamZanzo_r(task* tp)
{
	if (!tp->disp && multiplayer::IsActive())
	{
		tp->disp = Eggrob_BeamZanzo_disp;
	}

	TARGET_STATIC(Eggrob_BeamZanzo)(tp);
}

static void __cdecl Eggrob_LockOnCursor_disp(task* tp)
{
	if (!MissedFrames && SplitScreen::IsActive() && SplitScreen::GetCurrentScreenNum() != 0)
	{
		auto twp = tp->twp;
		ghDefaultBlendingMode();
		njPushMatrixEx();
		njTranslateEx((NJS_POINT3*)0x3C5C61C);
		njRotateX_(twp->ang.x);
		njRotateY_(twp->ang.y);
		njScale(0, twp->scl.x, twp->scl.x, twp->scl.x);
		DrawLine3D_Queue((NJS_POINT3COL*)0x9BE678, 6, NJD_TRANSPARENT, QueuedModelFlagsB_EnableZWrite);
		njPopMatrixEx();
	}
}

static void __cdecl Eggrob_LockOnCursor_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		if (!tp->disp)
			tp->disp = Eggrob_LockOnCursor_disp; // <- please stop not putting a display

		if (er_tp)
		{
			auto backup_ptr = playertwp[0];
			playertwp[0] = playertwp[EGGROB_PNUM(er_tp->twp)];
			TARGET_STATIC(Eggrob_LockOnCursor)(tp); // one occurence of playertwp[0] in the middle, todo: rewrite
			playertwp[0] = backup_ptr;
		}
	}

	TARGET_STATIC(Eggrob_LockOnCursor)(tp);
}

static void __cdecl Eggrob_Display_r(task* tp)
{
	TARGET_STATIC(Eggrob_Display)(tp);

	if (tp->twp->mode == 17 && !MissedFrames && SplitScreen::IsActive() && SplitScreen::GetCurrentScreenNum() != 0)
	{
		njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
		DrawLine3D_Queue((NJS_POINT3COL*)0x9BE570, 1, NJD_TRANSPARENT, QueuedModelFlagsB_EnableZWrite);
		ghDefaultBlendingMode();
	}
}

static void __cdecl EggRob_r(task* tp) // no way in hell I'm rewriting that
{
	if (multiplayer::IsActive())
	{
		EGGROB_PNUM(tp->twp) = GetClosestPlayerNum(&tp->twp->pos);
		auto backup_ptr = playertwp[0];
		playertwp[0] = playertwp[EGGROB_PNUM(tp->twp)];
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
		TARGET_STATIC(ERSC_CommonExec)(tp); // todo: rewrite
		playertwp[0] = backup_ptr;
	}
	else
	{
		TARGET_STATIC(ERSC_CommonExec)(tp);
	}
}
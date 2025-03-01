#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"

/*

Zero in levels
Patched most objects because they lacked display functions
Hack to get zero to aim the closest player

*/

#define EGGROB_PNUM(twp) twp->smode

static void __cdecl ERSC_CommonExec_r(task* tp);
static void __cdecl AmyERobApperChecker_r(task* tp);
static Bool __cdecl ERobStart_r(erctrlstr* cmd);
static void __cdecl EggRob_r(task* tp);
static void __cdecl Eggrob_Display_r(task* tp);
static void __cdecl Eggrob_LockOnCursor_r(task* tp);
static void __cdecl Eggrob_BeamZanzo_r(task* tp);
static void __cdecl Eggrob_BeamKonseki_r(task* tp);

FastFunctionHookPtr<decltype(&ERSC_CommonExec_r)> ERSC_CommonExec_h(0x7B0640);
FastFunctionHookPtr<decltype(&AmyERobApperChecker_r)> AmyERobApperChecker_h(0x486980);
FastFunctionHookPtr<decltype(&ERobStart_r)> ERobStart_h(0x4B3EB0);
FastFunctionHookPtr<decltype(&EggRob_r)> EggRob_h(0x4D2960);
FastFunctionHookPtr<decltype(&Eggrob_Display_r)> Eggrob_Display_h(0x4CFB70);
FastFunctionHookPtr<decltype(&Eggrob_LockOnCursor_r)> Eggrob_LockOnCursor_h(0x4D01B0);
FastFunctionHookPtr<decltype(&Eggrob_BeamZanzo_r)> Eggrob_BeamZanzo_h(0x4CFEE0);
FastFunctionHookPtr<decltype(&Eggrob_BeamKonseki_r)> Eggrob_BeamKonseki_h(0x4D00E0);

DataPointer(task*, er_tp, 0x3C5815C);
DataPointer(mtnjvwk, er_mwk, 0x3C5C658);
DataPointer(int, er_syorikaru, 0x3C5C838);
DataArray(NJS_POINT3, konseki_p, 0x3C5C810, 2);
DataArray(NJS_COLOR, konseki_c, 0x9BE5A0, 2);
DataArray(NJS_POINT3, zanzo_p, 0x3C5C6AC, 4);
DataArray(NJS_COLOR, zanzo_c, 0x9BE580, 4);
DataArray(NJS_POINT3, chain_pos, 0x3C5C700, 16);

static auto Eggrob_DrawChain = GenerateUsercallWrapper<void (*)(NJS_POINT3* p, NJS_POINT3* v)>(noret, 0x4D04C0, rEAX, rESI); // custom name

static void __cdecl Eggrob_BeamKonseki_disp(task* tp)
{
	if (!MissedFrames && tp->twp->wtimer < 45ui16 && splitscreen::IsActive() && splitscreen::GetCurrentScreenNum() != 0)
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

	Eggrob_BeamKonseki_h.Original(tp);
}

static void __cdecl Eggrob_BeamZanzo_disp(task* tp)
{
	if (!MissedFrames && tp->twp->wtimer < 9ui16 && splitscreen::IsActive() && splitscreen::GetCurrentScreenNum() != 0)
	{
		auto twp = tp->twp;

		zanzo_p[0] = twp->pos;
		zanzo_p[1] = twp->scl;
		zanzo_p[2] = { twp->counter.f, twp->timer.f, twp->value.f };
		zanzo_p[3].x = (float)twp->ang.x * 0.000015258789f + twp->pos.x;
		zanzo_p[3].y = (float)twp->ang.y * 0.000015258789f + twp->pos.y;
		zanzo_p[3].z = (float)twp->ang.z * 0.000015258789f + twp->pos.z;

		float v4 = (float)(9 - twp->wtimer) / 9.0f;
		zanzo_c[0].argb.a = (uint8_t)(v4 * 192.0f);
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

	Eggrob_BeamZanzo_h.Original(tp);
}

static void __cdecl Eggrob_LockOnCursor_disp(task* tp)
{
	if (!MissedFrames && splitscreen::IsActive() && splitscreen::GetCurrentScreenNum() != 0)
	{
		auto twp = tp->twp;
		ghDefaultBlendingMode();
		njPushMatrixEx();
		njTranslateEx((NJS_POINT3*)0x3C5C61C);
		ROTATEX(0, twp->ang.x);
		ROTATEY(0, twp->ang.y);
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
			tp->disp = Eggrob_LockOnCursor_disp;

		if (er_tp)
		{
			auto backup_ptr = playertwp[0];
			playertwp[0] = playertwp[EGGROB_PNUM(er_tp->twp)];
			Eggrob_LockOnCursor_h.Original(tp); // one occurence of playertwp[0] in the middle, todo: rewrite
			playertwp[0] = backup_ptr;
		}
	}
	else
	{
		Eggrob_LockOnCursor_h.Original(tp);
	}
}

static void DrawWeapon(taskwk* twp)
{
	if (twp->mode == 19 || twp->mode == 20)
	{
		//Eggrob_DrawChain:
		for (int i = 0; i < 15; ++i)
		{
			Eggrob_DrawChain(&chain_pos[i + 1], &chain_pos[i]);
		}

		Eggrob_DrawChain((NJS_POINT3*)0x3C5C684, &chain_pos[15]);

		// Eggrob_DrawRPunchUnit:
		(*(NJS_OBJECT*)0x98B6EC).evalflags &= ~8u;
		(*(NJS_OBJECT*)0x98BC6C).evalflags &= ~8u;
		(*(NJS_OBJECT*)0x98B98C).evalflags &= ~8u;
		(*(NJS_OBJECT*)0x98B3C4).evalflags &= ~8u;
		njPushMatrixEx();
		___dsSetPalette(6);
		njTranslateEx((NJS_POINT3*)0x3C5C648);
		ROTATEY(0, *(Angle*)0x3C5C834);
		njRotateX(0, 0x8000);
		ROTATEZ(0, *(Angle*)0x3C5C654);
		njSetTexture(&EGGROB_TEXLIST);
		dsDrawObject((NJS_OBJECT*)0x98B98C);
		___dsSetPalette(0);
		njPopMatrixEx();
		(*(NJS_OBJECT*)0x98B6EC).evalflags |= 8u;
		(*(NJS_OBJECT*)0x98BC6C).evalflags |= 8u;
		(*(NJS_OBJECT*)0x98B98C).evalflags |= 8u;
		(*(NJS_OBJECT*)0x98B3C4).evalflags |= 8u;
	}
}

static void DrawMainLaser(taskwk* twp)
{
	if (twp->mode == 17 && er_mwk.reqaction == 12 && er_mwk.nframe >= 19.0f)
	{
		njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
		DrawLine3D_Queue((NJS_POINT3COL*)0x9BE570, 1, NJD_TRANSPARENT, QueuedModelFlagsB_EnableZWrite);
		ghDefaultBlendingMode();
	}
}

static void __cdecl Eggrob_Display_r(task* tp)
{
	Eggrob_Display_h.Original(tp);

	if (!MissedFrames && splitscreen::IsActive() && splitscreen::GetCurrentScreenNum() != 0)
	{
		DrawMainLaser(tp->twp);
		DrawWeapon(tp->twp);
	}
}

static void __cdecl EggRob_r(task* tp) // no way in hell I'm rewriting that
{
	if (multiplayer::IsActive())
	{
		EGGROB_PNUM(tp->twp) = GetClosestPlayerNum(&tp->twp->pos);
		auto backup_ptr = playertwp[0];
		playertwp[0] = playertwp[EGGROB_PNUM(tp->twp)];
		EggRob_h.Original(tp);
		playertwp[0] = backup_ptr;
	}
	else
	{
		EggRob_h.Original(tp);
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
		twp->timer.l = static_cast<int>(sqrtf(x * x + z * z) * 0.33333334f);

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
			VibShot(i, 0);
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

static Bool __cdecl ERobStart_r(erctrlstr* cmd)
{
	if (multiplayer::IsActive())
	{
		return ERobStart_m(cmd) ? TRUE : FALSE;
	}
	else
	{
		return ERobStart_h.Original(cmd);
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
		AmyERobApperChecker_h.Original(tp);
	}
}

static void __cdecl ERSC_CommonExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto backup_ptr = playertwp[0];
		playertwp[0] = playertwp[GetTheNearestPlayerNumber(&tp->twp->pos)];
		ERSC_CommonExec_h.Original(tp); // todo: rewrite
		playertwp[0] = backup_ptr;
	}
	else
	{
		ERSC_CommonExec_h.Original(tp);
	}
}

void patch_eggrob_init()
{
	ERSC_CommonExec_h.Hook(ERSC_CommonExec_r);
	AmyERobApperChecker_h.Hook(AmyERobApperChecker_r);
	ERobStart_h.Hook(ERobStart_r);
	EggRob_h.Hook(EggRob_r);
	Eggrob_Display_h.Hook(Eggrob_Display_r);
	Eggrob_LockOnCursor_h.Hook(Eggrob_LockOnCursor_r);
	Eggrob_BeamZanzo_h.Hook(Eggrob_BeamZanzo_r);
	Eggrob_BeamKonseki_h.Hook(Eggrob_BeamKonseki_r);
}

RegisterPatch patch_eggrob(patch_eggrob_init);
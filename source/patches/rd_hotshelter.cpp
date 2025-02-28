#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"
#include "teleport.h"

DataPointer(NJS_POINT3, tunnel_pos, 0x17D0AE8);
DataPointer(Bool, tunnel_flag, 0x3C72A78);
DataPointer(Bool, clear_flag, 0x3C72A7C);

static void __cdecl ObjShelterFadeDisp_r(task* tp); // "Disp"
static void __cdecl ObjShelterTunnelscrollExec_r(task* tp); // "Exec"
static void __cdecl ObjShelterTunnelscrollDisp_r(task* tp); // "Disp"
static void __cdecl ObjShelterTunnelcolExec_r(task* tp); // "Exec"
static void __cdecl TunnelManagerExec_r(task* tp); // "Exec"

FastFunctionHookPtr<decltype(&ObjShelterFadeDisp_r)> ObjShelterFadeDisp_t(0x5ABB80);
FastFunctionHookPtr<decltype(&ObjShelterTunnelscrollExec_r)> ObjShelterTunnelscrollExec_t(0x5AC3B0);
FastFunctionHookPtr<decltype(&ObjShelterTunnelscrollDisp_r)> ObjShelterTunnelscrollDisp_t(0x5AC2F0);
FastFunctionHookPtr<decltype(&ObjShelterTunnelcolExec_r)> ObjShelterTunnelcolExec_t(0x5AC050);
FastFunctionHookPtr<decltype(&TunnelManagerExec_r)> TunnelManagerExec_t(0x59AD50);

#pragma region ObjShelterFade
static void ObjShelterFadeDisp_m(task* tp)
{
	if (MissedFrames)
	{
		return;
	}

	auto twp = tp->twp;
	auto pnum = twp->btimer;

	if (SplitScreen::GetCurrentScreenNum() == pnum)
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);

		ghDefaultBlendingMode();
		SetMaterial(twp->counter.f, 0.0, 0.0, 0.0);

		auto ratio = SplitScreen::GetScreenRatio(twp->btimer);

		sprite_fade.p.x = HorizontalStretch * 320.0f * ratio->w + ratio->x * HorizontalResolution;
		sprite_fade.p.y = VerticalStretch * 240.0f * ratio->h + ratio->y * VerticalResolution;
		sprite_fade.sx = (ratio->w * HorizontalStretch * 640.0f) * 0.015625f;
		sprite_fade.sy = (ratio->h * VerticalStretch * 480.0f) * 0.015625f;

		late_DrawSprite2D(&sprite_fade, 0, 22047.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, LATE_LIG);

		ResetMaterial();
		SplitScreen::RestoreViewPort();
	}
}

static void __cdecl ObjShelterFadeDisp_r(task* tp)
{
	if (SplitScreen::IsActive())
	{
		ObjShelterFadeDisp_m(tp);
	}
	else
	{
		ObjShelterFadeDisp_t.Original(tp);
	}
}

static void CreateObjShelterFade(char pnum)
{
	auto tp = CreateElementalTask(2u, 3, ObjShelterFade);
	tp->twp->btimer = pnum;
}
#pragma endregion

#pragma region ObjShelterTunnelscroll
static void __cdecl ObjShelterTunnelscrollDisp_r(task* tp)
{
	if (SplitScreen::IsActive() && tunnel_flag)
	{
		auto pnum = SplitScreen::GetCurrentScreenNum();

		if (pnum >= 0)
		{
			tp->twp->pos.z = playertwp[pnum]->pos.z;
		}
	}

	ObjShelterTunnelscrollDisp_t.Original(tp);
}

static void ObjShelterTunnelscrollExec_m(task* tp)
{
	if (tunnel_flag)
	{
		auto twp = tp->twp;

		if (clear_flag)
		{
			twp->timer.f = max(2.0f, twp->timer.f - 0.1f);
		}
		else if (twp->mode == 2)
		{
			twp->value.l += 1;
			if (twp->value.l > 600)
			{
				twp->value.l = 0;
				twp->mode = 3;
			}
		}
		else
		{
			twp->timer.f = min(20.0f, twp->timer.f + 0.1f);
		}

		twp->counter.f -= twp->timer.f;
		if (twp->counter.f <= 520.0f)
			twp->counter.f += 520.0f;

		tp->disp(tp);
	}
}

static void __cdecl ObjShelterTunnelscrollExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjShelterTunnelscrollExec_m(tp);
	}
	else
	{
		ObjShelterTunnelscrollExec_t.Original(tp);
	}
}
#pragma endregion

#pragma region ObjShelterTunnelcol
static void __cdecl ObjShelterTunnelcolExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (twp->wtimer != GetStageNumber())
		{
			FreeTask(tp);
			return;
		}

		if (tunnel_flag)
		{
			// Hack: make collision so large it works for both players
			auto obj = (NJS_OBJECT*)tp->twp->counter.ptr;
			obj->pos[2] = 1500.0f;
			obj->scl[2] = 20.0f;
			obj->basicdxmodel->r = 3000.0f;

			twp->flag |= 1;
		}
		else
		{
			twp->flag &= ~1;
		}
	}
	else
	{
		ObjShelterTunnelcolExec_t.Original(tp);
	}
}
#pragma endregion

#pragma region TunnelManager
static NJS_VECTOR end_pos = { -70.0f, 10007.0f, 3250.0f };

static void TunnelManagerExec_m(task* tp)
{
	auto twp = tp->twp;

	if (twp->wtimer != GetStageNumber())
	{
		tunnel_flag = FALSE;
		FreeTask(tp);
		return;
	}

	switch (twp->mode)
	{
	case 0:
		if (IsPlayerInsideSphere(&tunnel_pos, 3000.0f))
		{
			tunnel_flag = TRUE;
			twp->mode = 1;

			CreateElementalTask(2u, 3, ObjShelterTunnelscroll);
			CreateTunnelcol();
			CreateElementalTask(2u, 3, ObjShelterNo2cargo);
		}
		break;
	case 1:
		if (!IsPlayerInsideSphere(&tunnel_pos, 3000.0f))
		{
			tunnel_flag = FALSE;
			twp->mode = 0;
			break;
		}

		if (GetSwitchOnOff(16))
		{
			SetSwitchOnOff(16, 0);
			clear_flag = TRUE;
			twp->timer.l = 0;
			twp->mode = 2;
		}

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (playertwp[i] && playertwp[i]->pos.z > 3440.0f)
			{
				playertwp[i]->pos.z = 3440.0f;
			}
		}

		break;
	case 2:
		dsPlay_timer(325, (int)tp, 1, 0, 2);

		if (++twp->timer.l > 120)
		{
			twp->timer.l = 0;
			twp->mode = 3i8;

			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				if (playertwp[i] && GetDistance(&end_pos, &playertwp[i]->pos) < 200.0f)
				{
					CreateObjShelterFade(i);
					PadReadOffP(i);
				}
			}
		}
		break;
	case 3:
		dsPlay_timer(325, (int)tp, 1, 0, 2);
		++twp->timer.l;

		if (twp->timer.l > 170)
		{
			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				if (playertwp[i] && GetDistance(&end_pos, &playertwp[i]->pos) < 200.0f)
				{
					TeleportPlayer(i, -850.0f, 3030.0f, -3183.0f);
					PadReadOnP(i);
				}
			}

			twp->mode = 4;
			twp->timer.l = 0;
		}
		else if (twp->timer.l > 165)
		{
			for (int i = 0; i < PLAYER_MAX; ++i)
				if (playertwp[i] && GetDistance(&end_pos, &playertwp[i]->pos) < 200.0f)
					SetPositionP(i, end_pos.x, end_pos.y, end_pos.z);
		}
		break;
	case 4:
		if (twp->timer.l < 240)
		{
			++twp->timer.l;
		}
		else if (IsPlayerInsideSphere(&tunnel_pos, 3000.0f)) // Restart train if needed
		{
			clear_flag = FALSE;
			twp->mode = 1;
			twp->timer.l = 0;
		}
		break;
	}
}

void __cdecl TunnelManagerExec_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		TunnelManagerExec_m(tp);
	}
	else
	{
		ObjShelterTunnelscrollExec_t.Original(tp);
	}
}
#pragma endregion

void patch_rd_hotshelter_init()
{
	ObjShelterFadeDisp_t.Hook(ObjShelterFadeDisp_r);
	ObjShelterTunnelscrollExec_t.Hook(ObjShelterTunnelscrollExec_r);
	ObjShelterTunnelscrollDisp_t.Hook(ObjShelterTunnelscrollDisp_r);
	ObjShelterTunnelcolExec_t.Hook(ObjShelterTunnelcolExec_r);
	TunnelManagerExec_t.Hook(TunnelManagerExec_r);
}

RegisterPatch patch_rd_hotshelter(patch_rd_hotshelter_init);
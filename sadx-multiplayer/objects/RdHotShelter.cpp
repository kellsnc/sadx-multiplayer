#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"

DataPointer(int, tunnel_flag, 0x3C72A78);
DataPointer(int, clear_flag, 0x3C72A7C);

static void __cdecl ObjShelterFadeDisp_r(task* tp); // "Disp"
static void __cdecl ObjShelterTunnelscrollExec_r(task* tp); // "Exec"

Trampoline ObjShelterFadeDisp_t(0x5ABB80, 0x5ABB85, ObjShelterFadeDisp_r);
Trampoline ObjShelterTunnelscrollExec_t(0x5AC3B0, 0x5AC3B5, ObjShelterTunnelscrollExec_r);

#pragma region ObjShelterFade
static void ObjShelterFadeDisp_m(task* tp)
{
	if (MissedFrames)
	{
		return;
	}

	auto twp = tp->twp;
	auto pnum = twp->btimer - 1;

	if (pnum == -1)
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		TARGET_STATIC(ObjShelterFadeDisp)(tp);
		SplitScreen::RestoreViewPort();
	}
	else if (SplitScreen::GetCurrentScreenNum() == pnum)
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

		late_DrawSprite2D(&sprite_fade, 0, 22047.0f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, 4u);

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
		TARGET_STATIC(ObjShelterFadeDisp)(tp);
	}
}
#pragma endregion

#pragma region ObjShelterTunnelscroll
static void GetAveragePosition(NJS_VECTOR* v)
{
	*v = playertwp[0]->pos;

	for (int i = 1; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			v->x += ptwp->pos.x;
			v->y += ptwp->pos.y;
			v->z += ptwp->pos.z;
		}
	}

	auto pcount = GetPlayerCount();

	v->x /= pcount;
	v->y /= pcount;
	v->z /= pcount;
}

static void ObjShelterTunnelscrollExec_m(task* tp)
{
	if (tunnel_flag)
	{
		auto twp = tp->twp;
		NJS_VECTOR v;

		GetAveragePosition(&v);
		
		twp->pos.z += floorf((v.z - twp->pos.z) * 0.0019230769f) * 520.0f;

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
		TARGET_STATIC(ObjShelterTunnelscrollExec)(tp);
	}
}
#pragma endregion
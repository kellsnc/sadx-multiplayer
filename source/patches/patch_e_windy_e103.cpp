#include "pch.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "result.h"
#include "camera.h"

FastUsercallHookPtr<void(*)(task* tp), noret, rEAX> e103_move_h(0x4E6D00);
FastUsercallHookPtr<Bool(*)(task* tp, Float r), rEAX, rEAX, stack4> e103_chkPlayerRadius_h(0x4E6900);
FastFunctionHook<void, task*> e103_waitPlayer_h(0x4E6B90);
FastUsercallHookPtr<void(*)(task*, NJS_POINT3*, int), noret, rEAX, rECX, rEDI> e103_turnBody_h(0x4E6940);
FastUsercallHookPtr<void(*)(task* tp), noret, rESI> e103_chkDamage_h(0x4E6790);
FastUsercallHookPtr<void(*)(task* tp), noret, rEAX> e103_normal_h(0x4E6ED0);

static void SetE103Camera(task* tp)
{
	auto ewp = (enemywk*)tp->mwp;
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertwp[i])
		{
			if (GetDistance(&playertwp[i]->pos, &ewp->home) < 200.0f)
			{
				CameraSetEventCamera_m(i, CAMMD_E103, CAMADJ_NONE);
			}
		}
	}
}

#pragma region move
static void e103_move_m(task* tp)
{
	auto twp = tp->twp;
	auto wk = (TGT_WK*)tp->awp;

	wk->oldPos = twp->pos;

	NJS_VECTOR vec;
	if (ChkE104ColliLandXZ(&twp->pos, 5.0, &wk->fGroundY, &vec) == 1)
	{
		twp->pos.x += vec.x;
		twp->pos.y += vec.y;
		twp->pos.z += vec.z;
		wk->spd.x = -wk->spd.x;
		wk->spd.z = -wk->spd.z;
	}

	wk->fGroundY += 14.0f;
	wk->fGroundDist = twp->pos.y - wk->fGroundY;

	if (wk->flag & 4)
	{
		wk->spd.y -= 0.05f;
	}

	twp->pos.y += wk->spd.y;

	if (twp->pos.y > wk->fGroundY)
	{
		wk->flag &= ~0x80u;
	}
	else
	{
		twp->pos.y = wk->fGroundY;
		wk->spd.y = 0.0f;
		if (!(wk->flag & 0x80u))
		{
			wk->flag |= 0x80u;
			dsPlay_oneshot_v(830, 0, 0, 0, twp->pos.x, twp->pos.y, twp->pos.z);
		}
	}

	twp->pos.x += wk->spd.x;
	twp->pos.z += wk->spd.z;

	wk->spd.x *= 0.95f;
	wk->spd.z *= 0.95f;

	if (wk->spd.x < 0.05f && wk->spd.x > -0.05f)
	{
		wk->spd.x = 0.0f;
	}
	if (wk->spd.z < 0.05f && wk->spd.z > -0.05f)
	{
		wk->spd.z = 0.0f;
	}

	auto ptwp = playertwp[GetClosestPlayerNum(&twp->pos)];
	Angle ang = 0;

	if ((wk->flag & 2) && ptwp)
	{
		ang = -LOWORD(twp->ang.y) - NJM_RAD_ANG(atan2f(twp->pos.z - ptwp->pos.z, twp->pos.x - ptwp->pos.x));
	}

	ang = SubAngle(wk->angHead, ang);

	if (ang >= -455)
	{
		if (ang > 455)
		{
			ang = 455;
		}
	}
	else
	{
		ang = -455;
	}

	wk->angHead += ang;
	SpringAngle(&wk->angSpdX, &twp->ang.x);
}

static void __cdecl e103_move_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		e103_move_m(tp);
	}
	else
	{
		e103_move_h.Original(tp);
	}
}
#pragma endregion

static Bool __cdecl e103_chkPlayerRadius_r(task* tp, float r)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto ptwp = playertwp[i];

			if (ptwp)
			{
				auto x = twp->pos.x - ptwp->pos.x;
				auto z = twp->pos.z - ptwp->pos.z;

				if (x * x + z * z < r)
				{
					return TRUE;
				}
			}
		}

		return FALSE;
	}
	else
	{
		return e103_chkPlayerRadius_h.Original(tp, r);
	}
}

static void __cdecl e103_waitPlayer_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto wk = (TGT_WK*)tp->awp;

		CheckRangeOut(tp);

		if (IsPlayerInSphere(&tp->twp->pos, 150.0f) && twp->smode == 0)
		{
			wk->fHitPoint = 5.0f;
			wk->bwk.req_action = 0;
			*(int*)0x3C5D71C = 0;
			wk->flag &= ~8;
			SetE103Camera(tp);
			if (twp->mode != 7)
			{
				wk->mode_old = twp->mode;
				wk->mode_req = 7;
			}
			twp->smode = -1;
		}

		if (twp->smode == -1 || wk->fGroundDist < 150.0f)
		{
			wk->flag |= 4;
		}
		else
		{
			wk->flag &= ~4;
		}
	}
	else
	{
		e103_waitPlayer_h.Original(tp);
	}
}

static void __cdecl e103_turnBody_r(task* pTask, NJS_POINT3* posTarget, Angle angMax)
{
	if (multiplayer::IsActive())
	{
		auto ptwp = playertwp[GetClosestPlayerNum(&pTask->twp->pos)];

		if (ptwp)
		{
			e103_turnBody_h.Original(pTask, &ptwp->pos, angMax);
		}
	}
	else
	{
		e103_turnBody_h.Original(pTask, posTarget, angMax);
	}
}

static void __cdecl e103_chkDamage_r(task* tp)
{
	auto twp = tp->twp;
	auto ewp = (enemywk*)tp->mwp;

	// Teleport back if too far
	if (GetDistance(&twp->pos, &ewp->home) > 500.0f)
	{
		twp->pos = ewp->home;
	}

	if (twp->mode != 6)
	{
		auto hit_twp = CCL_IsHitPlayer(twp);
		if (hit_twp && TASKWK_CHARID(hit_twp) != Characters_Gamma) // allow other characters to hurt enemy
		{
			SetWinnerMulti(TASKWK_PLAYERID(hit_twp));
			twp->flag |= Status_Hurt;
		}

		hit_twp = CCL_IsHitBullet(twp);
		if (hit_twp)
		{
			SetWinnerMulti(hit_twp->btimer); // player number is stored in btimer thanks to patch in E102.cpp
		}
	}

	e103_chkDamage_h.Original(tp);
}

static void __cdecl e103_normal_r(task* tp)
{
	auto twp = tp->twp;
	auto wk = (TGT_WK*)tp->awp;

	if (twp->smode == 0)
	{
		SetE103Camera(tp);
		wk->bwk.req_action = 0;
		wk->flag |= 2;
		twp->smode = 1;
		return;
	}

	e103_normal_h.Original(tp);
}

void patch_windy_e103_init()
{
	e103_move_h.Hook(e103_move_r);
	e103_chkPlayerRadius_h.Hook(e103_chkPlayerRadius_r);
	e103_waitPlayer_h.Hook(e103_waitPlayer_r);
	e103_turnBody_h.Hook(e103_turnBody_r);
	e103_chkDamage_h.Hook(e103_chkDamage_r);
	e103_normal_h.Hook(e103_normal_r);
}

RegisterPatch patch_windy_e103(patch_windy_e103_init);
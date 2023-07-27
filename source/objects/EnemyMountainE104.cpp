#include "pch.h"
#include "multiplayer.h"
#include "result.h"
#include "camera.h"

// E104 is almost entirely a copy paste of E103

Trampoline* e104_move_t = nullptr;
Trampoline* e104_waitPlayer_t = nullptr;
Trampoline* e104_chkPlayerRadius_t = nullptr;
Trampoline* e104_turnBody_t = nullptr;
Trampoline* e104_chkDamage_t = nullptr;
Trampoline* e104_execMode_t = nullptr;

static void SetE104Camera(task* tp)
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
static void e104_move_o(task* tp)
{
	auto target = e104_move_t->Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

static void e104_move_m(task* tp)
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

static void __cdecl e104_move_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		e104_move_m(tp);
	}
	else
	{
		e104_move_o(tp);
	}
}

static void __declspec(naked) e104_move_w()
{
	__asm
	{
		push eax
		call e104_move_r
		pop eax
		retn
	}
}
#pragma endregion

static void __cdecl e104_waitPlayer_r(task* tp)
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
			*(int*)0x3C80FB8 = 0;
			wk->flag &= ~8;
			SetE104Camera(tp);
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
			wk->spd.y = 0.0f;
			wk->flag &= ~4;
		}
	}
	else
	{
		TARGET_DYNAMIC(e104_waitPlayer)(tp);
	}
}

#pragma region chkPlayerRadius
static BOOL e104_chkPlayerRadius_o(task * tp, Float r)
{
	auto target = e104_chkPlayerRadius_t->Target();
	BOOL rt;
	__asm
	{
		push[r]
		mov eax, [tp]
		call target
		mov rt, eax
		add esp, 4
	}
	return rt;
}

static BOOL __cdecl e104_chkPlayerRadius_r(task* tp, Float r)
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
		return e104_chkPlayerRadius_o(tp, r);
	}
}

static void __declspec(naked) e104_chkPlayerRadius_w()
{
	__asm
	{
		push edx
		push[esp + 04h]
		push eax
		call e104_chkPlayerRadius_r
		add esp, 8
		pop edx
		retn
	}
}
#pragma endregion

#pragma region turnBody
static void e104_turnBody_o(task* pTask, NJS_POINT3* posTarget, Angle angMax)
{
	auto target = e104_turnBody_t->Target();
	__asm
	{
		mov eax, [angMax]
		mov eax, [posTarget]
		mov ecx, [pTask]
		call target
	}
}

static void __cdecl e104_turnBody_r(task* pTask, NJS_POINT3* posTarget, Angle angMax)
{
	if (multiplayer::IsActive())
	{
		auto ptwp = playertwp[GetClosestPlayerNum(&pTask->twp->pos)];

		if (ptwp)
		{
			e104_turnBody_o(pTask, &ptwp->pos, angMax);
		}
	}
	else
	{
		e104_turnBody_o(pTask, posTarget, angMax);
	}
}

static void __declspec(naked) e104_turnBody_w()
{
	__asm
	{
		push edi
		push eax
		push ecx
		call e104_turnBody_r
		pop ecx
		pop eax
		pop edi
		retn
	}
}
#pragma endregion

#pragma region chkDamage
static void e104_chkDamage_o(task* tp)
{
	auto target = e104_chkDamage_t->Target();
	__asm
	{
		mov esi, [tp]
		call target
	}
}

static void __cdecl e104_chkDamage_r(task* tp)
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

	e104_chkDamage_o(tp);
}

static void __declspec(naked) e104_chkDamage_w()
{
	__asm
	{
		push esi
		call e104_chkDamage_r
		pop esi
		retn
	}
}
#pragma endregion

#pragma region execMode
static void e104_execMode_o(task* tp)
{
	auto target = e104_execMode_t->Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

static void __cdecl e104_execMode_r(task* tp)
{
	auto twp = tp->twp;
	auto wk = (TGT_WK*)tp->awp;
	
	if (twp->mode == 1 && twp->smode == 0) // "normal" was inlined
	{
		SetE104Camera(tp);
		wk->bwk.req_action = 0;
		wk->flag |= 2;
		twp->smode = 1;
		return;
	}

	e104_execMode_o(tp);
}

static void __declspec(naked) e104_execMode_w()
{
	__asm
	{
		push eax
		call e104_execMode_r
		pop eax
		retn
	}
}
#pragma endregion

void InitE104Patches()
{
	e104_move_t = new Trampoline(0x6048B0, 0x6048B5, e104_move_w);
	e104_waitPlayer_t = new Trampoline(0x6046A0, 0x6046A5, e104_waitPlayer_r);
	e104_turnBody_t = new Trampoline(0x604480, 0x604485, e104_turnBody_w);
	e104_chkDamage_t = new Trampoline(0x604310, 0x604316, e104_chkDamage_w);
	e104_execMode_t = new Trampoline(0x605400, 0x605406, e104_execMode_w);
	e104_chkPlayerRadius_t = new Trampoline(0x566D80, 0x566D89, e104_chkPlayerRadius_w);
}

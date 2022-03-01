#include "pch.h"
#include "multiplayer.h"
#include "result.h"

Trampoline* e103_move_t                 = nullptr;
Trampoline* e103_chkPlayerRadius_t      = nullptr;
Trampoline* e103_chkPlayerRangeIn_t     = nullptr;
Trampoline* e103_turnBody_t             = nullptr;
Trampoline* e103_chkDamage_t            = nullptr;

#pragma region move
static void e103_move_o(task* tp)
{
	auto target = e103_move_t->Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

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

    auto ptwp = playertwp[GetTheNearestPlayerNumber(&twp->pos)];
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
		e103_move_o(tp);
	}
}

static void __declspec(naked) e103_move_w()
{
	__asm
	{
		push eax
		call e103_move_r
        pop eax
		retn
	}
}
#pragma endregion

#pragma region chkPlayerRadius
static BOOL e103_chkPlayerRadius_o(task* tp, float r)
{
    auto target = e103_chkPlayerRadius_t->Target();
    BOOL ret;
    __asm
    {
        push[r]
        mov eax, [tp]
        call target
        mov ret, eax
        add esp, 4
    }
    return ret;
}

static BOOL __cdecl e103_chkPlayerRadius_r(task* tp, float r)
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
        return e103_chkPlayerRadius_o(tp, r);
    }
}

static void __declspec(naked) e103_chkPlayerRadius_w()
{
    __asm
    {
        push[esp + 04h]
        push eax
        call e103_chkPlayerRadius_r
        add esp, 8
        retn
    }
}
#pragma endregion

#pragma region chkPlayerRangeIn
static BOOL __cdecl e103_chkPlayerRangeIn_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        auto twp = tp->twp;

        if (IsPlayerInSphere(&tp->twp->pos, 150.0f))
        {
            twp->smode = 2;
            ccsi_flag = 1ui8;
        }

        return FALSE;
    }
    else
    {
        return TARGET_DYNAMIC(e103_chkPlayerRangeIn)(tp);
    }
}
#pragma endregion

#pragma region turnBody
static void e103_turnBody_o(task* pTask, NJS_POINT3* posTarget, Angle angMax)
{
    auto target = e103_turnBody_t->Target();
    __asm
    {
        mov eax, [angMax]
        mov ecx, [posTarget]
        mov eax, [pTask]
        call target
    }
}

static void __cdecl e103_turnBody_r(task* pTask, NJS_POINT3* posTarget, Angle angMax)
{
    if (multiplayer::IsActive())
    {
        auto ptwp = playertwp[GetTheNearestPlayerNumber(&pTask->twp->pos)];

        if (ptwp)
        {
            e103_turnBody_o(pTask, &ptwp->pos, angMax);
        }
    }
    else
    {
        e103_turnBody_o(pTask, posTarget, angMax);
    }
}

static void __declspec(naked) e103_turnBody_w()
{
    __asm
    {
        push edi
        push ecx
        push eax
        call e103_turnBody_r
        pop eax
        pop ecx
        pop edi
        retn
    }
}
#pragma endregion

#pragma region chkDamage
static void e103_chkDamage_o(task* tp)
{
    auto target = e103_chkDamage_t->Target();
    __asm
    {
        mov esi, [tp]
        call target
    }
}

static void __cdecl e103_chkDamage_r(task* tp)
{
    if (multiplayer::IsBattleMode())
    {
        auto twp = tp->twp;

        if (twp->mode != 6 && twp->mode != 8)
        {
            auto hit_twp = CCL_IsHitBullet(twp);

            if (hit_twp)
            {
                SetWinnerMulti(hit_twp->btimer); // player number is stored in btimer thanks to patch in E102.cpp
            }
        }
    }
    
    e103_chkDamage_o(tp);
}

static void __declspec(naked) e103_chkDamage_w()
{
    __asm
    {
        push esi
        call e103_chkDamage_r
        pop esi
        retn
    }
}
#pragma endregion

void InitE103Patches()
{
    e103_move_t                 = new Trampoline(0x4E6D00, 0x4E6D07, e103_move_w);
    e103_chkPlayerRadius_t      = new Trampoline(0x4E6900, 0x4E6908, e103_chkPlayerRadius_w);
    e103_turnBody_t             = new Trampoline(0x4E6940, 0x4E6949, e103_turnBody_w);
    e103_chkDamage_t            = new Trampoline(0x4E6790, 0x4E6796, e103_chkDamage_w);

    e103_chkPlayerRangeIn_t = new Trampoline(0x4E6B30, 0x4E6B37, e103_chkPlayerRangeIn_r);
    WriteCall((void*)((int)e103_chkPlayerRangeIn_t->Target() + 2), (void*)0x441AC0); // Patch trampoline
}
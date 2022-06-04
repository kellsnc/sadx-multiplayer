#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

static void __cdecl EnemyAir_r(task* tp);
static void __cdecl AirMissle_r(task* tp);
static void MissleLockOn_w();

Trampoline EnemyAir_t(0x4AA340, 0x4AA347, EnemyAir_r);
Trampoline AirMissle_t(0x4AA270, 0x4AA276, AirMissle_r);
Trampoline MissleLockOn_t(0x4A8AF0, 0x4A8AF8, MissleLockOn_w);

#pragma region MissleLockOn
static void MissleLockOn_o(enemywk* ewp, task* tp)
{
	auto target = MissleLockOn_t.Target();
	__asm
	{
		push[tp]
		mov ebx, [ewp]
		call target
	}
}

static void MissleLockOn_m(enemywk* ewp, task* tp)
{
    auto twp = tp->twp;
    auto ptp = tp->ptp;
    auto ptwp = ptp->twp;
    auto pewp = (enemywk*)ptp->mwp;

    if ((ptwp->flag & 0x400) != 0)
    {
        twp->ang.z = pewp->sub_angle[1];
        twp->ang.y = ptwp->ang.y;
        twp->ang.x = pewp->sub_angle[3];
        ewp->velo = pewp->velo;
        njPushMatrix(_nj_unit_matrix_);
        njTranslate(0, ptwp->pos.x, ptwp->pos.y, ptwp->pos.z);
        njRotateY_(twp->ang.y);
        njRotateZ_(twp->ang.z);
        njRotateX_(twp->ang.x);
        njRotateY(0, 0x8000);
        njSetTexture(&E_AIRCRAFT_TEXLIST);
        if (twp->smode)
        {
            njTranslate(0, 4.395678f, -14.181034f, 25.476711f);
        }
        else
        {
            njTranslate(0, 4.395678f, -14.181034f, -25.476711f);
        }
        njGetTranslation(0, &twp->pos);
        njPopMatrixEx();
        ewp->old_mode = twp->mode;
        twp->mode = 2i8;
        twp->wtimer = 0ui16;
        dsPlay_oneshot(862, 0, 0, 0);
        auto pnum = GetClosestPlayerNum(&ptwp->pos);
        ewp->aim = playertwp[pnum]->pos;
        auto shadowpos = playerpwp[pnum]->shadow.y_bottom;
        if (shadowpos != -1000000.0f)
        {
            ewp->aim.y = shadowpos;
        }
        auto ctp = CreateChildTask(2u, EffectSkyScope, tp);
        if (ctp)
        {
            ctp->twp->pos = ewp->aim;
            ctp->twp->btimer |= 3u;
            ctp->twp->wtimer = pnum;
        }
    }
}

static void __cdecl MissleLockOn_r(enemywk* ewp, task* tp)
{
	if (multiplayer::IsActive())
	{
		MissleLockOn_m(ewp, tp);
	}
	else
	{
		MissleLockOn_o(ewp, tp);
	}
}

static void __declspec(naked) MissleLockOn_w()
{
	__asm
	{
		push[esp + 04h]
		push ebx
		call MissleLockOn_r
		pop ebx
		add esp, 4
		retn
	}
}
#pragma endregion

#pragma region AirMissle
// Missing display task for AirMissle
static void __cdecl AirMissleDisp(task* tp)
{
    auto twp = tp->twp;
    if (twp->mode == 2 && !MissedFrames);
        MissleDraw(twp, (enemywk*)tp->mwp);
}

static void __cdecl AirMissle_r(task* tp)
{
    TARGET_STATIC(AirMissle)(tp);
    tp->disp = AirMissleDisp;
}
#pragma endregion

#pragma region EnemyAir
// Missing display task for EnemyAir
static void __cdecl EnemyAirDisp(task* tp)
{
    AirDraw(tp->twp, (enemywk*)tp->mwp);
}

static void __cdecl EnemyAir_r(task* tp)
{
    TARGET_STATIC(EnemyAir)(tp);
    tp->disp = EnemyAirDisp;
}
#pragma endregion
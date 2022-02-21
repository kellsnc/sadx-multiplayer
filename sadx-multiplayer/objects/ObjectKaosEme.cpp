#include "pch.h"
#include "multiplayer.h"
#include "result.h"

// Patch Emerald goals for multiplayer
// Adds a display function so that it displays when the game is paused & on other screens
// Adds a custom trigger that works for all players

static void __cdecl ObjectKaosEme_r(task* tp);
static void __cdecl ObjectKaosEmeIC_r(task* tp);
static void __cdecl ObjectKaosEmeCA_r(task* tp);

Trampoline ObjectKaosEme_t(0x4DF3B0, 0x4DF3B6, ObjectKaosEme_r);
Trampoline ObjectKaosEmeIC_t(0x4ECFA0, 0x4ECFA6, ObjectKaosEmeIC_r);
Trampoline ObjectKaosEmeCA_t(0x5DD0A0, 0x5DD0A6, ObjectKaosEmeCA_r);

static void __cdecl ObjectKaosEmeDisp(task* tp)
{
	taskwk* twp = tp->twp;

	if (twp->mode == 4)
	{
		return;
	}

	njSetTexture(&KAOS_EME_TEXLIST);
	njPushMatrixEx();
	njTranslate(0, twp->pos.x, twp->pos.y + 10.0f, twp->pos.z);
	njRotateY_(twp->ang.y);

	auto id = static_cast<int>(twp->scl.y);

	switch (id)
	{
	case 0:
		late_DrawObjectMesh(&object_goaleme_blue_blue, 1);
		break;
	case 1:
	default:
		late_DrawObjectMesh(&object_goaleme_white_white, 1);
		break;
	case 2:
		late_DrawObjectMesh(&object_goaleme_green_green, 1);
		break;
	}
	njPopMatrixEx();
}

static void CheckGameClear_m(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pnum = IsPlayerInSphere(&twp->pos, (twp->scl.x + 1.0f) * 14.0f) - 1;
		if (pnum >= 0)
		{
			SetWinnerMulti(pnum);
			SetFinishAction();
			twp->mode = 4;
		}
	}
}

static void __cdecl ObjectKaosEme_r(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 0)
	{
		tp->disp = ObjectKaosEmeDisp;
	}
	else if (twp->mode == 2)
	{
		CheckGameClear_m(tp);
	}

	TARGET_STATIC(ObjectKaosEme)(tp);
}

static void __cdecl ObjectKaosEmeIC_r(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 0)
	{
		tp->disp = ObjectKaosEmeDisp;
	}
	else if (twp->mode == 2)
	{
		CheckGameClear_m(tp);
	}

	TARGET_STATIC(ObjectKaosEmeIC)(tp);
}

static void __cdecl ObjectKaosEmeCA_r(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 0)
	{
		tp->disp = ObjectKaosEmeDisp;
	}
	else if (twp->mode == 2)
	{
		CheckGameClear_m(tp);
	}

	TARGET_STATIC(ObjectKaosEmeCA)(tp);
}
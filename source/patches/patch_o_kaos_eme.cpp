#include "pch.h"
#include "multiplayer.h"
#include "result.h"

// Patch Emerald goals for multiplayer
// Adds a display function so that it displays when the game is paused & on other screens
// Adds a custom trigger that works for all players

static void __cdecl ObjectKaosEme_r(task* tp);
static void __cdecl ObjectKaosEmeIC_r(task* tp);
static void __cdecl ObjectKaosEmeCA_r(task* tp);

FastFunctionHookPtr<decltype(&ObjectKaosEme_r)> ObjectKaosEme_h(0x4DF3B0);
FastFunctionHookPtr<decltype(&ObjectKaosEmeIC_r)> ObjectKaosEmeIC_h(0x4ECFA0);
FastFunctionHookPtr<decltype(&ObjectKaosEmeCA_r)> ObjectKaosEmeCA_h(0x5DD0A0);

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
	ROTATEY(0, twp->ang.y);

	auto id = static_cast<int>(twp->scl.y);

	switch (id)
	{
	case 0:
		late_DrawObjectMesh(&object_goaleme_blue_blue, LATE_WZ);
		break;
	case 1:
	default:
		late_DrawObjectMesh(&object_goaleme_white_white, LATE_WZ);
		break;
	case 2:
		late_DrawObjectMesh(&object_goaleme_green_green, LATE_WZ);
		break;
	}
	njPopMatrixEx();
}

static void CheckGameClear_m(task* tp)
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

static void __cdecl ObjectKaosEme_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		switch (tp->twp->mode)
		{
		case 0:
			tp->disp = ObjectKaosEmeDisp;
			break;
		case 2:
			CheckGameClear_m(tp);
			break;
		}
	}

	ObjectKaosEme_h.Original(tp);
}

static void __cdecl ObjectKaosEmeIC_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		switch (tp->twp->mode)
		{
		case 0:
			tp->disp = ObjectKaosEmeDisp;
			break;
		case 2:
			CheckGameClear_m(tp);
			break;
		}
	}

	ObjectKaosEmeIC_h.Original(tp);
}

static void __cdecl ObjectKaosEmeCA_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		switch (tp->twp->mode)
		{
		case 0:
			tp->disp = ObjectKaosEmeDisp;
			break;
		case 2:
			CheckGameClear_m(tp);
			break;
		}
	}

	ObjectKaosEmeCA_h.Original(tp);
}

void patch_kaos_eme_init()
{
	ObjectKaosEme_h.Hook(ObjectKaosEme_r);
	ObjectKaosEmeIC_h.Hook(ObjectKaosEmeIC_r);
	ObjectKaosEmeCA_h.Hook(ObjectKaosEmeCA_r);
}

RegisterPatch patch_kaos_eme(patch_kaos_eme_init);
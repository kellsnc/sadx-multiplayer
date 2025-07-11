#include "pch.h"
#include "multiplayer.h"
#include "result.h"
#include <splitscreen.h>

// Patch Emerald goals for multiplayer
// Adds a display function so that it displays when the game is paused & on other screens
// Adds a custom trigger that works for all players

static void __cdecl ObjectKaosEme_r(task* tp);
static void __cdecl ObjectKaosEmeIC_r(task* tp);
static void __cdecl ObjectKaosEmeCA_r(task* tp);

FastFunctionHookPtr<decltype(&ObjectKaosEme_r)> ObjectKaosEme_h(0x4DF3B0);
FastFunctionHookPtr<decltype(&ObjectKaosEmeIC_r)> ObjectKaosEmeIC_h(0x4ECFA0);
FastFunctionHookPtr<decltype(&ObjectKaosEmeCA_r)> ObjectKaosEmeCA_h(0x5DD0A0);

DataPointer(NJS_SPRITE, sprite_kaos_eme_eff, 0x03C5E32C); // Goal emerald effect Ice Cap
DataPointer(NJS_SPRITE, sprite_kaos_eme_eff_0, 0x03C750F4); // Goal emerald effect Casino
DataPointer(NJS_SPRITE, sprite_kaos_eme_eff_1, 0x03C5D680); // Goal emerald effect Windy Valley

// Draws the emerald glow effect sprite 
// Original code from DC Conversion by PkR, re used here to make it works on split screen
void DrawEmeraldGlow(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		if (splitscreen::IsActive() && splitscreen::GetCurrentViewPortNum() != 0)
		{

			NJS_SPRITE* sprite;
			switch ((int)twp->scl.y)
			{
			case 0: // Windy Valley 
				sprite = &sprite_kaos_eme_eff_1;
				break;
			case 1: // Casinopolis
				sprite = &sprite_kaos_eme_eff_0;
				break;
			case 2: // Ice Cap
			default:
				sprite = &sprite_kaos_eme_eff;
				break;
			}
			njPushMatrix(0);
			njTranslate(0, twp->pos.x, twp->pos.y + 15, twp->pos.z);
			njRotateXYZ(0, camera_twp->ang.x, camera_twp->ang.y, 0);
			float alpha = min(255, abs(twp->value.l)) / 255.0f;
			SetMaterial(1.0f, alpha, alpha, alpha);
			njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_ONE);
			njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
			// Draw it twice lol
			late_DrawSprite3D(sprite, (int)twp->scl.y, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR | NJD_SPRITE_ANGLE, LATE_LIG);
			late_DrawSprite3D(sprite, (int)twp->scl.y, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR | NJD_SPRITE_ANGLE, LATE_LIG);
			njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
			njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
			njPopMatrix(1u);
			ResetMaterial();
			// Increase and limit glow timer
			twp->value.l += 2;
			if ((twp->value.l > 0 && (unsigned int)twp->value.l > twp->counter.l + 128) || (twp->value.l < 0 && twp->value.l > -128))
				twp->value.l = twp->value.l * -1;
		}
	}

}

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

	DrawEmeraldGlow(twp);
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
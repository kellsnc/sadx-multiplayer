#include "pch.h"
#include <math.h>
#include "splitscreen.h"
#include "hud_emerald.h"

FastFunctionHook<void, task*> Knuckles_KakeraGameExec_h(0x475E50);
FastFunctionHook<void, task*> Knuckles_KakeraGamePutRadar1C_h(0x475A70);

void DrawBattleEmeRadar(int pnum, int scalors)
{
	auto ratio = SplitScreen::GetScreenRatio(pnum);

	float screenX = HorizontalResolution * ratio->x;
	float screenY = VerticalResolution * ratio->y;
	float scaleY = VerticalStretch * ratio->h;
	float scaleX = HorizontalStretch * ratio->w;
	float scale = min(scaleX, scaleY);

	for (int i = 0; i < 3; ++i)
	{
		auto& fragm = fragmset_tbl[i];

		if (fragm.boutflag == pnum + 1) // Draw as collected
		{
			NJS_VECTOR pos;
			pos.x = screenX + (scaleX * 640.0f) - (52.0f * scale) - ((70.0f * scale) * i);
			pos.y = screenY + scaleY * 16.0f + scale * 32.0f;
			EmeraldRadarHud_Draw(&pos, 3, scale, 1.0f);
		}
		else if (fragm.boutflag == 0) // Draw as non collected
		{
			auto ptwp = playertwp[pnum];
			float dist = GetDistance(&ptwp->pos, &fragm.pos);

			NJS_VECTOR pos;
			float padding = 35.0f * scale;
			pos.x = screenX + scaleX * 320.0f - padding + i * padding;
			pos.y = screenY + scaleY * 480.0f - 62.0f * ratio->h;

			float sprite_scale = ((BYTEn(scalors, i) * 0.005f) + 0.5f) * scale;

			if (!fragm.psflag || dist > 400.0f)
			{
				EmeraldRadarHud_Draw(&pos, 2, 0.5f * scale, 0.2f);
			}
			else if (dist > 300.0f)
			{
				EmeraldRadarHud_Draw(&pos, 5, sprite_scale, 1.0f);
			}
			else if (dist > 100.0f)
			{
				EmeraldRadarHud_Draw(&pos, 4, sprite_scale, 1.0f);
			}
			else if (dist > 50.0f)
			{
				EmeraldRadarHud_Draw(&pos, 1, sprite_scale, 1.0f);
			}
			else if (dist > 30.0f)
			{
				EmeraldRadarHud_Draw(&pos, 7, sprite_scale, 1.0f);
			}
			else
			{
				EmeraldRadarHud_Draw(&pos, 0, sprite_scale, 1.0f);
			}
		}
	}
}

void __cdecl Knuckles_KakeraGameExec_r(task* tp)
{
	if (SplitScreen::IsActive())
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		Knuckles_KakeraGameExec_h.Original(tp);
		SplitScreen::RestoreViewPort();
	}
	else
	{
		Knuckles_KakeraGameExec_h.Original(tp);
	}
}

void __cdecl Knuckles_KakeraGamePutRadar1C_r(task* tp)
{
	if (SplitScreen::IsActive())
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		Knuckles_KakeraGamePutRadar1C_h.Original(tp);
		SplitScreen::RestoreViewPort();
	}
	else
	{
		Knuckles_KakeraGamePutRadar1C_h.Original(tp);
	}
}

void patch_hud_emerald_init()
{
	Knuckles_KakeraGameExec_h.Hook(Knuckles_KakeraGameExec_r);
	Knuckles_KakeraGamePutRadar1C_h.Hook(Knuckles_KakeraGamePutRadar1C_r);
}

RegisterPatch patch_hud_emerald(patch_hud_emerald_init);
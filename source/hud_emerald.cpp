#include "pch.h"
#include <math.h>
#include "splitscreen.h"
#include "hud_emerald.h"

Trampoline* Knuckles_KakeraGameExec_t = nullptr;
Trampoline* Knuckles_KakeraGamePutRadar1C_t = nullptr;

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
		TARGET_DYNAMIC(Knuckles_KakeraGameExec)(tp);
		SplitScreen::RestoreViewPort();
	}
	else
	{
		TARGET_DYNAMIC(Knuckles_KakeraGameExec)(tp);
	}
}

void __cdecl Knuckles_KakeraGamePutRadar1C_r(task* tp)
{
	if (SplitScreen::IsActive())
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		TARGET_DYNAMIC(Knuckles_KakeraGamePutRadar1C)(tp);
		SplitScreen::RestoreViewPort();
	}
	else
	{
		TARGET_DYNAMIC(Knuckles_KakeraGamePutRadar1C)(tp);
	}
}

void InitEmeraldRadar()
{
	Knuckles_KakeraGameExec_t = new Trampoline(0x475E50, 0x475E55, Knuckles_KakeraGameExec_r);
	Knuckles_KakeraGamePutRadar1C_t = new Trampoline(0x475A70, 0x475A75, Knuckles_KakeraGamePutRadar1C_r);
}
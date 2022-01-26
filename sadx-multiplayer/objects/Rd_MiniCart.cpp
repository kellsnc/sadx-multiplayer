#include "pch.h"
#include "splitscreen.h"

/*

Rewrite Twinkle Circuit manager to work for all players

*/

// RACE MANAGER:

static void dispRaceSingle(int num)
{
	auto ratio = SplitScreen::GetScreenRatio(num);

	float scaleY = VerticalStretch * ratio->h;
	float scaleX = HorizontalStretch * ratio->w;

	// LAP COUNT:
	OBJ_MINI_CART_SPRITE_LAPS_A.p.x = 640.0f * scaleX - 16.0f * scaleX - 96.0f * scaleX + HorizontalResolution * ratio->x;
	OBJ_MINI_CART_SPRITE_LAPS_A.p.y = 96.0f * scaleY + VerticalResolution * ratio->y;
	OBJ_MINI_CART_SPRITE_LAPS_A.sx = OBJ_MINI_CART_SPRITE_LAPS_A.sy = scaleX;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_LAPS_A, 0, 22045.998, NJD_SPRITE_ALPHA, QueuedModelFlagsB_SomeTextureThing);

	OBJ_MINI_CART_SPRITE_LAPS_B.p.x = 640.0f * scaleX - 16.0f * scaleX - 96.0f * scaleX + 39.0f * scaleX + HorizontalResolution * ratio->x;
	OBJ_MINI_CART_SPRITE_LAPS_B.p.y = 99.0f * scaleY + VerticalResolution * ratio->y;
	OBJ_MINI_CART_SPRITE_LAPS_B.sx = OBJ_MINI_CART_SPRITE_LAPS_B.sy = scaleX;
	njDrawSprite2D_Queue(&OBJ_MINI_CART_SPRITE_LAPS_B, 0, 22046.0, NJD_SPRITE_ALPHA, QueuedModelFlagsB_SomeTextureThing);
}

static void __cdecl dispRaceM(task* tp)
{
	if (!MissedFrames && HideHud >= 0)
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);

		//MirenEffSaveState:
		ghDefaultBlendingMode();
		SetMaterialAndSpriteColor(&SpriteColor_White);
		njControl3D_Backup();
		BackupConstantAttr();

		njSetTexture(&OBJ_MINI_CART_TEXLIST);

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (SplitScreen::IsScreenEnabled(i))
			{
				dispRaceSingle(i);
			}
		}
		
		// MirenEffLoadState:
		ghDefaultBlendingMode();
		njControl3D_Restore();
		RestoreConstantAttr();
		ResetMaterial();

		SplitScreen::RestoreViewPort();
	}
}

static void __cdecl execRaceM(task* tp)
{
	tp->disp(tp);
}

static void __cdecl initRaceM(task* tp, void* param_p)
{
	RaceManageTask_p = tp;
}

static const TaskInfo infoM = { 0x40, 0, initRaceM, execRaceM, dispRaceM, nullptr };

// ROUND TASK:

static void __cdecl deadRoundM(task* tp)
{
	ResetMleriRangeRad();
}

static void __cdecl execRoundM(task* tp)
{
	EnableController(0);
	EnableControl();
}

static void LoadAdditionalCarts()
{
	for (int i = 1; i < multiplayer::GetPlayerCount(); ++i)
	{
		auto tp = CreateElementalTask(LoadObj_Data1 | LoadObj_UnknownA | LoadObj_UnknownB, LEV_3, EnemyCart);

		if (tp && tp->twp)
		{
			tp->twp->btimer = i;

			if (playertwp[i])
			{
				tp->twp->scl.y = 1.0f;
				tp->twp->pos = { 1513.0f, 9.0f, 74.0f };
				tp->twp->ang.y = 0xC000;
			}
		}
	}
}

static void __cdecl initRoundM(task* tp, void* param_p)
{
	InitFreeCamera();
	LoadAdditionalCarts();
	MirenSetTask(LEV_2, &infoM, 0);
}

static const TaskInfo RdTaskInfoM = { 1, 2, initRoundM, execRoundM, 0, deadRoundM };

void __cdecl Rd_MiniCart_r(task* tp);
Trampoline Rd_MiniCart_t(0x4DAA80, 0x4DAA86, Rd_MiniCart_r);
void __cdecl Rd_MiniCart_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		MirenInitTask(tp, &RdTaskInfoM, nullptr);
	}
	else
	{
		TARGET_STATIC(Rd_MiniCart)(tp);
	}
}
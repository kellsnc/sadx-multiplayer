#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "VariableHook.hpp"
#include "multiplayer.h"
#include "splitscreen.h"

enum : __int8
{
	MODE_NONE,
	MODE_MOVE,
	MODE_WAIT,
	MODE_SHRINK // custom name
};

Trampoline* manager_Disp_t = nullptr;
Trampoline* itemBoxManager_t = nullptr;

VariableHook<ITEM_MANAGER, 0x3C5A9D8> manager_data_m;

static void manager_DrawItems(ITEM_MANAGER* data, int num) // custom
{
	auto ratio = SplitScreen::GetScreenRatio(num);

	float screenX = HorizontalResolution * ratio->x;
	float screenY = VerticalResolution * ratio->y;
	float scaleY = VerticalStretch * ratio->h;
	float scaleX = HorizontalStretch * ratio->w;
	float scale = min(scaleX, scaleY);

	for (int i = 0; i < data->current_list; ++i)
	{
		auto& item = data->itemdata[i];
		auto id = item.item_list;

		if (id == 4)
		{
			if (item.random_ring == 5)
			{
				id = 2;
			}
			else if (item.random_ring == 10)
			{
				id = 3;
			}
		}

		NJS_TEXANIM* tx = reinterpret_cast<NJS_TEXANIM*>(late_alloca(sizeof(NJS_TEXANIM)));
		*tx = *(NJS_TEXANIM*)0x7E6BA4;
		tx->texid = item_info[id].texture_id;

		NJS_SPRITE sp;
		sp.tlist = &OBJ_REGULAR_TEXLIST;
		sp.tanim = tx;
		sp.p.x = screenX + scaleX * item.item_pos;
		sp.p.y = screenY + scaleY * 370.0f;
		sp.sx = sp.sy = item.scale * scale;
		sp.ang = 0;

		njDrawSprite2D_Queue(&sp, 0, 22046.998f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, QueuedModelFlagsB_SomeTextureThing);

		if (id == 4)
		{
			sp.tanim = reinterpret_cast<NJS_TEXANIM*>(late_alloca(sizeof(NJS_TEXANIM)));
			*sp.tanim = *(NJS_TEXANIM*)0x7E6B8C;

			sp.p.x = screenX + scaleX * (item.item_pos - 1.0f);
			sp.p.y = screenY + scaleY * 372.0f;

			if (item.random_ring == 15)
			{
				sp.tanim->u1 = 128;
				sp.tanim->v1 = 0;
				sp.tanim->u2 = 255;
				sp.tanim->v2 = 127;
			}
			else if (item.random_ring == 20)
			{
				sp.tanim->u1 = 0;
				sp.tanim->v1 = 128;
				sp.tanim->u2 = 127;
				sp.tanim->v2 = 255;
			}
			else if (item.random_ring == 40)
			{
				sp.tanim->u1 = 0;
				sp.tanim->v1 = 0;
				sp.tanim->u2 = 127;
				sp.tanim->v2 = 127;
			}
			else
			{
				sp.tanim->u1 = 128;
				sp.tanim->v1 = 128;
				sp.tanim->u2 = 255;
				sp.tanim->v2 = 255;
			}

			njDrawSprite2D_ForcePriority(&sp, 0, -1.002f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
		}
	}
}

static void manager_Disp_m(task* tp)
{
	if (!MissedFrames && (GameMode != MD_GAME_FADEOUT_CHANGE2 || !GetMiClearStatus()))
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		ghDefaultBlendingMode();
		SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (SplitScreen::GetCurrentScreenNum() == i)
			{
				manager_DrawItems(&manager_data_m[i], i);
			}
		}

		SplitScreen::RestoreViewPort();
		ResetMaterial();
	}
}

static void __cdecl manager_Disp_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		manager_Disp_m(tp);
	}
	else
	{
		TARGET_DYNAMIC(manager_Disp)(tp);
	}
}

static void manager_Move_m(ITEM_MANAGER* data)
{
	auto next_list = 0;

	for (int i = 0; i < data->current_list; ++i)
	{
		auto& item = data->itemdata[i];

		item.item_pos += 30.0f;

		float max = (float)(data->current_list - 1) * 40.0f + 320.0f - i * 80.0f;

		if (item.item_pos >= max)
		{
			item.item_pos = max;
			++next_list;
		}

		item.scale += 0.2f;

		if (item.scale > 2.0f)
		{
			item.scale = 2.0f;
		}
	}

	if (data->current_list == next_list)
	{
		data->mode = MODE_WAIT;
		data->counter = 0;
	}
}

static void manager_Wait_m(ITEM_MANAGER* data)
{
	for (int i = 0; i < data->current_list; ++i)
	{
		auto& item = data->itemdata[i];

		item.scale += 0.2f;

		if (item.scale > 2.0f)
		{
			item.scale = 2.0f;
		}
	}

	if (++data->counter > 60)
	{
		data->counter = 60;
		data->mode = MODE_SHRINK;
	}
}

static void manager_Shrink_m(ITEM_MANAGER* data) // custom name
{
	for (int i = 0; i < data->current_list; ++i)
	{
		data->itemdata[i].scale = (float)data->counter * 0.033333335f;
	}

	if (--data->counter <= 0)
	{
		data->current_list = 0;
		data->mode = MODE_NONE;
	}
}

static void itemBoxManager_m(task* tp)
{
	for (auto& data : manager_data_m)
	{
		switch (data.mode)
		{
		case MODE_MOVE:
			manager_Move_m(&data);
			break;
		case MODE_WAIT:
			manager_Wait_m(&data);
			break;
		case MODE_SHRINK:
			manager_Shrink_m(&data);
			break;
		}
	}

	if (explosion_r >= 0.0f)
	{
		explosion_r += 10.0f;
		if (explosion_r > 500.0f)
		{
			explosion_r = -1.0f;
		}
	}

	tp->disp(tp);
}

static void __cdecl itemBoxManager_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		itemBoxManager_m(tp);
	}
	else
	{
		TARGET_DYNAMIC(itemBoxManager)(tp);
	}
}

void EntryItemBoxPanel_m(int panel, int pnum)
{
	auto& data = manager_data_m[pnum];
	data.itemdata[data.current_list].item_list = panel;
	data.itemdata[data.current_list].item_pos = -60.0f;
	data.itemdata[data.current_list].scale = 2.0f;
	data.itemdata[data.current_list].random_ring = RandomRingNum;
	++data.current_list;
	data.mode = 1;
}

void InitItemBoxHUD()
{
	manager_Disp_t   = new Trampoline(0x4C0790, 0x4C0795, manager_Disp_r);
	itemBoxManager_t = new Trampoline(0x4C09B0, 0x4C09B5, itemBoxManager_r);
}
#include "pch.h"
#include "sadx_utils.h"
#include "splitscreen.h"
#include "ObjectItemBox.h"

enum : __int8 // hud modes
{
	MODE_NONE,
	MODE_MOVE,
	MODE_WAIT,
	MODE_SHRINK // custom name
};

Trampoline* TBarrier_t = nullptr;

Trampoline* manager_Disp_t = nullptr;
Trampoline* itemBoxManager_t = nullptr;
Trampoline* EntryItemBoxPanel_t = nullptr;

MAKEVARMULTI(ITEM_MANAGER, manager_data, 0x3C5A9D8);

static int itembox_pnum = -1;

#pragma region ObjectItemBox
static void play_itembox_sound(int pnum)
{
	auto num = TASKWK_CHARID(playertwp[pnum]);
	if (num == Characters_Gamma)
		dsPlay_oneshot(1306, 0, 0, 0);
	dsPlay_oneshot(11, 0, 0, 0);
}

static int itembox_getpnum(taskwk* twp)
{
	auto pltwp = twp->cwp->hit_cwp->mytask->twp;

	if (!pltwp)
	{
		itembox_pnum = 0;
	}
	else
	{
		itembox_pnum = pltwp->cwp->id == 1 ? pltwp->btimer : TASKWK_PLAYERID(pltwp);
	}

	return itembox_pnum;
}

static void __cdecl ef_muteki_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = itembox_getpnum(twp);
		GetInvincibleBodyP(pnum);
		play_itembox_sound(pnum);
	}
	else
	{
		ef_muteki(twp);
	}
}

static void __cdecl ef_5ring_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = itembox_getpnum(twp);
		AddNumRingM(pnum, 5);
		play_itembox_sound(pnum);
	}
	else
	{
		ef_5ring(twp);
	}
}

static void __cdecl ef_10ring_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = itembox_getpnum(twp);
		AddNumRingM(pnum, 10);
		play_itembox_sound(pnum);
	}
	else
	{
		ef_10ring(twp);
	}
}

static void __cdecl ef_random_ring_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = itembox_getpnum(twp);
		RandomRingNum = RandomRingAmounts[rand() % 6];
		AddNumRingM(pnum, RandomRingNum);
		play_itembox_sound(pnum);
	}
	else
	{
		ef_random_ring(twp);
	}
}

static void __cdecl ef_baria_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = itembox_getpnum(twp);
		GetBarrierP(pnum);
		play_itembox_sound(pnum);
	}
	else
	{
		ef_baria(twp);
	}
}

static void __cdecl ef_1up_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = itembox_getpnum(twp);
		AddNumPlayerM(pnum, 1);
		play_itembox_sound(pnum);
	}
	else
	{
		ef_1up(twp);
	}
}

static void __cdecl ef_explosion_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		play_itembox_sound(itembox_getpnum(twp));

		explosion_pos = twp->pos;
		explosion_r = 10.0f;
	}
	else
	{
		ef_explosion(twp);
	}
}

static void __cdecl ef_th_baria_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = itembox_getpnum(twp);
		GetThunderBarrierP(pnum);
		play_itembox_sound(pnum);
	}
	else
	{
		ef_th_baria(twp);
	}
}

// Magnetic barrier has support for all player... except for one condition that checks player 1 ;_;
static void __cdecl TBarrier_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pnum = TASKWK_PLAYERID(twp);
		auto pltwp = playertwp[pnum];
		auto plpwp = playerpwp[pnum];

		if (pltwp && plpwp && (plpwp->item & Powerups_MagneticBarrier) != 0)
		{
			EffBarrierPosSet(twp, pltwp);
			
			if (UNIT_RAND > 0.70)
			{
				auto ctp = CreateChildTask(LoadObj_Data1, ThunderB, tp);
				auto ctwp = ctp->twp;

				if (ctwp)
				{
					ctwp->ang.x = rand() % 0x10001; // better version of (Angle)(UNIT_RAND * 65536.0)
					ctwp->ang.y = rand() % 0x10001;
					ctwp->value.f = 1.0f;
					ctp->disp = TBarrierDisp;
				}
			}
			LoopTaskC(tp);
		}
		else
		{
			FreeTask(tp);
		}
	}
	else
	{
		TARGET_DYNAMIC(TBarrier)(tp);
	}
}
#pragma endregion

#pragma region itemBoxManager
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
				manager_DrawItems(manager_data_m[i], i);
			}
		}

		SplitScreen::RestoreViewPort();
		ResetMaterial();
	}
}

static void __cdecl manager_Disp_r(task* tp)
{
	if (multiplayer::IsBattleMode())
	{
		manager_Disp_m(tp);
	}
	else if (SplitScreen::IsActive())
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		TARGET_DYNAMIC(manager_Disp)(tp);
		SplitScreen::RestoreViewPort();
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
		switch (data->mode)
		{
		case MODE_MOVE:
			manager_Move_m(data);
			break;
		case MODE_WAIT:
			manager_Wait_m(data);
			break;
		case MODE_SHRINK:
			manager_Shrink_m(data);
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
	if (multiplayer::IsBattleMode())
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
	data->itemdata[data->current_list].item_list = panel;
	data->itemdata[data->current_list].item_pos = -60.0f;
	data->itemdata[data->current_list].scale = 2.0f;
	data->itemdata[data->current_list].random_ring = RandomRingNum;
	++data->current_list;
	data->mode = 1;
}

static void __cdecl EntryItemBoxPanel_r(int panel)
{
	if (multiplayer::IsBattleMode() && itembox_pnum != -1)
	{
		EntryItemBoxPanel_m(panel, itembox_pnum);
		itembox_pnum = -1;
	}
	else
	{
		TARGET_DYNAMIC(EntryItemBoxPanel)(panel);
	}
}
#pragma endregion

void InitItemBoxPatches()
{
	TBarrier_t = new Trampoline(0x4BA2A0, 0x4BA2A5, TBarrier_r);

	manager_Disp_t = new Trampoline(0x4C0790, 0x4C0795, manager_Disp_r);
	itemBoxManager_t = new Trampoline(0x4C09B0, 0x4C09B5, itemBoxManager_r);
	EntryItemBoxPanel_t = new Trampoline(0x4C00B0, 0x4C00B5, EntryItemBoxPanel_r);

	item_info[1].effect_func = ef_muteki_r;
	item_info[2].effect_func = ef_5ring_r;
	item_info[3].effect_func = ef_10ring_r;
	item_info[4].effect_func = ef_random_ring_r;
	item_info[5].effect_func = ef_baria_r;
	item_info[6].effect_func = ef_1up_r;
	item_info[7].effect_func = ef_explosion_r;
	item_info[8].effect_func = ef_th_baria_r;
}
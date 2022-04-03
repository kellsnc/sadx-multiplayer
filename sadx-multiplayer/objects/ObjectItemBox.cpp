#include "pch.h"
#include "sadx_utils.h"
#include "splitscreen.h"
#include "hud_itembox.h"
#include "ObjectItemBox.h"

Trampoline* TBarrier_t = nullptr;

Trampoline* ObjectItemboxNormal_t = nullptr;
Trampoline* itembox_airCollisitonBefore_t = nullptr;

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

static int itembox_getpnum(taskwk* twp)
{
	auto pltwp = twp->cwp->hit_cwp->mytask->twp;

	if (!pltwp)
	{
		return 0;
	}
	else
	{
		return pltwp->cwp->id == 1 ? pltwp->btimer : TASKWK_PLAYERID(pltwp);
	}
}

#pragma region Effect calls
static void play_itembox_sound(int pnum)
{
	auto num = TASKWK_CHARID(playertwp[pnum]);
	if (num == Characters_Gamma)
		dsPlay_oneshot(1306, 0, 0, 0);
	dsPlay_oneshot(11, 0, 0, 0);
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
#pragma endregion

#pragma region ObjectItemBox
static void ObjectItemboxNormal_o(task* tp)
{
	auto target = ObjectItemboxNormal_t->Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

static void ObjectItemboxNormal_m(task* tp)
{
	auto twp = tp->twp;
	auto cwp = twp->cwp;

	if ((cwp->flag & 1) && (cwp->hit_cwp->id == 0 || cwp->hit_cwp->id == 1))
	{
		auto pnum = itembox_getpnum(twp);

		if (cwp->my_num == 1)
		{
			twp->mode = 3i8;
			SetVelocityP(pnum, 0.0f, 2.0f, 0.0f);
		}
		else if (twp->flag & Status_Hurt)
		{
			twp->mode = 3i8;
		}
		else
		{
			EntryColliList(twp);
			twp->scl.y += 3.0f;
			return;
		}

		if (item_info[item_kind].effect_func)
		{
			item_info[item_kind].effect_func(twp);
			EntryItemBoxPanel_m(item_kind, pnum);
		}
	}
	else
	{
		EntryColliList(twp);
		twp->scl.y += 3.0f;
	}
}

static void __cdecl ObjectItemboxNormal_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectItemboxNormal_m(tp);
	}
	else
	{
		ObjectItemboxNormal_o(tp);
	}
}

static void __declspec(naked) ObjectItemboxNormal_w()
{
	__asm
	{
		push eax
		call ObjectItemboxNormal_r
		pop eax
		retn
	}
}
#pragma endregion

#pragma region ObjectItemboxAir
static void itembox_airCollisitonBefore_o(task* tp)
{
	auto target = itembox_airCollisitonBefore_t->Target();
	__asm
	{
		mov edi, [tp]
		call target
	}
}

static void itembox_airCollisitonBefore_m(task* tp)
{
	auto twp = tp->twp;
	auto awp = tp->awp;

	itembox_air_data->timer += 0.05f;

	if (itembox_air_data->timer > 1.0f)
	{
		itembox_air_data->timer = 1.0f;

		auto cwp = twp->cwp;

		if ((cwp->flag & 1) && (cwp->hit_cwp->id == 0 || cwp->hit_cwp->id == 1))
		{
			auto pnum = itembox_getpnum(twp);
			twp->mode = 2i8;

			NJS_POINT3 p = { twp->pos.x, twp->pos.y + 15.0f, twp->pos.z };
			CreateSnow(&p, (NJS_POINT3*)0x7E6FD8, 1.2f);

			if (item_info[itembox_air_data->item].effect_func)
			{
				item_info[itembox_air_data->item].effect_func(twp);
				EntryItemBoxPanel_m(itembox_air_data->item, pnum);
			}

			E102KillCursor(tp);
			FreeColliWork(twp);
			CCL_Init(tp, (CCL_INFO*)0x981A78, 1, 4u);
		}
		else
		{
			EntryColliList(twp);
		}
	}
}

static void __cdecl itembox_airCollisitonBefore_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		itembox_airCollisitonBefore_m(tp);
	}
	else
	{
		itembox_airCollisitonBefore_o(tp);
	}
}

static void __declspec(naked) itembox_airCollisitonBefore_w()
{
	__asm
	{
		push edi
		call itembox_airCollisitonBefore_r
		pop edi
		retn
	}
}
#pragma endregion

void InitItemBoxPatches()
{
	TBarrier_t = new Trampoline(0x4BA2A0, 0x4BA2A5, TBarrier_r);

	ObjectItemboxNormal_t = new Trampoline(0x4D6670, 0x4D6677, ObjectItemboxNormal_w);
	itembox_airCollisitonBefore_t = new Trampoline(0x4C0610, 0x4C0616, itembox_airCollisitonBefore_w);

	item_info[1].effect_func = ef_muteki_r;
	item_info[2].effect_func = ef_5ring_r;
	item_info[3].effect_func = ef_10ring_r;
	item_info[4].effect_func = ef_random_ring_r;
	item_info[5].effect_func = ef_baria_r;
	item_info[6].effect_func = ef_1up_r;
	item_info[7].effect_func = ef_explosion_r;
	item_info[8].effect_func = ef_th_baria_r;
}
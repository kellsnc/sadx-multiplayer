#include "pch.h"
#include "sadx_utils.h"

Trampoline* EntryItemBoxPanel_t = nullptr;
Trampoline* TBarrier_t = nullptr;

void play_itembox_sound(int pnum)
{
	auto num = TASKWK_CHARID(playertwp[pnum]);
	if (num == Characters_Gamma)
		dsPlay_oneshot(1306, 0, 0, 0);
	dsPlay_oneshot(11, 0, 0, 0);
}

int itembox_getpnum(taskwk* twp)
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

void __cdecl ef_muteki_r(taskwk* twp)
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

void __cdecl ef_5ring_r(taskwk* twp)
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

void __cdecl ef_10ring_r(taskwk* twp)
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

void __cdecl ef_random_ring_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = itembox_getpnum(twp);
		RandomRingNum = RandomRingAmounts[(int)-(rand() * 0.000030517578 * -6.0) % 6];
		AddNumRingM(pnum, RandomRingNum);
		play_itembox_sound(pnum);
	}
	else
	{
		ef_random_ring(twp);
	}
}

void __cdecl ef_baria_r(taskwk* twp)
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

void __cdecl ef_1up_r(taskwk* twp)
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

void __cdecl ef_explosion_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		play_itembox_sound(itembox_getpnum(twp));

		//ItemBox_Explosion:
		explosion_pos = twp->pos;
		explosion_r = 10.0f;

		// Todo: add hurt other players
	}
	else
	{
		ef_explosion(twp);
	}
}

void __cdecl ef_th_baria_r(taskwk* twp)
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
void __cdecl TBarrier_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pltwp = playertwp[twp->counter.b[0]];
		auto plpwp = playerpwp[twp->counter.b[0]];

		if (pltwp && plpwp && (plpwp->item & Powerups_MagneticBarrier) != 0)
		{
			EffBarrierPosSet(twp, pltwp);
			
			if ((double)rand() * 0.000030517578 > 0.70)
			{
				auto ctp = CreateChildTask(LoadObj_Data1, ThunderB, tp);
				auto ctwp = ctp->twp;

				if (ctwp)
				{
					ctwp->ang.x = (rand() * 0.000030517578 * 65536.0);
					ctwp->ang.y = (rand() * 0.000030517578 * 65536.0);
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

void __cdecl EntryItemBoxPanel_r(int panel)
{
	if (!multiplayer::IsActive())
	{
		TARGET_DYNAMIC(EntryItemBoxPanel)(panel);
	}
}

void InitItemBoxPatches()
{
	EntryItemBoxPanel_t = new Trampoline(0x4C00B0, 0x4C00B5, EntryItemBoxPanel_r);
	TBarrier_t = new Trampoline(0x4BA2A0, 0x4BA2A5, TBarrier_r);

	item_info[1].effect_func = ef_muteki_r;
	item_info[2].effect_func = ef_5ring_r;
	item_info[3].effect_func = ef_10ring_r;
	item_info[4].effect_func = ef_random_ring_r;
	item_info[5].effect_func = ef_baria_r;
	item_info[6].effect_func = ef_1up_r;
	item_info[7].effect_func = ef_explosion_r;
	item_info[8].effect_func = ef_th_baria_r;
}
#include "pch.h"
#include "objects.h"

Trampoline* Ring_t = nullptr;
Trampoline* EnemyCheckDamage_t = nullptr;

// Patch for other players to collect rings
void __cdecl Ring_r(task* tp)
{
	if (IsMultiplayerEnabled())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1)
		{
			auto player = CCL_IsHitPlayer(twp);
			
			if (player)
			{
				int pID = TASKWK_PLAYERID(player);

				if (!(playerpwp[pID]->item & 0x4000))
				{
					twp->mode = 2;
					AddRingsM(pID, 1);
					dsPlay_oneshot(7, 0, 0, 0);
					tp->disp = RingDoneDisplayer;
					return;
				}
			}
		}
	}

	TARGET_DYNAMIC(Ring)(tp);
}

// Patch for other players to get kill score
BOOL __cdecl EnemyCheckDamage_r(taskwk* twp, enemywk* ewp)
{
	if (!IsMultiplayerEnabled())
	{
		return TARGET_DYNAMIC(EnemyCheckDamage)(twp, ewp);
	}

	if (twp->flag & 4)
	{
		ewp->buyoscale = 0.35f;

		auto player = CCL_IsHitPlayer(twp);

		if (player)
		{
			int pID = TASKWK_PLAYERID(player);

			ewp->flag |= 0x1000u;
			AddScoreM(pID, 10);

			if ((twp->cwp->hit_cwp->info[twp->cwp->hit_num].attr & 0x1000) != 0)
			{
				ewp->flag |= 0x800u;
				ewp->velo.y = 0.7f;
			}
			else
			{
				ewp->flag &= ~0x400u;
				EnemyBumpPlayer(pID);
			}

			return TRUE;
		}
		else if (CCL_IsHitBullet(twp))
		{
			ewp->flag |= 0x1000u;

			if ((twp->cwp->hit_cwp->info[twp->cwp->hit_num].attr & 0x1000) != 0)
			{
				ewp->flag |= 0x800u;
			}
			else
			{
				ewp->flag &= ~0x400u;
			}
			
			return TRUE;
		}

		return FALSE;
	}
	else
	{
		return CheckItemExplosion(&twp->pos);
	}
}

// EBuyon is the only object to manually call AddEnemyScore
void __cdecl EBuyon_ScorePatch(task* tp)
{
	if (IsMultiplayerEnabled())
	{
		auto player = CCL_IsHitPlayer(tp->twp);

		if (player)
		{
			AddScoreM(TASKWK_PLAYERID(player), 100);
		}
	}
	else
	{
		AddEnemyScore(100);
	}
}

void InitObjectPatches()
{
	Ring_t = new Trampoline(0x450370, 0x450375, Ring_r);
	EnemyCheckDamage_t = new Trampoline(0x4CE030, 0x4CE036, EnemyCheckDamage_r);

	//EBuyon:
	WriteCall((void*)0x7B3273, EBuyon_ScorePatch);
	WriteData<5>((void*)0x7B326D, 0x90);
}
#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

FastFunctionHook<Float, taskwk*, int> EnemyDist2FromPlayer_t(0x4CD610);
FastFunctionHook<Angle, taskwk*, enemywk*, Uint8> EnemyCalcPlayerAngle_t(0x4CD670);
FastFunctionHook<Angle, taskwk*, enemywk*, Uint8> EnemyTurnToPlayer_t(0x4CD6F0);
FastFunctionHook<int, taskwk*, enemywk*> EnemyCheckDamage_t(0x4CE030);

// Despite taking player id, it always gets 0 so let's check closest player
float __cdecl EnemyDist2FromPlayer_r(taskwk* twp, int num)
{
	if (multiplayer::IsActive() && num == 0)
	{
		return EnemyDist2FromPlayer_t.Original(twp, GetClosestPlayerNum(&twp->pos));
	}
	else
	{
		return EnemyDist2FromPlayer_t.Original(twp, num);
	}
}

// Despite taking player id, it always gets 0 so let's check closest player
Angle __cdecl EnemyCalcPlayerAngle_r(taskwk* twp, enemywk* ewp, Uint8 pnum)
{
	if (multiplayer::IsActive() && pnum == 0)
	{
		return EnemyCalcPlayerAngle_t.Original(twp, ewp, GetClosestPlayerNum(&twp->pos));
	}
	else
	{
		return EnemyCalcPlayerAngle_t.Original(twp, ewp, pnum);
	}
}

// Despite taking player id, it always gets 0 so let's check closest player
Angle __cdecl EnemyTurnToPlayer_r(taskwk* twp, enemywk* ewp, Uint8 pnum)
{
	if (multiplayer::IsActive() && pnum == 0)
	{
		return EnemyTurnToPlayer_t.Original(twp, ewp, GetClosestPlayerNum(&twp->pos));
	}
	else
	{
		return EnemyTurnToPlayer_t.Original(twp, ewp, pnum);
	}
}

// Patch for other players to get kill score
Bool __cdecl EnemyCheckDamage_r(taskwk* twp, enemywk* ewp)
{
	if (!multiplayer::IsActive())
	{
		return EnemyCheckDamage_t.Original(twp, ewp);
	}

	if (twp->flag & 4)
	{
		ewp->buyoscale = 0.35f;

		auto hit_twp = CCL_IsHitPlayer(twp);

		if (hit_twp)
		{
			auto pID = TASKWK_PLAYERID(hit_twp);

			ewp->flag |= 0x1000;
			AddEnemyScoreM(pID, 10);

			if (twp->cwp->hit_cwp->info[twp->cwp->hit_num].attr & 0x1000)
			{
				ewp->flag |= 0x800;
				ewp->velo.y = 0.7f;
			}
			else
			{
				ewp->flag &= ~0x400;
				EnemyBumpPlayer(pID);
			}
		}

		hit_twp = CCL_IsHitBullet(twp);

		if (hit_twp)
		{
			ewp->flag |= 0x1000;
			AddEnemyScoreM(hit_twp->btimer, 10); // we put the player id in btimer

			if (twp->cwp->hit_cwp->info[twp->cwp->hit_num].attr & 0x1000)
			{
				ewp->flag |= 0x800;
			}
			else
			{
				ewp->flag &= ~0x400;
			}
		}

		return TRUE;
	}
	else
	{
		return CheckItemExplosion(&twp->pos);
	}
}

void patch_enemy_init()
{
	EnemyCheckDamage_t.Hook(EnemyCheckDamage_r);
	EnemyDist2FromPlayer_t.Hook(EnemyDist2FromPlayer_r);
	EnemyCalcPlayerAngle_t.Hook(EnemyCalcPlayerAngle_r);
	EnemyTurnToPlayer_t.Hook(EnemyTurnToPlayer_r);
	WriteData((char*)0x4CCB3F, (char)PLAYER_MAX); // EnemySearchPlayer
}

RegisterPatch patch_enemy(patch_enemy_init);
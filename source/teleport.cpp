#include "pch.h"
#include "SADXModLoader.h"
#include "multiplayer.h"
#include "camera.h"
#include "utils.h"
#include "teleport.h"

// Teleport helpers as we need to move players a lot in multiplayer
// To prevent failure, we disable the player's collision and force the position for 2 frames.

task* NoColliTp[PLAYER_MAX] = { NULL };

void NoColliDest(task* tp)
{
	anywk* awp = tp->awp;
	NoColliTp[awp->work.sw[0]] = NULL;
}

void NoColliExec(task* tp)
{
	anywk* awp = tp->awp;
	taskwk* ptwp = playertwp[awp->work.sw[0]];

	if (++awp->work.sw[1] >= 2)
	{
		if (ptwp && ptwp->cwp)
		{
			CharColliOn(ptwp);
		}

		FreeTask(tp);
	}
	else
	{
		if (ptwp)
		{
			ptwp->pos = { awp->work.f[1], awp->work.f[2], awp->work.f[3] };
		}
	}
}

void CreateNoColliTp(int pnum, float x, float y, float z)
{
	task* tp = NoColliTp[pnum];

	if (!tp)
	{
		tp = CreateElementalTask(LoadObj_UnknownB, LEV_0, NoColliExec);
		tp->dest = NoColliDest;
		tp->awp->work.sw[0] = pnum;
		NoColliTp[pnum] = tp;
	}
	else if (tp->exec == DestroyTask)
	{
		tp->exec = NoColliExec;
	}

	tp->awp->work.sw[1] = 0;
	tp->awp->work.f[1] = x;
	tp->awp->work.f[2] = y;
	tp->awp->work.f[3] = z;
}

void TeleportPlayer(int pnum, float x, float y, float z)
{
	auto ptwp = playertwp[pnum];
	if (ptwp)
	{
		SetPositionP(pnum, x, y, z);

		// Reset mode except cart/snowboard ones
		if (!IsPlayerInCart(pnum) && !IsPlayerOnSnowboard(pnum))
		{
			SetInputP(pnum, PL_OP_LETITGO);
		}

		PClearSpeed(playermwp[pnum], playerpwp[pnum]);
		
		if (ptwp->cwp)
		{
			CharColliOff(ptwp);
			CreateNoColliTp(pnum, x, y, z);
		}
	}
}

void TeleportPlayer(int pnum, NJS_VECTOR* pPos)
{
	TeleportPlayer(pnum, pPos->x, pPos->y, pPos->z);
}

void TeleportPlayers(float x, float y, float z)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		TeleportPlayer(i, x, y, z);
	}
}

void TeleportPlayers(NJS_VECTOR* pPos)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		TeleportPlayer(i, pPos->x, pPos->y, pPos->z);
	}
}

void TeleportPlayerArea(int pnum, NJS_POINT3* pin, Float dist)
{
	auto pcount = multiplayer::GetPlayerCount();
	taskwk* ptwp = playertwp[pnum];

	if (!ptwp)
		return;

	NJS_VECTOR pos;

	if (pcount <= 4)
	{
		pos.x = pin->x + njCos(ptwp->ang.y - 0x2000 + 0x4000 * pnum) * dist;
		pos.y = pin->y;
		pos.z = pin->z + njSin(ptwp->ang.y - 0x2000 + 0x4000 * pnum) * dist;
	}
	else if (pcount <= 2)
	{
		pos.y = pin->y;

		if (pnum == 0)
		{
			pos.x = pin->x + njCos(ptwp->ang.y + 0x4000) * dist;
			pos.z = pin->z + njSin(ptwp->ang.y + 0x4000) * dist;
		}
		else
		{
			pos.x = pin->x + njCos(ptwp->ang.y - 0x4000) * dist;
			pos.z = pin->z + njSin(ptwp->ang.y - 0x4000) * dist;
		}
	}
	else
	{
		pos = *pin;
	}

	TeleportPlayer(pnum, &pos);
}

void TeleportPlayersToPlayer(int pnum)
{
	taskwk* ptgt = playertwp[pnum];

	if (ptgt)
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			taskwk* ptwp = playertwp[i];
			if (ptwp)
			{
				TeleportPlayer(i, &ptgt->pos);
				ptwp->ang.y = ptgt->ang.y;
			}
		}
	}
}

void TeleportPlayerToStart(int pnum)
{
	taskwk* ptwp = playertwp[pnum];
	if (ptwp)
	{
		NJS_POINT3 pos; Angle3 ang;
		GetPlayerInitialPositionM(&pos, &ang);

		ptwp->ang = ang;
		TeleportPlayer(pnum, &pos);
	}
}

void TeleportPlayersToStart()
{
	NJS_POINT3 pos; Angle3 ang;
	GetPlayerInitialPositionM(&pos, &ang);
	
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			Float offset = 0.0f;
			if (GetHoldingItemIDP(i)) 
			{
				offset = 8.0f; //fix goofy collision making item held by player despawning 
			}

			ptwp->ang = ang;
			TeleportPlayerArea(i, &pos, 5.0f + offset);
		}
	}
}

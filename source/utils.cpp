#include "pch.h"
#include "camera.h"
#include "splitscreen.h"
#include "teleport.h"
#include "utils.h"

float GetDistance(NJS_VECTOR* v1, NJS_VECTOR* v2)
{
	return sqrtf((v2->x - v1->x) * (v2->x - v1->x) +
		(v2->y - v1->y) * (v2->y - v1->y) +
		(v2->z - v1->z) * (v2->z - v1->z));
}

float GetDistance2(NJS_VECTOR* v1, NJS_VECTOR* v2)
{
	return (v2->x - v1->x) * (v2->x - v1->x) +
		(v2->y - v1->y) * (v2->y - v1->y) +
		(v2->z - v1->z) * (v2->z - v1->z);
}

//trick the game to make a character able to access every areas
int __cdecl GetCharacterID_r(char index)
{
	if (multiplayer::IsActive())
	{
		return CurrentCharacter;
	}
	else {
		return GetCharacterID(index);
	}
}

int GetClosestPlayerNumRange(NJS_POINT3* pos, float range)
{
	float max = range;
	int num = -1;
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];
		if (ptwp && ptwp->cwp)
		{
			auto dist = GetDistance(pos, &ptwp->cwp->info->center);
			if (dist < max)
			{
				max = dist;
				num = i;
			}
		}
	}
	return num;
}

int GetClosestPlayerNumRange(float x, float y, float z, float range)
{
	NJS_POINT3 pos = { x, y, z };
	return GetClosestPlayerNumRange(&pos, range);
}

int GetClosestPlayerNum(NJS_POINT3* pos)
{
	return GetClosestPlayerNumRange(pos, 100000000.0f);
}

int GetClosestPlayerNum(float x, float y, float z)
{
	NJS_POINT3 pos = { x, y, z };
	return GetClosestPlayerNum(&pos);
}

int IsPlayerInSphere(NJS_POINT3* p, float r)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		taskwk* ptwp = playertwp[i];
		
		if (ptwp && ptwp->cwp)
		{
			if (GetDistance(&ptwp->cwp->info->center, p) < r)
			{
				return i + 1;
			}
		}
	}

	return 0;
}

int IsPlayerInSphere(float x, float y, float z, float r)
{
	NJS_VECTOR p = { x, y, z };
	return IsPlayerInSphere(&p, r);
}

bool IsCameraInSphere(NJS_POINT3* p, float r)
{
	if (splitscreen::IsActive())
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (splitscreen::IsScreenEnabled(i))
			{
				if (GetDistance(GetCameraPosition(i), p) < r)
				{
					return true;
				}
			}
		}
		return false;
	}
	else if (camera_twp)
	{
		return GetDistance(&camera_twp->pos, p) < r;
	}
	else
	{
		return IsPlayerInSphere(p, r);
	}
}

bool IsCameraInSphere(float x, float y, float z, float r)
{
	NJS_VECTOR p = { x, y, z };
	return IsCameraInSphere(&p, r);
}

int IsPlayerOnDyncol(task* tp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (CheckPlayerRideOnMobileLandObjectP(i, tp))
		{
			return i + 1;
		}
	}

	return 0;
}

void ChangeActM(int amount)
{
	ADX_Close();
	LandChangeStage(amount);
	AddSetStage(amount);
	AddCameraStage(amount);
	AdvanceAct(amount);
	TeleportPlayersToStart();
}

bool IsInAdventureField()
{
	return CurrentLevel >= LevelIDs_StationSquare && CurrentLevel <= LevelIDs_Past;
}

bool IsPlayerInCart(char pnum)
{
	auto p = playertwp[pnum];

	if (p)
	{
		auto mde = p->mode;
		switch (p->counter.b[1])
		{
		case Characters_Sonic:
		default:
			return mde == 45;
		case Characters_Tails:
			return mde == 43;
		case Characters_Knuckles:
			return mde == 52;
		case Characters_Amy:
			return mde == 48;
		case Characters_Gamma:
			return mde == 53;
		case Characters_Big:
			return mde == 55;
		}
	}

	return false;
}

bool IsPlayerOnSnowboard(char pnum)
{
	auto p = playertwp[pnum];

	if (p)
	{
		switch (TASKWK_CHARID(p))
		{
		case Characters_Sonic:
			return p->mode >= 62 && p->mode <= 68;
		case Characters_Tails:
			return p->mode >= 48 && p->mode <= 54;
		}
	}

	return false;
}

int GetHoldingPlayerId(task* htp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playerpwp[i] && playerpwp[i]->htp == htp)
		{
			return i;
		}
	}
	return 0;
}

int GetBossTargetPlayerRandom()
{
	static int pnum = 0;
	static int timer = 0;
	if (ulGlobalTimer - timer > 500 || !playertwp[pnum])
	{
		pnum = rand() % multiplayer::GetPlayerCount();
		timer = ulGlobalTimer;

		while (!playertwp[pnum])
		{
			pnum = rand() % multiplayer::GetPlayerCount();
		}
	}
	return pnum;
}

static int curPnum = 0;
int GetBossTargetPlayerWithTimer(NJS_POINT3* pos)
{

	static int timer = 0;
	if (ulGlobalTimer - timer > 500 || !playertwp[curPnum])
	{
		curPnum = GetTheNearestPlayerNumber(pos);
		timer = ulGlobalTimer;
	}

	return curPnum;
}
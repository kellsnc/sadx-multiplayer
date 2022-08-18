#include "pch.h"
#include "camera.h"
#include "splitscreen.h"

void DrawSADXText(const char* text, __int16 y)
{
	NH_MSG_Close(&jimakumsgc);
	DialogJimakuInit();
	NH_MSG_Open(&jimakumsgc, 0, y, 0, 0, 0xFF000020, jimakubuf);
	DoSomethingRelatedToText_(text);
	MSG_LoadTexture(&jimakumsgc);
}

short tolevelnum(short num)
{
	return num >> 8;
}

short toactnum(short num)
{
	return num & 0xf;
}

void SetAllPlayersInitialPosition()
{
	NJS_POINT3 pos; Angle3 ang;
	GetPlayerInitialPositionM(&pos, &ang);

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			PClearSpeed(playermwp[i], playerpwp[i]);
			SetInputP(i, PL_OP_LETITGO);
			ptwp->pos = pos;
			ptwp->ang = ang;
		}
	}
}

void SetAllPlayersPosition(float x, float y, float z, Angle angy)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			PClearSpeed(playermwp[i], playerpwp[i]);
			SetInputP(i, PL_OP_LETITGO);
			ptwp->pos = { x, y, z };
			ptwp->ang = { 0, angy, 0 };
		}
	}
}

float GetDistance(NJS_VECTOR* v1, NJS_VECTOR* v2)
{
	return sqrtf((v2->x - v1->x) * (v2->x - v1->x) +
		(v2->y - v1->y) * (v2->y - v1->y) +
		(v2->z - v1->z) * (v2->z - v1->z));
}

int GetClosestPlayerNum(NJS_POINT3* pos)
{
	float max = 100000000.0f;
	int num = 0;
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];
		if (ptwp)
		{
			auto dist = GetDistance(pos, &ptwp->pos);
			if (dist < max)
			{
				max = dist;
				num = i;
			}
		}
	}
	return num;
}

int GetClosestPlayerNum(float x, float y, float z)
{
	NJS_POINT3 pos = { x, y, z };
	return GetClosestPlayerNum(&pos);
}

int GetClosestPlayerNumRange(NJS_POINT3* pos, float range)
{
	float max = range;
	int num = -1;
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];
		if (ptwp)
		{
			auto dist = GetDistance(pos, &ptwp->pos);
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

int IsPlayerInSphere(NJS_POINT3* p, float r)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertwp[i])
		{
			if (GetDistance(&playertwp[i]->pos, p) < r)
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
	if (SplitScreen::IsActive())
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (SplitScreen::IsScreenEnabled(i))
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

void njRotateX_(Angle ang)
{
	if (ang)
	{
		njRotateX(nullptr, (uint16_t)ang);
	}
}

void njRotateY_(Angle ang)
{
	if (ang)
	{
		njRotateY(nullptr, (uint16_t)ang);
	}
}

void njRotateZ_(Angle ang)
{
	if (ang)
	{
		njRotateZ(nullptr, (uint16_t)ang);
	}
}

void ChangeActM(int amount)
{
	ADX_Close();
	LandChangeStage(amount);
	AddSetStage(amount);
	AddCameraStage(amount);
	AdvanceAct(amount);
	SetAllPlayersInitialPosition();
}
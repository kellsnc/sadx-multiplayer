#include "pch.h"

void DrawSADXText(const char* text, __int16 y)
{
	MSG_Close(&jimakumsgc);
	DialogJimakuInit();
	NH_MSG_Open(&jimakumsgc, 0, y, 0, 0, 0xFF000020, jimakubuf);
	DoSomethingRelatedToText_(text);
	MSG_LoadTexture(&jimakumsgc);
}

int MenuSelectButtonsPressedM(int pnum)
{
	return PressedButtons[pnum] & (Buttons_Start | Buttons_A);
}

int MenuBackButtonsPressedM(int pnum)
{
	return PressedButtons[pnum] & (Buttons_X | Buttons_B);
}

short tolevelnum(short num)
{
	return num >> 8;
}

short toactnum(short num)
{
	return num & 0xf;
}

void ToggleControllers(bool enabled)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (enabled)
		{
			EnableController(i);
		}
		else
		{
			DisableController(i);
		}

		ControllerEnabled[i] = enabled;
	}
}

void TeleportPlayersInitialPosition(int pNum)
{
	// Save time because SetPlayerInitialPosition resets it
	auto min = TimeMinutes;
	auto sec = TimeSeconds;
	auto frm = TimeFrames;

	SetPlayerInitialPosition(playertwp[pNum]);

	// Restore time
	TimeMinutes = min;
	TimeSeconds = sec;
	TimeFrames = frm;
}

void SetAllPlayersInitialPosition()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertwp[i])
		{
			SetPlayerInitialPosition(playertwp[i]);
		}
	}
}

float GetDistance(NJS_VECTOR* v1, NJS_VECTOR* v2)
{
	return sqrtf((v2->x - v1->x) * (v2->x - v1->x) +
		(v2->y - v1->y) * (v2->y - v1->y) +
		(v2->z - v1->z) * (v2->z - v1->z));
}

bool IsPlayerInSphere(float x, float y, float z, float r)
{
	NJS_VECTOR p = { x, y, z };

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertwp[i])
		{
			if (GetDistance(&playertwp[i]->pos, &p) < r)
			{
				return true;
			}
		}
	}

	return false;
}
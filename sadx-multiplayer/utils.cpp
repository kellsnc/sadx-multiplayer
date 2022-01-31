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
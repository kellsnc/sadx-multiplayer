#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "splitscreen.h"
#include "network.h"
#include "camera.h"

Trampoline* GetPlayersInputData_t = nullptr;

static void __cdecl PadReadOnP_r(int8_t pnum)
{
	if (pnum == -1)
	{
		for (size_t i = 0ui32; i < ucInputStatusForEachPlayer.size(); ++i)
		{
			ucInputStatusForEachPlayer[i] = TRUE;
		}
	}
	else
	{
		ucInputStatusForEachPlayer[pnum] = TRUE;
	}
}

static void __cdecl PadReadOffP_r(int8_t pnum)
{
	if (pnum < 0)
	{
		for (size_t i = 0ui32; i < ucInputStatusForEachPlayer.size(); ++i)
		{
			ucInputStatusForEachPlayer[i] = FALSE;
		}
	}
	else
	{
		ucInputStatusForEachPlayer[pnum] = FALSE;
	}
}

// Patch analog forward calculation to use multiplayer cameras
static void __cdecl GetPlayersInputData_r()
{
	if (!SplitScreen::IsActive())
	{
		TARGET_DYNAMIC(GetPlayersInputData)();
		return;
	}

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto controller = per[i];
		float lx = static_cast<float>(controller->x1 << 8); // left stick x
		float ly = static_cast<float>(controller->y1 << 8); // left stick y

		Angle ang;
		float strk;

		if (lx > 3072.0f || lx < -3072.0f || ly > 3072.0f || ly < -3072.0f)
		{
			lx = lx <= 3072.0f ? (lx >= -3072.0f ? 0.0f : lx + 3072.0f) : lx - 3072.0f;
			ly = ly <= 3072.0f ? (ly >= -3072.0f ? 0.0f : ly + 3072.0f) : ly - 3072.0f;

			auto cam_ang = GetCameraAngle(i);

			if (cam_ang)
			{
				ang = -cam_ang->y - NJM_RAD_ANG(-atan2f(ly, lx));
			}
			else
			{
				ang = NJM_RAD_ANG(-atan2f(ly, lx));
			}

			double magnitude = ly * ly + lx * lx;
			strk = static_cast<float>(sqrt(magnitude) * magnitude * 3.9187027e-14);
			if (strk > 1.0f)
			{
				strk = 1.0f;
			}
		}
		else
		{
			strk = 0.0f;
			ang = 0;
		}

		input_data[i] = { ang, strk };

		if (ucInputStatus == 1 && (i >= 4 || ucInputStatusForEachPlayer[i] == 1))
		{
			input_dataG[i] = input_data[i];
		}
		else
		{
			input_dataG[i] = {};
		}
	}
}

void InitInputPatches()
{
	GetPlayersInputData_t = new Trampoline(0x40F170, 0x40F175, GetPlayersInputData_r);

	// Patch controller toggles for 4 players
	WriteJump(PadReadOnP, PadReadOnP_r);
	WriteJump(PadReadOffP, PadReadOffP_r);
}

unsigned int GetPressedButtons(int pnum)
{
	return PressedButtons[pnum];
}

unsigned int GetPressedButtons()
{
	unsigned int buttons = 0;
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		buttons |= GetPressedButtons(i);
	}
	return buttons;
}

bool MenuSelectButtonsPressedM(int pnum)
{
	return (GetPressedButtons(pnum) & (Buttons_Start | Buttons_A));
}

bool MenuSelectButtonsPressedM()
{
	return (GetPressedButtons() & (Buttons_Start | Buttons_A));
}

bool MenuBackButtonsPressedM(int pnum)
{
	return (GetPressedButtons(pnum) & (Buttons_X | Buttons_B));
}

bool MenuBackButtonsPressedM()
{
	return (GetPressedButtons() & (Buttons_X | Buttons_B));
}

extern "C"
{
	__declspec(dllexport) void __cdecl OnInput()
	{
		network.PollInputs();
	}

	__declspec(dllexport) void __cdecl OnControl()
	{
		// Allow input from the rest of the players
		for (int i = 2; i < PLAYER_MAX; i++)
		{
			if (CheckPadReadModeP(i))
			{
				Controllers[i] = *ControllerPointers[i];
			}
			else
			{
				Controllers[i] = {};
			}
		}
	}
}

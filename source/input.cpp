#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "splitscreen.h"
#include "network.h"
#include "camera.h"

// Everything related to inputs
// Networking part of the code originally by @michael-fadely

DataPointer(bool*, pInputStatusForEachPlayer, 0x40F30C); // we get a pointer to `ucInputStatusForEachPlayer` since the input mod replaces the array

static SONIC_INPUT net_analogs[PLAYER_MAX]{};
static PDS_PERIPHERAL net_pers[PLAYER_MAX]{};

FunctionHook<void, Sint8> PadReadOnP_hook(0x40EF70);
FunctionHook<void, Sint8> PadReadOffP_hook(0x40EFA0);
FunctionHook<void>        GetPlayersInputData_hook(0x40F170);

static void __cdecl PadReadOnP_r(int8_t pnum)
{
	if (!multiplayer::IsActive())
	{
		PadReadOnP_hook.Original(pnum);
		return;
	}

	if (pnum == -1)
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			pInputStatusForEachPlayer[i] = TRUE;
		}
	}
	else
	{
		pInputStatusForEachPlayer[pnum] = TRUE;
	}
}

static void __cdecl PadReadOffP_r(int8_t pnum)
{
	if (!multiplayer::IsActive())
	{
		PadReadOffP_hook.Original(pnum);
		return;
	}

	if (pnum < 0)
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			pInputStatusForEachPlayer[i] = FALSE;
		}
	}
	else
	{
		pInputStatusForEachPlayer[pnum] = FALSE;
	}
}

// Patch analog forward calculation to use multiplayer cameras
static void __cdecl GetPlayersInputData_r()
{
	if (!SplitScreen::IsActive())
	{
		GetPlayersInputData_hook.Original();
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

		if (ucInputStatus == TRUE && pInputStatusForEachPlayer[i])
		{
			input_dataG[i] = input_data[i];
		}
		else
		{
			input_dataG[i] = {};
		}
	}
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

static bool InputListener(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum)
{
	switch (type)
	{
	case Network::PACKET_INPUT_BUTTONS:
		packet >> net_pers[pnum].on;
		return true;
	case Network::PACKET_INPUT_STICK_X:
		packet >> net_pers[pnum].x1 >> net_pers[pnum].x2;
		return true;
	case Network::PACKET_INPUT_STICK_Y:
		packet >> net_pers[pnum].y1 >> net_pers[pnum].y2;
		return true;
	case Network::PACKET_INPUT_ANALOG:
		packet >> net_analogs[pnum];
		return true;
	default:
		return false;
	}
}

static bool InputSender(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum)
{
	switch (type)
	{
	case Network::PACKET_INPUT_BUTTONS:
		packet << net_pers[pnum].on;
		return true;
	case Network::PACKET_INPUT_STICK_X:
		packet << net_pers[pnum].x1 << net_pers[pnum].x2;
		return true;
	case Network::PACKET_INPUT_STICK_Y:
		packet << net_pers[pnum].y1 << net_pers[pnum].y2;
		return true;
	case Network::PACKET_INPUT_ANALOG:
		packet << net_analogs[pnum];
		return true;
	default:
		return false;
	}
}

void InitInputPatches()
{
	// Enable inputs for the other players
	for (int i = 2; i < PLAYER_MAX; i++)
	{
		pInputStatusForEachPlayer[i] = TRUE;
	}

	PadReadOnP_hook.Hook(PadReadOnP_r);
	PadReadOffP_hook.Hook(PadReadOffP_r);
	GetPlayersInputData_hook.Hook(GetPlayersInputData_r);

	network.RegisterListener(Network::PACKET_INPUT_BUTTONS, InputListener);
	network.RegisterListener(Network::PACKET_INPUT_STICK_X, InputListener);
	network.RegisterListener(Network::PACKET_INPUT_STICK_Y, InputListener);
	network.RegisterListener(Network::PACKET_INPUT_ANALOG, InputListener);
}

extern "C"
{
	__declspec(dllexport) void __cdecl OnInput()
	{
		if (!network.IsConnected())
			return;

		// Update local/online controller data
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto local_per = per[i];
			auto& net_per = net_pers[i];

			if (i == network.GetPlayerNum())
			{
				if (local_per->press || local_per->release)
				{
					net_per.on = local_per->on;
					network.Send(Network::PACKET_INPUT_BUTTONS, InputSender);
				}

				if (local_per->x1 != net_per.x1 || local_per->x2 != net_per.x2)
				{
					net_per.x1 = local_per->x1;
					net_per.x2 = local_per->x2;
					network.Send(Network::PACKET_INPUT_STICK_X, InputSender);
				}

				if (local_per->y1 != net_per.y1 || local_per->y2 != net_per.y2)
				{
					net_per.y1 = local_per->y1;
					net_per.y2 = local_per->y2;
					network.Send(Network::PACKET_INPUT_STICK_Y, InputSender);
				}
			}
			else
			{
				local_per->x1 = net_per.x1;
				local_per->x2 = net_per.x2;
				local_per->y1 = net_per.y1;
				local_per->y2 = net_per.y2;

				local_per->on = net_per.on;
				local_per->off = ~local_per->on;

				const auto mask = local_per->on ^ net_per.old;
				local_per->release = net_per.old & mask;
				local_per->press = local_per->on & mask;

				net_per.old = local_per->old = local_per->on;

				local_per->l = local_per->on & Buttons_L ? UCHAR_MAX : 0;
				local_per->r = local_per->on & Buttons_R ? UCHAR_MAX : 0;
			}
		}
	}

	__declspec(dllexport) void __cdecl OnControl()
	{
		// Allow input from the rest of the players
		for (int i = 2; i < PLAYER_MAX; i++)
		{
			if (CheckPadReadModeP(i))
			{
				perG[i] = *per[i];
			}
			else
			{
				perG[i] = {};
			}
		}

		// Update network analog data
		if (network.IsConnected())
		{
			for (auto i = 0; i < PLAYER_MAX; i++)
			{
				auto& current = input_dataG[i];
				auto& net_analog = net_analogs[i];

				if (i == network.GetPlayerNum())
				{
					if (current.angle != net_analog.angle || std::abs(current.stroke - net_analog.stroke) >= std::numeric_limits<float>::epsilon())
					{
						net_analog = current;
						network.Send(Network::PACKET_INPUT_ANALOG, InputSender);
					}
				}
				else
				{
					input_dataG[i] = net_analog;
				}
			}
		}
	}
}

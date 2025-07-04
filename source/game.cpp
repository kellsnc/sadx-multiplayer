#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "netplay.h"
#include "timer.h"
#include "multiplayer.h"
#include "config.h"

// Game manager
// Manage how the game works in multiplayer, especially in netplay

#ifdef MULTI_NETPLAY
static Timer game_timer(std::chrono::steady_clock::duration(std::chrono::milliseconds(500)));

static int netGlobalMode;
static short netGameMode;
static short netStageNumber;
static int netActNumber;

static int oldTimerWake;
static int oldPauseEnabled;

FastFunctionHook<void, int> AdvanceActLocal_h(0x4146E0);
FastFunctionHook<void, Sint16> SetChangeGameMode_h(0x413C90);
FastFunctionHook<void, Sint8, Sint8> ChangeStageWithFadeOut_h(0x4145D0);
FastFunctionHook<void> RestartStageWithFadeOut_h(0x414600);

static bool GameListener(Packet& packet, Netplay::PACKET_TYPE type, Netplay::PNUM pnum)
{
	switch (type)
	{
	case Netplay::PACKET_GAME_TIMER:
		packet >> GameTimer >> ulGlobalTimer >> gu32GameCnt >> gu32LocalCnt;
		return true;
	case Netplay::PACKET_GAME_CLOCK:
		packet >> bWake >> TimeMinutes >> TimeSeconds >> TimeFrames;
		return true;
	case Netplay::PACKET_GAME_MODE:
		packet >> netGlobalMode >> netGameMode >> netStageNumber >> netActNumber;

		if (netGameMode == MD_GAME_PAUSE && ssGameMode == MD_GAME_MAIN)
			ssGameMode = MD_GAME_PAUSE;
		else if (netGameMode == MD_GAME_MAIN && ssGameMode == MD_GAME_PAUSE)
			ssGameMode = MD_GAME_MAIN;
		else if ((netGameMode == MD_GAME_END || netGameMode == MD_GAME_END2) && (ssGameMode != MD_GAME_END || ssGameMode != MD_GAME_END2))
			ssGameMode = MD_GAME_END;

		return true;
	case Netplay::PACKET_GAME_LEVEL:
		packet >> ssStageNumber >> ssActNumber;
		return true;
	case Netplay::PACKET_GAME_ACTSWAP:
	{
		int act;
		packet >> act;
		if (act != ssActNumber)
		{
			act -= ssActNumber;
			ChangeActM(act);
		}
		return true;
	}
	case Netplay::PACKET_GAME_EXIT:
		packet >> ssGameModeChange;
		return true;
	case Netplay::PACKET_GAME_STAGECHG:
		packet >> ssGameModeChange >> ssNextStageNumber >> ssNextActNumber;
		return true;
	case Netplay::PACKET_GAME_PAUSE:
		packet >> PauseEnabled >> PauseSelection;
		return true;
	case Netplay::PACKET_GAME_RAND:
		packet >> seed;
		return true;
	default:
		return false;
	}
}

static bool GameSender(Packet& packet, Netplay::PACKET_TYPE type, Netplay::PNUM pnum)
{
	switch (type)
	{
	case Netplay::PACKET_GAME_TIMER:
		packet << GameTimer << ulGlobalTimer << gu32GameCnt << gu32LocalCnt;
		return true;
	case Netplay::PACKET_GAME_CLOCK:
		packet << bWake << TimeMinutes << TimeSeconds << TimeFrames;
		return true;
	case Netplay::PACKET_GAME_MODE:
		packet << ulGlobalMode << ssGameMode;
		return true;
	case Netplay::PACKET_GAME_LEVEL:
		packet << ssStageNumber << ssActNumber;
		return true;
	case Netplay::PACKET_GAME_ACTSWAP:
		packet << ssActNumber;
		return true;
	case Netplay::PACKET_GAME_EXIT:
		packet << ssGameModeChange;
		return true;
	case Netplay::PACKET_GAME_STAGECHG:
		packet << ssGameModeChange << ssNextStageNumber << ssNextActNumber;
		return true;
	case Netplay::PACKET_GAME_PAUSE:
		packet << PauseEnabled << PauseSelection;
		return true;
	case Netplay::PACKET_GAME_RAND:
		packet << seed;
		return true;
	default:
		return false;
	}
}

static void AdvanceActLocal_r(int ssActAddition)
{
	AdvanceActLocal_h.Original(ssActAddition);
	netplay.Send(Netplay::PACKET_GAME_EXIT, GameSender, -1, true);
}

static void SetChangeGameMode_r(Sint16 mode)
{
	if (netplay.IsConnected())
	{
		auto old = ssGameModeChange;
		SetChangeGameMode_h.Original(mode);

		if (ssGameModeChange != old)
		{
			netplay.Send(Netplay::PACKET_GAME_EXIT, GameSender, -1, true);
		}
	}
	else
	{
		SetChangeGameMode_h.Original(mode);
	}
}

static void ChangeStageWithFadeOut_r(Sint8 stg, Sint8 act)
{
	ChangeStageWithFadeOut_h.Original(stg, act);
	netplay.Send(Netplay::PACKET_GAME_STAGECHG, GameSender, -1, true);
}

static void RestartStageWithFadeOut_r()
{
	RestartStageWithFadeOut_h.Original();
	netplay.Send(Netplay::PACKET_GAME_EXIT, GameSender, -1, true);
}
#endif

void ExecGameManager()
{
#ifdef MULTI_NETPLAY
	if (netplay.IsConnected())
	{
		netplay.Poll();

		if (netplay.GetPlayerNum() == 0)
		{
			if (netGlobalMode != ulGlobalMode || netGameMode != ssGameMode)
			{
				netplay.Send(Netplay::PACKET_GAME_MODE, GameSender, -1, true);
				netGlobalMode = ulGlobalMode;
				netGameMode = ssGameMode;
			}

			if (netStageNumber != ssStageNumber || netActNumber != ssActNumber)
			{
				netplay.Send(Netplay::PACKET_GAME_LEVEL, GameSender, -1, true);
				netStageNumber = ssStageNumber;
				netActNumber = ssActNumber;
			}

			if (ssGameMode == 16 || PauseEnabled != oldPauseEnabled)
			{
				netplay.Send(Netplay::PACKET_GAME_PAUSE, GameSender);
				oldPauseEnabled = PauseEnabled;
			}

			if (game_timer.Finished())
			{
				netplay.Send(Netplay::PACKET_GAME_MODE, GameSender);
				netplay.Send(Netplay::PACKET_GAME_TIMER, GameSender);

				if (oldTimerWake != bWake || bWake == TRUE)
				{
					netplay.Send(Netplay::PACKET_GAME_CLOCK, GameSender);
					oldTimerWake = bWake;
				}
			}

			netplay.Send(Netplay::PACKET_GAME_RAND, GameSender, -1, true);
		}
	}
#endif
}

static void __cdecl InitTimer_r()
{
	TimeFrames = 0;
	TimeSeconds = 0;
	TimeMinutes = 0;
	GameTimer = 0;

	if (CurrentCharacter == Characters_Gamma && !multiplayer::IsBattleMode())
	{
		TimeMinutes = 3;
		TimeSeconds = 0;
	}
}

static void __cdecl InitTime_r()
{
	ulGlobalTimer = 0;
	InitTimer_r();
}

void __cdecl SetTimeFrame_r(Sint8 minutes, Sint8 second, Sint8 frame)
{
	if (multiplayer::IsActive() && GameState == 15)
		return;

	TimeMinutes = minutes,
		TimeSeconds = second;
	TimeFrames = frame;
}

void InitGameManager()
{
	// Normal start timer for Gamma in multiplayer
	WriteJump((void*)0x425FF0, InitTimer_r);
	WriteJump((void*)0x427F10, InitTime_r);

	// Don't reset time with death in multiplayer
	WriteCall((void*)0x44EE0A, SetTimeFrame_r);

#ifdef MULTI_NETPLAY
	netplay.RegisterListener(Netplay::PACKET_GAME_TIMER, GameListener);
	netplay.RegisterListener(Netplay::PACKET_GAME_CLOCK, GameListener);
	netplay.RegisterListener(Netplay::PACKET_GAME_MODE, GameListener);
	netplay.RegisterListener(Netplay::PACKET_GAME_LEVEL, GameListener);
	netplay.RegisterListener(Netplay::PACKET_GAME_ACTSWAP, GameListener);
	netplay.RegisterListener(Netplay::PACKET_GAME_EXIT, GameListener);
	netplay.RegisterListener(Netplay::PACKET_GAME_STAGECHG, GameListener);
	netplay.RegisterListener(Netplay::PACKET_GAME_PAUSE, GameListener);
	netplay.RegisterListener(Netplay::PACKET_GAME_RAND, GameListener);

	AdvanceActLocal_h.Hook(AdvanceActLocal_r);
	SetChangeGameMode_h.Hook(SetChangeGameMode_r);
	ChangeStageWithFadeOut_h.Hook(ChangeStageWithFadeOut_r);
	RestartStageWithFadeOut_h.Hook(RestartStageWithFadeOut_r);
#endif
}
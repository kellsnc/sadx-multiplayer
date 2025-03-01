#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "netplay.h"
#include "timer.h"
#include "players.h"
#include "config.h"

#ifdef MULTI_NETPLAY
static Timer logic_timer(std::chrono::steady_clock::duration(std::chrono::milliseconds(500)));

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

static bool LogicListener(Packet& packet, Netplay::PACKET_TYPE type, Netplay::PNUM pnum)
{
	switch (type)
	{
	case Netplay::PACKET_LOGIC_TIMER:
		packet >> GameTimer >> ulGlobalTimer >> gu32GameCnt >> gu32LocalCnt;
		return true;
	case Netplay::PACKET_LOGIC_CLOCK:
		packet >> bWake >> TimeMinutes >> TimeSeconds >> TimeFrames;
		return true;
	case Netplay::PACKET_LOGIC_MODE:
		packet >> netGlobalMode >> netGameMode >> netStageNumber >> netActNumber;

		if (netGameMode == MD_GAME_PAUSE && ssGameMode == MD_GAME_MAIN)
			ssGameMode = MD_GAME_PAUSE;
		else if (netGameMode == MD_GAME_MAIN && ssGameMode == MD_GAME_PAUSE)
			ssGameMode = MD_GAME_MAIN;
		else if ((netGameMode == MD_GAME_END || netGameMode == MD_GAME_END2) && (ssGameMode != MD_GAME_END || ssGameMode != MD_GAME_END2))
			ssGameMode = MD_GAME_END;

		return true;
	case Netplay::PACKET_LOGIC_LEVEL:
		packet >> ssStageNumber >> ssActNumber;
		return true;
	case Netplay::PACKET_LOGIC_ACTSWAP:
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
	case Netplay::PACKET_LOGIC_EXIT:
		packet >> ssGameModeChange;
		return true;
	case Netplay::PACKET_LOGIC_STAGECHG:
		packet >> ssGameModeChange >> ssNextStageNumber >> ssNextActNumber;
		return true;
	case Netplay::PACKET_LOGIC_PAUSE:
		packet >> PauseEnabled >> PauseSelection;
		return true;
	case Netplay::PACKET_LOGIC_RAND:
		packet >> seed;
		return true;
	default:
		return false;
	}
}

static bool LogicSender(Packet& packet, Netplay::PACKET_TYPE type, Netplay::PNUM pnum)
{
	switch (type)
	{
	case Netplay::PACKET_LOGIC_TIMER:
		packet << GameTimer << ulGlobalTimer << gu32GameCnt << gu32LocalCnt;
		return true;
	case Netplay::PACKET_LOGIC_CLOCK:
		packet << bWake << TimeMinutes << TimeSeconds << TimeFrames;
		return true;
	case Netplay::PACKET_LOGIC_MODE:
		packet << ulGlobalMode << ssGameMode;
		return true;
	case Netplay::PACKET_LOGIC_LEVEL:
		packet << ssStageNumber << ssActNumber;
		return true;
	case Netplay::PACKET_LOGIC_ACTSWAP:
		packet << ssActNumber;
		return true;
	case Netplay::PACKET_LOGIC_EXIT:
		packet << ssGameModeChange;
		return true;
	case Netplay::PACKET_LOGIC_STAGECHG:
		packet << ssGameModeChange << ssNextStageNumber << ssNextActNumber;
		return true;
	case Netplay::PACKET_LOGIC_PAUSE:
		packet << PauseEnabled << PauseSelection;
		return true;
	case Netplay::PACKET_LOGIC_RAND:
		packet << seed;
		return true;
	default:
		return false;
	}
}

static void AdvanceActLocal_r(int ssActAddition)
{
	AdvanceActLocal_h.Original(ssActAddition);
	netplay.Send(Netplay::PACKET_LOGIC_EXIT, LogicSender, -1, true);
}

static void SetChangeGameMode_r(Sint16 mode)
{
	if (netplay.IsConnected())
	{
		auto old = ssGameModeChange;
		SetChangeGameMode_h.Original(mode);

		if (ssGameModeChange != old)
		{
			netplay.Send(Netplay::PACKET_LOGIC_EXIT, LogicSender, -1, true);
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
	netplay.Send(Netplay::PACKET_LOGIC_STAGECHG, LogicSender, -1, true);
}

static void RestartStageWithFadeOut_r()
{
	RestartStageWithFadeOut_h.Original();
	netplay.Send(Netplay::PACKET_LOGIC_EXIT, LogicSender, -1, true);
}
#endif

extern "C"
{
	__declspec(dllexport) void __cdecl OnFrame()
	{
		if (ChkGameMode())
		{
			UpdatePlayersInfo();
		}

#ifdef MULTI_NETPLAY
		if (netplay.IsConnected())
		{
			netplay.Poll();

			if (netplay.GetPlayerNum() == 0)
			{
				if (netGlobalMode != ulGlobalMode || netGameMode != ssGameMode)
				{
					netplay.Send(Netplay::PACKET_LOGIC_MODE, LogicSender, -1, true);
					netGlobalMode = ulGlobalMode;
					netGameMode = ssGameMode;
				}

				if (netStageNumber != ssStageNumber || netActNumber != ssActNumber)
				{
					netplay.Send(Netplay::PACKET_LOGIC_LEVEL, LogicSender, -1, true);
					netStageNumber = ssStageNumber;
					netActNumber = ssActNumber;
				}

				if (ssGameMode == 16 || PauseEnabled != oldPauseEnabled)
				{
					netplay.Send(Netplay::PACKET_LOGIC_PAUSE, LogicSender);
					oldPauseEnabled = PauseEnabled;
				}

				if (logic_timer.Finished())
				{
					netplay.Send(Netplay::PACKET_LOGIC_MODE, LogicSender);
					netplay.Send(Netplay::PACKET_LOGIC_TIMER, LogicSender);

					if (oldTimerWake != bWake || bWake == TRUE)
					{
						netplay.Send(Netplay::PACKET_LOGIC_CLOCK, LogicSender);
						oldTimerWake = bWake;
					}
				}

				netplay.Send(Netplay::PACKET_LOGIC_RAND, LogicSender, -1, true);
			}
		}
#endif
	}
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

void InitLogic()
{
	// Normal start timer for Gamma in multiplayer
	WriteJump((void*)0x425FF0, InitTimer_r);
	WriteJump((void*)0x427F10, InitTime_r);

	// Don't reset time with death in multiplayer
	WriteCall((void*)0x44EE0A, SetTimeFrame_r);

#ifdef MULTI_NETPLAY
	netplay.RegisterListener(Netplay::PACKET_LOGIC_TIMER, LogicListener);
	netplay.RegisterListener(Netplay::PACKET_LOGIC_CLOCK, LogicListener);
	netplay.RegisterListener(Netplay::PACKET_LOGIC_MODE, LogicListener);
	netplay.RegisterListener(Netplay::PACKET_LOGIC_LEVEL, LogicListener);
	netplay.RegisterListener(Netplay::PACKET_LOGIC_ACTSWAP, LogicListener);
	netplay.RegisterListener(Netplay::PACKET_LOGIC_EXIT, LogicListener);
	netplay.RegisterListener(Netplay::PACKET_LOGIC_STAGECHG, LogicListener);
	netplay.RegisterListener(Netplay::PACKET_LOGIC_PAUSE, LogicListener);
	netplay.RegisterListener(Netplay::PACKET_LOGIC_RAND, LogicListener);

	AdvanceActLocal_h.Hook(AdvanceActLocal_r);
	SetChangeGameMode_h.Hook(SetChangeGameMode_r);
	ChangeStageWithFadeOut_h.Hook(ChangeStageWithFadeOut_r);
	RestartStageWithFadeOut_h.Hook(RestartStageWithFadeOut_r);
#endif
}
#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "netplay.h"
#include "timer.h"
#include "players.h"
#include "logic.h"
#include "config.h"

#ifdef MULTI_NETPLAY
static Timer logic_timer(std::chrono::steady_clock::duration(std::chrono::milliseconds(500)));

static int netGlobalMode;
static short netGameMode;
static short netStageNumber;
static int netActNumber;

static int oldTimerWake;
static int oldPauseEnabled;

FastFunctionHook<void, int> AdvanceActLocal_t(0x4146E0);
FastFunctionHook<void, Sint16> SetChangeGameMode_t(0x413C90);
FastFunctionHook<void, Sint8, Sint8> ChangeStageWithFadeOut_t(0x4145D0);
FastFunctionHook<void> RestartStageWithFadeOut_t(0x414600);

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
	AdvanceActLocal_t.Original(ssActAddition);
	netplay.Send(Netplay::PACKET_LOGIC_EXIT, LogicSender, -1, true);
}

static void SetChangeGameMode_r(Sint16 mode)
{
	if (netplay.IsConnected())
	{
		auto old = ssGameModeChange;
		SetChangeGameMode_t.Original(mode);

		if (ssGameModeChange != old)
		{
			netplay.Send(Netplay::PACKET_LOGIC_EXIT, LogicSender, -1, true);
		}
	}
	else
	{
		SetChangeGameMode_t.Original(mode);
	}
}

static void ChangeStageWithFadeOut_r(Sint8 stg, Sint8 act)
{
	ChangeStageWithFadeOut_t.Original(stg, act);
	netplay.Send(Netplay::PACKET_LOGIC_STAGECHG, LogicSender, -1, true);
}

static void RestartStageWithFadeOut_r()
{
	RestartStageWithFadeOut_t.Original();
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

void InitLogic()
{
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

	AdvanceActLocal_t.Hook(AdvanceActLocal_r);
	SetChangeGameMode_t.Hook(SetChangeGameMode_r);
	ChangeStageWithFadeOut_t.Hook(ChangeStageWithFadeOut_r);
	RestartStageWithFadeOut_t.Hook(RestartStageWithFadeOut_r);
#endif
}
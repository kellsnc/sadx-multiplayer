#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "network.h"
#include "timer.h"
#include "players.h"
#include "logic.h"

static Timer logic_timer(std::chrono::steady_clock::duration(std::chrono::milliseconds(500)));

static int netGlobalMode;
static short netGameMode;
static short netStageNumber;
static int netActNumber;

static int oldTimerWake;
static int oldPauseEnabled;

Trampoline* AdvanceActLocal_t = nullptr;
Trampoline* SetChangeGameMode_t = nullptr;
Trampoline* ChangeStageWithFadeOut_t = nullptr;
Trampoline* RestartStageWithFadeOut_t = nullptr;

static bool LogicListener(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum)
{
    switch (type)
    {
    case Network::PACKET_LOGIC_TIMER:
        packet >> GameTimer >> ulGlobalTimer >> gu32GameCnt >> gu32LocalCnt;
        return true;
    case Network::PACKET_LOGIC_CLOCK:
        packet >> bWake >> TimeMinutes >> TimeSeconds >> TimeFrames;
        return true;
    case Network::PACKET_LOGIC_MODE:
        packet >> netGlobalMode >> netGameMode >> netStageNumber >> netActNumber;

        if (netGameMode == MD_GAME_PAUSE && ssGameMode == MD_GAME_MAIN)
            ssGameMode = MD_GAME_PAUSE;
        else if (netGameMode == MD_GAME_MAIN && ssGameMode == MD_GAME_PAUSE)
            ssGameMode = MD_GAME_MAIN;
        else if ((netGameMode == MD_GAME_END || netGameMode == MD_GAME_END2) && (ssGameMode != MD_GAME_END || ssGameMode != MD_GAME_END2))
            ssGameMode = MD_GAME_END;

        return true;
    case Network::PACKET_LOGIC_LEVEL:
        packet >> ssStageNumber >> ssActNumber;
        return true;
    case Network::PACKET_LOGIC_ACTSWAP:
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
    case Network::PACKET_LOGIC_EXIT:
        packet >> ssGameModeChange;
        return true;
    case Network::PACKET_LOGIC_STAGECHG:
        packet >> ssGameModeChange >> ssNextStageNumber >> ssNextActNumber;
        return true;
    case Network::PACKET_LOGIC_PAUSE:
        packet >> PauseEnabled >> PauseSelection;
        return true;
    case Network::PACKET_LOGIC_RAND:
        packet >> seed;
        return true;
    default:
        return false;
    }
}

static bool LogicSender(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum)
{
    switch (type)
    {
    case Network::PACKET_LOGIC_TIMER:
        packet << GameTimer << ulGlobalTimer << gu32GameCnt << gu32LocalCnt;
        return true;
    case Network::PACKET_LOGIC_CLOCK:
        packet << bWake << TimeMinutes << TimeSeconds << TimeFrames;
        return true;
    case Network::PACKET_LOGIC_MODE:
        packet << ulGlobalMode << ssGameMode;
        return true;
    case Network::PACKET_LOGIC_LEVEL:
        packet << ssStageNumber << ssActNumber;
        return true;
    case Network::PACKET_LOGIC_ACTSWAP:
        packet << ssActNumber;
        return true;
    case Network::PACKET_LOGIC_EXIT:
        packet << ssGameModeChange;
        return true;
    case Network::PACKET_LOGIC_STAGECHG:
        packet << ssGameModeChange << ssNextStageNumber << ssNextActNumber;
        return true;
    case Network::PACKET_LOGIC_PAUSE:
        packet << PauseEnabled << PauseSelection;
        return true;
    case Network::PACKET_LOGIC_RAND:
        packet << seed;
        return true;
    default:
        return false;
    }
}

static void AdvanceActLocal_r(int ssActAddition)
{
    TARGET_DYNAMIC(AdvanceActLocal)(ssActAddition);
    network.Send(Network::PACKET_LOGIC_EXIT, LogicSender, -1, true);
}

static void SetChangeGameMode_r(__int16 mode)
{
    if (network.IsConnected())
    {
        auto old = ssGameModeChange;
        TARGET_DYNAMIC(SetChangeGameMode)(mode);

        if (ssGameModeChange != old)
        {
            network.Send(Network::PACKET_LOGIC_EXIT, LogicSender, -1, true);
        }
    }
    else
    {
        TARGET_DYNAMIC(SetChangeGameMode)(mode);
    }
}

static void ChangeStageWithFadeOut_r(int8_t stg, int8_t act)
{
    TARGET_DYNAMIC(ChangeStageWithFadeOut)(stg, act);
    network.Send(Network::PACKET_LOGIC_STAGECHG, LogicSender, -1, true);
}

static void RestartStageWithFadeOut_r()
{
    TARGET_DYNAMIC(RestartStageWithFadeOut)();
    network.Send(Network::PACKET_LOGIC_EXIT, LogicSender, -1, true);
}

extern "C"
{
    __declspec(dllexport) void __cdecl OnFrame()
    {
        if (network.IsConnected())
        {
            network.Poll();

            if (network.GetPlayerNum() == 0)
            {
                if (netGlobalMode != ulGlobalMode || netGameMode != ssGameMode)
                {
                    network.Send(Network::PACKET_LOGIC_MODE, LogicSender, -1, true);
                    netGlobalMode = ulGlobalMode;
                    netGameMode = ssGameMode;
                }

                if (netStageNumber != ssStageNumber || netActNumber != ssActNumber)
                {
                    network.Send(Network::PACKET_LOGIC_LEVEL, LogicSender, -1, true);
                    netStageNumber = ssStageNumber;
                    netActNumber = ssActNumber;
                }

                if (ssGameMode == 16 || PauseEnabled != oldPauseEnabled)
                {
                    network.Send(Network::PACKET_LOGIC_PAUSE, LogicSender);
                    oldPauseEnabled = PauseEnabled;
                }

                if (logic_timer.Finished())
                {
                    network.Send(Network::PACKET_LOGIC_MODE, LogicSender);
                    network.Send(Network::PACKET_LOGIC_TIMER, LogicSender);

                    if (oldTimerWake != bWake || bWake == TRUE)
                    {
                        network.Send(Network::PACKET_LOGIC_CLOCK, LogicSender);
                        oldTimerWake = bWake;
                    }
                }

                network.Send(Network::PACKET_LOGIC_RAND, LogicSender, -1, true);
            }
        }
        
        UpdatePlayersInfo();
    }
}

void InitLogic()
{
	network.RegisterListener(Network::PACKET_LOGIC_TIMER, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_CLOCK, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_MODE, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_LEVEL, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_ACTSWAP, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_EXIT, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_STAGECHG, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_PAUSE, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_RAND, LogicListener);

    AdvanceActLocal_t = new Trampoline(0x4146E0, 0x4146E5, AdvanceActLocal_r);
    SetChangeGameMode_t = new Trampoline(0x413C90, 0x413C96, SetChangeGameMode_r);
    ChangeStageWithFadeOut_t = new Trampoline(0x4145D0, 0x4145D6, ChangeStageWithFadeOut_r);
    RestartStageWithFadeOut_t = new Trampoline(0x414600, 0x414609, RestartStageWithFadeOut_r);
}
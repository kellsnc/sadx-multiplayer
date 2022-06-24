#include "pch.h"
#include "SADXModLoader.h"
#include "network.h"
#include "timer.h"
#include "players.h"
#include "logic.h"

static Timer logic_timer(std::chrono::steady_clock::duration(std::chrono::seconds(1)));

static int netGlobalMode;
static short netGameMode;
static short netStageNumber;
static int netActNumber;

static int oldTimerWake;

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
        return true;
    case Network::PACKET_LOGIC_LEVEL:
        packet >> ssStageNumber >> ssActNumber;
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
    default:
        return false;
    }
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
                    netGameMode == ssGameMode;
                }

                if (netStageNumber != ssStageNumber || netActNumber != ssActNumber)
                {
                    network.Send(Network::PACKET_LOGIC_LEVEL, LogicSender, -1, true);
                    netStageNumber = ssStageNumber;
                    netActNumber = netActNumber;
                }

                if (logic_timer.Finished())
                {
                    network.Send(Network::PACKET_LOGIC_TIMER, LogicSender);

                    if (oldTimerWake != bWake || bWake == TRUE)
                    {
                        network.Send(Network::PACKET_LOGIC_CLOCK, LogicSender);
                        oldTimerWake = bWake;
                    }
                }
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
}
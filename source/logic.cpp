#include "pch.h"
#include "SADXModLoader.h"
#include "network.h"
#include "players.h"
#include "logic.h"

static int netGlobalMode;
static short netGameMode;
static short netStageNumber;
static int netActNumber;

static bool LogicListener(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum)
{
    switch (type)
    {
    case Network::PACKET_LOGIC_TIMER:
        packet >> GameTimer >> ulGlobalTimer >> gu32GameCnt >> gu32LocalCnt;
        return true;
    case Network::PACKET_LOGIC_CLOCK:
        packet >> TimeThing >> TimeMinutes >> TimeSeconds >> TimeFrames;
        return true;
    case Network::PACKET_LOGIC_MODE:
        packet >> netGlobalMode >> netGameMode >> netStageNumber >> netActNumber;
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
        packet << TimeThing << TimeMinutes << TimeSeconds << TimeFrames;
        return true;
    case Network::PACKET_LOGIC_MODE:
        packet << ulGlobalMode << ssGameMode << ssStageNumber << ssActNumber;
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

            network.Send(Network::PACKET_LOGIC_TIMER, LogicSender);
            network.Send(Network::PACKET_LOGIC_CLOCK, LogicSender);
            network.Send(Network::PACKET_LOGIC_MODE, LogicSender);
        }
        
        UpdatePlayersInfo();
    }
}

void InitLogic()
{
	network.RegisterListener(Network::PACKET_LOGIC_TIMER, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_CLOCK, LogicListener);
	network.RegisterListener(Network::PACKET_LOGIC_MODE, LogicListener);
}
#pragma once

#include <vector>
#include "enet/enet.h"
#include "packet.h"

class Network
{
public:
	enum class Type
	{
		Client,
		Server
	};

	enum class Error
	{
		None,
		InitializationFailed,
		ServerCreationFailed,
		ClientCreationFailed,
		NoPeerAvailable,
		ConnectionFailed,
		AlreadyRunning
	};

	enum PACKET_TYPE : uint8_t
	{
		PACKET_CUSTOM, // for mods, keep as 0
		PACKET_PNUM,

		PACKET_INPUT_BUTTONS,
		PACKET_INPUT_STICK_X,
		PACKET_INPUT_STICK_Y,
		PACKET_INPUT_ANALOG,

		PACKET_PLAYER_LOCATION,
		PACKET_PLAYER_MODE,
		PACKET_PLAYER_SMODE,
		PACKET_PLAYER_FLAG,
		PACKET_PLAYER_ANIM,
		PACKET_PLAYER_RINGS,
		PACKET_PLAYER_SCORE,
		PACKET_PLAYER_LIVES,

		PACKET_LOGIC_TIMER,
		PACKET_LOGIC_CLOCK,
		PACKET_LOGIC_MODE
	};

	using PNUM = int8_t;
	using PACKET_CALL = bool(*)(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum);

	template<typename T>
	bool Send(PACKET_TYPE type, const T& data, bool reliable = false)
	{
		Packet packet = Packet(sizeof(type) + sizeof(PlayerNum) + sizeof(T));
		packet << type << PlayerNum << data;
		return packet.Send();
	}

	bool Send(PACKET_TYPE type, PACKET_CALL cb, PNUM player = -1, bool reliable = false);

	void RegisterListener(PACKET_TYPE type, PACKET_CALL);
	void Poll();

	int GetPlayerCount();
	int GetPlayerNum();
	bool IsPlayerConnected(int pnum);
	bool IsConnected();

	bool Create(Type type, const char* address, unsigned __int16 port);
	void Exit();

	Network();
	~Network();

private:
	enum PACKET_CHANNEL : enet_uint8
	{
		CHANNEL_RELIABLE,
		CHANNEL_UNRELIABLE,

		MAX_CHANNEL
	};

	struct PacketHeader
	{
		PACKET_TYPE identifier;
		PNUM player_num; // of sender, or additional identifier
	};

	Error last_error = Error::None;
	Type m_Type	     = Type::Client;
	bool connected   = false;
	PNUM PlayerNum = -1;
	PNUM PlayerCount = 0;

	void UpdatePeers();
	bool IsServer();
	void ReadPacket(ENetEvent&);

	std::vector<ENetPeer*> m_ConnectedPeers;
	std::vector<std::pair<PACKET_TYPE, PACKET_CALL>> listeners;

	ENetAddress m_Address;
	ENetHost* m_pHost = nullptr;
	ENetPeer* m_pPeer = nullptr;
};

extern Network network;

#pragma once

#ifdef NETWORK_BUILD
#include <vector>
#include "enet/enet.h"
#endif //NETWORK_BUILD

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

	void PollInputs();

	void Poll();
	bool Create(Type type, const char* address, unsigned __int16 port);
	void Exit();

	int GetPlayerCount();
	int GetPlayerNum();
	bool IsPlayerConnected(int pnum);
	bool IsConnected();

	Network();
	~Network();

#ifdef NETWORK_BUILD
	bool SendPacket(int channel, ENetPacket* packet);
#endif

private:
	enum PACKET_TYPE : uint8_t
	{
		PACKET_CUSTOM, // for mods, keep as 0
		PACKET_PNUM,
		PACKET_INPUT,
		PACKET_PLAYER,
		PACKET_GAME
	};

	enum PACKET_CHANNEL
	{
		GLOBAL_CHANNEL,
		INPUT_CHANNEL,

		MAX_CHANNEL
	};

	using PNUM = int8_t;

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

	// Update the player id of each peer
	void UpdatePeers();

	// If the current client is the server as well
	bool IsServer();

#ifdef NETWORK_BUILD
	void ReadPacket(Packet&);

	template <typename T>
	bool SendDataT(PACKET_TYPE packet_type, PACKET_CHANNEL channel, enet_uint32 flag, T& data)
	{
		Packet packet = Packet(sizeof(packet_type) + sizeof(PlayerNum) + sizeof(T));
		packet << packet_type << PlayerNum << data;
		return packet.Send();
	}

	std::vector<ENetPeer*> m_ConnectedPeers;
	ENetAddress m_Address;
	ENetHost* m_pHost = nullptr;
	ENetPeer* m_pPeer = nullptr;
#endif //NETWORK_BUILD
};

extern Network network;

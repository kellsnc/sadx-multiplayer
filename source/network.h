#pragma once

#ifdef NETWORK_BUILD
#include <vector>
#include "enet/enet.h"
#endif //NETWORK_BUILD

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
	void SendPacket(PACKET_CHANNEL channel, ENetPacket* packet);
	void ReadPacket(ENetPacket*);

	template <typename T>
	bool packet_write(ENetPacket* packet, size_t& pos, const T& data)
	{
		auto end_pos = pos + sizeof(T);
		if (end_pos <= packet->dataLength)
		{
			*(T*)(packet->data + pos) = data;
			pos = end_pos;
			return true;
		}
		return false;
	}

	template <typename T>
	bool packet_read(ENetPacket* packet, size_t& pos, T& output)
	{
		auto end_pos = pos + sizeof(T);
		if (end_pos <= packet->dataLength)
		{
			output = *(T*)(packet->data + pos);
			pos = end_pos;
			return true;
		}
		return false;
	}

	template <typename T>
	bool SendDataT(PACKET_TYPE packet_type, PACKET_CHANNEL channel, enet_uint32 flag, T& data)
	{
		PacketHeader header = { packet_type, PlayerNum };

		auto packet = enet_packet_create(NULL, sizeof(header) + sizeof(T), flag);

		if (packet == NULL)
		{
			return false;
		}

		size_t position = 0;
		if (!packet_write(packet, position, header)) return false;
		if (!packet_write(packet, position, data)) return false;

		SendPacket(channel, packet);
		return true;
	}

	std::vector<ENetPeer*> m_ConnectedPeers;
	ENetAddress m_Address;
	ENetHost* m_pHost = nullptr;
	ENetPeer* m_pPeer = nullptr;
#endif //NETWORK_BUILD
};

extern Network network;

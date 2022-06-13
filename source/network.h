#pragma once

#ifdef NETWORK_BUILD
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
		InitializationFailed,
		ServerCreationFailed,
		ClientCreationFailed,
		NoPeerAvailable,
		AlreadyRunning
	};

	void Poll();
	bool Create(Type type, const char* address, unsigned __int16 port);
	void Exit();

	Network();
	~Network();

private:
	Error last_error;
	Type m_Type;

	bool IsServer();

#ifdef NETWORK_BUILD
	ENetAddress m_Address;
	ENetHost* m_pHost;
	ENetPeer* m_pPeer;
#endif //NETWORK_BUILD
};

extern Network network;

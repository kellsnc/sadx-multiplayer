#include "pch.h"
#include "SADXFunctions.h"

#define NETWORK_BUILD
#include "network.h"

#define PRINT(text, ...) PrintDebug("[Multi] " text "\n", __VA_ARGS__)

bool Network::IsServer()
{
	return m_Type == Type::Server;
}

void Network::Poll()
{
	if (!m_pHost)
	{
		return;
	}

	ENetEvent event;
	while (enet_host_service(m_pHost, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			event.peer->data = NULL;
			break;
		}
	}
}

bool Network::Create(Type type, const char* ip, unsigned __int16 port)
{
	if (m_pHost)
	{
		last_error = Error::AlreadyRunning;
		return false;
	}

	m_Type = type;

	m_Address.host = ENET_HOST_ANY;
	m_Address.port = port;

	if (ip)
	{
		enet_address_set_host(&m_Address, ip);
	}
	else
	{
		enet_address_set_host(&m_Address, "localhost");
	}

	if (type == Type::Server)
	{
		m_pHost = enet_host_create(&m_Address, 4, 2, 0, 0);

		if (m_pHost == nullptr)
		{
			last_error = Error::ServerCreationFailed;
			PRINT("An error occurred while trying to create an ENet server host.");
			return false;
		}

		PRINT("Created server: %s:%d", ip == nullptr ? "localhost" : ip, port);
	}
	else
	{
		m_pHost = enet_host_create(NULL, 1, 2, 0, 0);

		if (m_pHost == nullptr)
		{
			last_error = Error::ClientCreationFailed;
			PRINT("An error occured while trying to create an ENet server host");
			return false;
		}

		m_pPeer = enet_host_connect(m_pHost, &m_Address, 2, 0);

		if (m_pPeer == nullptr)
		{
			last_error = Error::NoPeerAvailable;
			PRINT("No available peers for initializing an ENet connection");
			return false;
		}

		ENetEvent event;

		if (enet_host_service(m_pHost, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
		{
			PRINT("Connected to %s:%d", ip == nullptr ? "localhost" : ip, port);
		}
		else
		{
			return false;
		}
	}

	return true;
}

void Network::Exit()
{
	if (m_pHost)
	{
		enet_host_destroy(m_pHost);
		m_pHost = nullptr;
	}
}

Network::Network()
{
	if (enet_initialize() != 0)
	{
		last_error = Error::InitializationFailed;
		PRINT("An error occurred while initializing ENet");
	}
}

Network::~Network()
{
	Exit();
	enet_deinitialize();
}

Network network;

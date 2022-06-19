#include "pch.h"
#include "SADXFunctions.h"
#include "players.h"
#include "splitscreen.h"
#include "network.h"

// Networking system
// Listener system originally by @michael-fadely

#define PRINT(text, ...) PrintDebug("[Multi] " text "\n", __VA_ARGS__)

bool Network::SendPacket(ENetPacket* packet, bool reliable)
{
	auto channel = reliable ? CHANNEL_RELIABLE : CHANNEL_UNRELIABLE;
	if (IsServer())
	{
		enet_host_broadcast(m_pHost, channel, packet);
		return true;
	}
	else
	{
		return !enet_peer_send(m_pPeer, channel, packet);
	}
}

bool Network::SendPacket(ENetPeer* peer, ENetPacket* packet, bool reliable)
{
	return !enet_peer_send(peer, reliable ? CHANNEL_RELIABLE : CHANNEL_UNRELIABLE, packet);
}

bool Network::Send(PACKET_TYPE type, PACKET_CALL cb, bool reliable)
{
	if (IsConnected())
	{
		Packet packet = Packet(sizeof(type) + sizeof(PlayerNum));
		packet << type << PlayerNum;
		cb(packet, type, PlayerNum);
		return packet.Send();
	}
}

void Network::RegisterListener(PACKET_TYPE type, PACKET_CALL cb)
{
	listeners.push_back({ type, cb });
}

int Network::GetPlayerCount()
{
	return IsConnected() ? PlayerCount : 0;
}

int Network::GetPlayerNum()
{
	return PlayerNum;
}

bool Network::IsPlayerConnected(int pnum)
{
	return pnum < GetPlayerCount();
}

bool Network::IsConnected()
{
	return connected;
}

bool Network::IsServer()
{
	return m_Type == Type::Server;
}

void Network::ReadPacket(Packet& packet)
{
	size_t position = 0;
	PACKET_TYPE header;
	PNUM pnum;

	packet >> header;
	packet >> pnum;

	if (header == PACKET_PNUM)
	{
		PlayerNum = pnum;
		packet >> PlayerCount;
		return;
	}
	
	for (auto& i : listeners)
	{
		if (header == i.first)
		{
			if (i.second(packet, header, pnum))
			{
				return;
			}
		}
	}
}

void Network::UpdatePeers()
{
	PlayerCount = static_cast<PNUM>(m_ConnectedPeers.size() + 1);

	for (size_t i = 0; i < m_ConnectedPeers.size(); ++i)
	{
		Packet pnum_packet = Packet(sizeof(PACKET_TYPE) + sizeof(PNUM) + sizeof(PNUM), true);
		pnum_packet << PACKET_PNUM << static_cast<PNUM>(i + 1) << PlayerCount;
		pnum_packet.Send(m_ConnectedPeers[i]);
		m_ConnectedPeers[i]->data = reinterpret_cast<void*>(i + 1);
	}
}

void Network::Poll()
{
	if (!IsConnected())
	{
		return;
	}

	ENetEvent event;
	while (enet_host_service(m_pHost, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			if (IsServer())
			{
				m_ConnectedPeers.push_back(event.peer);
				UpdatePeers();
			}
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			{
				Packet packet = Packet(event.packet);
				ReadPacket(packet);
				break;
			}
		case ENET_EVENT_TYPE_DISCONNECT:
			if (IsServer())
			{
				m_ConnectedPeers.erase(std::remove(m_ConnectedPeers.begin(), m_ConnectedPeers.end(), event.peer), m_ConnectedPeers.end());
				event.peer->data = NULL;
				UpdatePeers();
			}

			PlayerNum = -1;
			PlayerCount = 0;
			connected = false;
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
	PlayerNum = -1;
	PlayerCount = 0;

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
		m_pHost = enet_host_create(&m_Address, PLAYER_MAX, MAX_CHANNEL, 0, 0);

		if (m_pHost == nullptr)
		{
			last_error = Error::ServerCreationFailed;
			PRINT("An error occurred while trying to create an ENet server host.");
			return false;
		}

		PlayerCount = 1;
		PlayerNum = 0;
		PRINT("Created server: %s:%d", ip == nullptr ? "localhost" : ip, port);
	}
	else
	{
		m_pHost = enet_host_create(NULL, 1, MAX_CHANNEL, 0, 0);

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
			Exit();
			return false;
		}

		ENetEvent event;

		if (enet_host_service(m_pHost, &event, 300) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
		{
			PRINT("Connected to %s:%d", ip == nullptr ? "localhost" : ip, port);
		}
		else
		{
			enet_peer_reset(m_pPeer);
			m_pPeer = nullptr;
			last_error = Error::ConnectionFailed;
			Exit();
			return false;
		}
	}

	return connected = true;
}

void Network::Exit()
{
	if (!IsServer())
	{
		if (m_pPeer)
		{
			ENetEvent event;
			enet_peer_disconnect(m_pPeer, 0);

			bool succeeded = false;

			while (!succeeded && enet_host_service(m_pHost, &event, 1000) > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_RECEIVE:
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					succeeded = true;
					PRINT("Disconnection succeeded.");
					break;
				}
			}

			if (!succeeded)
			{
				enet_peer_reset(m_pPeer);
			}

			m_pPeer = nullptr;
		}
	}
	else
	{
		// Disconnected all connected peers
		for (auto& p : m_ConnectedPeers)
		{
			enet_peer_disconnect(p, 0);
			enet_host_flush(p->host);
		}

		m_ConnectedPeers.clear();
	}

	if (m_pHost)
	{
		enet_host_destroy(m_pHost);
		m_pHost = nullptr;
	}

	PlayerNum = -1;
	PlayerCount = 0;
	connected = false;
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

#include "pch.h"
#include "SADXFunctions.h"
#include "netplay.h"

#ifdef MULTI_NETPLAY
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"enet.lib")
#endif

// Networking system
// Listener system originally by @michael-fadely

#define PRINT(text, ...) PrintDebug("[Multi] " text "\n", __VA_ARGS__)

bool Netplay::Send(PACKET_TYPE type, PACKET_CALL cb, PNUM player, bool reliable)
{
#ifdef MULTI_NETPLAY
	if (IsConnected())
	{
		Packet packet = Packet(type, player, reliable);

		if (!cb(packet, type, PlayerNum))
		{
			packet.Destroy();
			return false;
		}

		if (IsServer())
		{
			for (auto& p : m_ConnectedPeers)
			{
				auto peer_pnum = reinterpret_cast<int>(p->data);
				if (player == -1 || player == peer_pnum)
				{
					packet.Send(p);
				}
			}
			return true;
		}
		else
		{
			return packet.Send(m_pPeer);
		}
	}
#endif
	return false;
}

void Netplay::RegisterListener(PACKET_TYPE type, PACKET_CALL cb)
{
#ifdef MULTI_NETPLAY
	listeners.push_back({ type, cb });
#endif
}

int Netplay::GetPlayerCount()
{
	return IsConnected() ? PlayerCount : 0;
}

int Netplay::GetPlayerNum()
{
	return PlayerNum;
}

bool Netplay::IsPlayerConnected(int pnum)
{
	return pnum < GetPlayerCount();
}

bool Netplay::IsConnected()
{
#ifdef MULTI_NETPLAY
	return connected;
#else
	return false;
#endif
}

bool Netplay::IsServer()
{
	return m_Type == Type::Server;
}

void Netplay::ReadPacket(ENetEvent& event)
{
#ifdef MULTI_NETPLAY
	Packet packet = Packet(event.packet);

	auto header = packet.GetIndentifier();
	auto sender = packet.GetSender();
	auto receiver = packet.GetReceiver();

	// If we're the server, bounce packet to other peers
	if (IsServer() && receiver != PlayerNum)
	{
		for (auto& p : m_ConnectedPeers)
		{
			auto peer_pnum = reinterpret_cast<int>(p->data);

			// Do not send back to sender
			if (sender != peer_pnum)
			{
				if (receiver == -1 || receiver == peer_pnum)
				{
					packet.Send(p);
				}
			}
		}
	}

	// Read packet if it's for us
	if (receiver == -1 || receiver == PlayerNum)
	{
		for (auto& i : listeners)
		{
			if (header == i.first)
			{
				if (i.second(packet, (PACKET_TYPE)header, sender))
				{
					packet.Destroy();
					return;
				}
			}
		}
	}
#endif
}

void Netplay::UpdatePeers()
{
#ifdef MULTI_NETPLAY
	PlayerCount = static_cast<PNUM>(m_ConnectedPeers.size() + 1);

	for (size_t i = 0; i < m_ConnectedPeers.size(); ++i)
	{
		Packet pnum_packet = Packet(PACKET_PNUM, static_cast<PNUM>(i + 1), true);
		pnum_packet << PlayerCount << Version;
		pnum_packet.Send(m_ConnectedPeers[i]);
		m_ConnectedPeers[i]->data = reinterpret_cast<void*>(i + 1);
	}
#endif
}

bool Netplay::PollMessage(PACKET_TYPE type, Packet& packet)
{
#ifdef MULTI_NETPLAY
	ENetEvent event;
	while (enet_host_service(m_pHost, &event, 1) > 0)
	{
		if (event.type == ENET_EVENT_TYPE_RECEIVE)
		{
			auto packet_ = Packet(event.packet);
			auto receiver = packet_.GetReceiver();

			if (packet_.GetIndentifier() == type && (PlayerNum == -1 || receiver == -1 || receiver == PlayerNum))
			{
				packet = packet_;
				return true;
			}
		}
	}
#endif
	return false;
}

void Netplay::Poll()
{
#ifdef MULTI_NETPLAY
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
			ReadPacket(event);
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
#endif
}

bool Netplay::Create(Type type, const char* ip, unsigned __int16 port)
{
#ifdef MULTI_NETPLAY
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
		return connected = true;
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
			for (int i = 0; i < 120; ++i)
			{
				Packet packet;
				if (PollMessage(PACKET_PNUM, packet))
				{
					int32_t incomingver;
					PlayerNum = packet.GetReceiver();
					packet >> PlayerCount >> incomingver;
					packet.Destroy();

					if (incomingver != Version)
					{
						last_error = Error::VersionMismatch;
						PRINT("Server/client version mismatch");
						Exit();
						return false;
					}

					PRINT("Connected to %s:%d", ip == nullptr ? "localhost" : ip, port);
					return connected = true;
				}
			}
		}
		else
		{
			enet_peer_reset(m_pPeer);
			m_pPeer = nullptr;
			last_error = Error::ConnectionFailed;
		}

		last_error = Error::TimedOut;
		PRINT("Timed out");
		Exit();
	}
#endif
	return false;
}

void Netplay::Exit()
{
#ifdef MULTI_NETPLAY
	if (netplay.IsConnected())
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
#endif
}

Netplay::Netplay()
{
#ifdef MULTI_NETPLAY
	if (enet_initialize() != 0)
	{
		last_error = Error::InitializationFailed;
		PRINT("An error occurred while initializing ENet");
	}
#endif
}

Netplay::~Netplay()
{
#ifdef MULTI_NETPLAY
	Exit();
	enet_deinitialize();
#endif
}

Netplay netplay;
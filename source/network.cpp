#include "pch.h"
#include "SADXFunctions.h"
#include "players.h"
#include "splitscreen.h"

#define NETWORK_BUILD
#include "network.h"

#define PRINT(text, ...) PrintDebug("[Multi] " text "\n", __VA_ARGS__)

struct GameData
{
	int8_t TimerMinutes, TimerSeconds, TimerFrames;
	GameModes GameMode;
	int16_t GameState;
	int16_t Level;
	int32_t Act;
};

struct PlayerData
{
	NJS_POINT3 Position;
	Angle3 Angle;
	int8_t Mode, SMode;
	int16_t Timer, Flag, Equipement, Item;
	NJS_POINT3 Speed;
	int16_t Rings, Lives;
	int32_t Score;
	int32_t AnimID;
	float AnimFrame;
};

struct InputData
{
	PolarCoord Analog;
	ControllerData Controller;
};

static InputData netInputData[PLAYER_MAX];

void Network::SendPacket(PACKET_CHANNEL channel, ENetPacket* packet)
{
	if (IsServer())
	{
		enet_host_broadcast(m_pHost, channel, packet);
	}
	else
	{
		enet_peer_send(m_pPeer, channel, packet);
	}
}

void Network::PollInputs()
{
	if (!IsConnected())
	{
		return;
	}

	if (PlayerNum > -1)
	{
		InputData data;
		data.Analog = NormalizedAnalogs[PlayerNum];
		data.Controller = Controllers[PlayerNum];
		SendDataT(PACKET_INPUT, INPUT_CHANNEL, ENET_PACKET_FLAG_RELIABLE, data);
	}
	
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (i != PlayerNum)
		{
			auto& data = netInputData[i];
			NormalizedAnalogs[i] = data.Analog;
			*ControllerPointers[i] = data.Controller;
		}
	}
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

void Network::ReadPacket(ENetPacket* packet)
{
	size_t position = 0;
	PACKET_TYPE header;
	PNUM pnum;

	packet_read(packet, position, header);
	packet_read(packet, position, pnum);

	switch (header)
	{
	case PACKET_PNUM:
		PlayerNum = pnum;
		packet_read(packet, position, PlayerCount);
		break;
	case PACKET_INPUT:
		packet_read(packet, position, netInputData[pnum].Analog);
		packet_read(packet, position, netInputData[pnum].Controller);
		break;
	case PACKET_PLAYER:
	{
		auto twp = playertwp[pnum];
		auto pwp = playerpwp[pnum];

		if (twp && pwp)
		{
			PlayerData data;
			packet_read(packet, position, data);

			twp->pos = data.Position;
			twp->ang = data.Angle;
			twp->mode = data.Mode;
			twp->smode = data.SMode;
			twp->wtimer = data.Timer;
			twp->flag = data.Flag;
			pwp->equipment = data.Equipement;
			pwp->item = data.Item;
			pwp->spd = data.Speed;
			pwp->mj.reqaction = pwp->mj.action = data.AnimID;
			pwp->mj.nframe = data.AnimFrame;
			SetNumRingM(pnum, data.Rings);
			SetNumPlayerM(pnum, data.Lives);
			SetEnemyScoreM(pnum, data.Score);
		}
		break;
	}
	case PACKET_GAME:
	{
		GameData data;
		packet_read(packet, position, data);

		if (pnum == 0)
		{
			TimeMinutes = data.TimerMinutes;
			TimeSeconds = data.TimerSeconds;
			TimeFrames = data.TimerFrames;
		}

		break;
	}
	}

	enet_packet_destroy(packet);
}

void Network::UpdatePeers()
{
	PlayerCount = static_cast<PNUM>(m_ConnectedPeers.size() + 1);

	for (size_t i = 0; i < m_ConnectedPeers.size(); ++i)
	{
		auto packet = enet_packet_create(NULL, sizeof(PACKET_TYPE) + sizeof(PNUM) + sizeof(PNUM), ENET_PACKET_FLAG_RELIABLE);
		size_t position = 0;
		packet_write(packet, position, PACKET_PNUM);
		packet_write(packet, position, static_cast<PNUM>(i + 1));
		packet_write(packet, position, PlayerCount);
		enet_peer_send(m_ConnectedPeers[i], GLOBAL_CHANNEL, packet);
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
			ReadPacket(event.packet);
			break;
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

	if (GameTimer % 3 != 0 || PlayerNum < 0)
	{
		return;
	}

	auto twp = playertwp[PlayerNum];
	auto pwp = playerpwp[PlayerNum];
	if (twp && pwp)
	{
		PlayerData data;
		data.Position = twp->pos;
		data.Angle = twp->ang;
		data.Mode = twp->mode;
		data.SMode = twp->smode;
		data.Flag = twp->flag;
		data.Timer = twp->wtimer;
		data.Equipement = pwp->equipment;
		data.Item = pwp->item;
		data.AnimFrame = pwp->mj.nframe;
		data.AnimID = pwp->mj.jvmode == 2 ? pwp->mj.action : pwp->mj.reqaction;
		data.Speed = pwp->spd;
		data.Rings = GetNumRingM(PlayerNum);
		data.Lives = GetNumPlayerM(PlayerNum);
		data.Score = GetEnemyScoreM(PlayerNum);
		SendDataT(PACKET_PLAYER, GLOBAL_CHANNEL, 0, data);
	}

	GameData data;
	data.TimerMinutes = TimeMinutes;
	data.TimerSeconds = TimeSeconds;
	data.TimerFrames = TimeFrames;
	data.GameMode = GameMode;
	data.GameState = GameState;
	data.Level = CurrentLevel;
	data.Act = CurrentAct;
	SendDataT(PACKET_GAME, GLOBAL_CHANNEL, 0, data);
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

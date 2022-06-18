#include "pch.h"

#define NETWORK_BUILD
#include "network.h"
#include "packet.h"

bool Packet::Send()
{
	return sent = network.SendPacket(m_reliable ? 1 : 0, m_packet);
}

bool Packet::Send(ENetPeer* peer)
{
	return sent = !enet_peer_send(peer, m_reliable ? 1 : 0, m_packet);
}

Packet::Packet(size_t size, bool reliable)
{
	m_packet = enet_packet_create(NULL, size, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
	m_reliable = reliable;
}

Packet::Packet(ENetPacket* packet)
{
	m_packet = packet;
}

Packet::~Packet()
{
	if (m_packet && !sent)
	{
		enet_packet_destroy(m_packet);
	}
}

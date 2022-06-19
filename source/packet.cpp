#include "pch.h"
#include "network.h"
#include "packet.h"

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

bool Packet::Send()
{
	return sent = network.SendPacket(m_packet, m_reliable);
}

bool Packet::Send(ENetPeer* peer)
{
	return sent = network.SendPacket(peer, m_packet, m_reliable);
}

size_t Packet::size()
{
	return m_packet->dataLength;
}

uint8_t* Packet::at(size_t position)
{
	if (position < m_packet->dataLength)
	{
		return (BYTE*)(m_packet->data + position);
	}
	else
	{
		return nullptr;
	}
}

uint8_t* Packet::operator[](size_t position)
{
	return at(position);
}

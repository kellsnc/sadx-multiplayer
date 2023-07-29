#include "pch.h"
#include "netplay.h"
#include "packet.h"

Packet::Packet(uint8_t identifier, int8_t to, bool reliable)
{
	m_packet = enet_packet_create(NULL, sizeof(identifier) + sizeof(to), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
	m_reliable = reliable;
	sender = netplay.GetPlayerNum();
	recipient = to;

	int8_t address = (((((uint8_t)(sender)) << 4) & 0xF0) | ((uint8_t)(recipient + 1)) & 0x0F);
	*this << address << identifier;
}

Packet::Packet(ENetPacket* packet)
{
	m_packet = packet;
	m_reliable = packet->flags & ENET_PACKET_FLAG_RELIABLE;

	int8_t address;
	*this >> address >> m_identifier;
	sender = ((address & 0xF0) >> 4);
	recipient = (address & 0x0F) - 1;
}

void Packet::Destroy()
{
	if (m_packet && !sent)
	{
		enet_packet_destroy(m_packet);
	}
}

uint8_t Packet::GetIndentifier()
{
	return m_identifier;
}

int8_t Packet::GetSender()
{
	return sender;
}

int8_t Packet::GetReceiver()
{
	return recipient;
}

bool Packet::Send(ENetPeer* peer)
{
	return sent = !enet_peer_send(peer, m_reliable ? 1 : 0, m_packet);
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
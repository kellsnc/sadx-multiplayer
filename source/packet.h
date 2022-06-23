#pragma once

#include "enet/enet.h"

class Packet
{
private:
	size_t position = 0;
	bool sent = false;
	bool m_reliable = false;
	ENetPacket* m_packet = nullptr;

	int8_t sender;
	int8_t recipient;
	uint8_t m_identifier;

public:
	Packet() {};
	Packet(uint8_t identifier, int8_t to = -1, bool reliable = false);
	Packet(ENetPacket* packet);
	
	void Destroy();

	uint8_t GetIndentifier();
	int8_t GetSender();
	int8_t GetReceiver();

	bool Send(ENetPeer* peer);

	size_t size();

	uint8_t* at(size_t);
	uint8_t* operator[](size_t);

	template<typename T>
	Packet& operator<<(const T& data)
	{
		auto end_pos = position + sizeof(T);

		if (end_pos > m_packet->dataLength)
		{
			if (enet_packet_resize(m_packet, end_pos) < 0)
			{
				return *this;
			}
		}

		*(T*)(m_packet->data + position) = data;
		position = end_pos;

		return *this;
	}

	template<typename T>
	Packet& operator>>(T& data)
	{
		auto end_pos = position + sizeof(T);

		if (end_pos <= m_packet->dataLength)
		{
			data = *(T*)(m_packet->data + position);
			position = end_pos;
		}
		else
		{
			data = {};
		}

		return *this;
	}
};

#pragma once

#ifdef NETWORK_BUILD
#include "enet/enet.h"
#endif //NETWORK_BUILD

class Packet
{
private:
	size_t position = 0;
	bool sent = false;
	bool m_reliable = false;

#ifdef NETWORK_BUILD
	ENetPacket* m_packet = nullptr;
#endif

public:
	Packet(size_t size, bool reliable = false);
	~Packet();

	bool Send();

#ifdef NETWORK_BUILD
	Packet(ENetPacket* packet);

	bool Send(ENetPeer* peer);

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
#endif
};

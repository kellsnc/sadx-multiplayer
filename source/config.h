#pragma once

namespace config
{
	extern bool splitScreenEnabled;
	extern bool indicatorEnabled;
	extern bool infiniteLives;

	namespace network
	{
		extern std::string default_ip;
		extern int default_port;
	}

	void read(const char* path);

}

void SetInfiniteLives();
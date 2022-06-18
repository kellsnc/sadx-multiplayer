#pragma once

namespace config
{
	extern bool splitScreenEnabled;
	extern bool indicatorEnabled;

	namespace network
	{
		extern std::string default_ip;
		extern int default_port;
	}

	void read(const char* path);
}
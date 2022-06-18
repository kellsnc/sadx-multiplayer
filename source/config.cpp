#include "pch.h"
#include <IniFile.hpp>
#include "config.h"

namespace config
{
	bool splitScreenEnabled = true;
	bool indicatorEnabled = false;

	namespace network
	{
		std::string default_ip("localhost");
		int default_port = 27015;
	}

	void read(const char* path)
	{
		const IniFile* config = new IniFile(std::string(path) + "\\config.ini");
		const IniGroup* general = config->getGroup("");

		if (general)
		{
			splitScreenEnabled = general->getBool("SplitScreen", splitScreenEnabled);
			indicatorEnabled   = general->getBool("Indicator", indicatorEnabled);
		}

		const IniGroup* network = config->getGroup("Networking");

		if (network)
		{
			network::default_ip = network->getString("DefaultAddress", network::default_ip);
			network::default_port = network->getInt("DefaultPort", network::default_port);
		}

		delete config;
	}
}
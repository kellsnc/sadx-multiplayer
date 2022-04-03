#include "pch.h"
#include <IniFile.hpp>
#include "config.h"

namespace config
{
	bool splitScreenEnabled = true;
	bool indicatorEnabled = false;

	void read(const char* path)
	{
		const IniFile* config = new IniFile(std::string(path) + "\\config.ini");
		const IniGroup* general = config->getGroup("");

		if (general)
		{
			splitScreenEnabled = general->getBool("SplitScreen", splitScreenEnabled);
			indicatorEnabled   = general->getBool("Indicator", indicatorEnabled);
		}

		delete config;
	}
}
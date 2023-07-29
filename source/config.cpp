#include "pch.h"
#include <memory>
#include <IniFile.hpp>
#include "config.h"

Config config;

void Config::read(const char* path)
{
	std::unique_ptr<IniFile> config(new IniFile(std::string(path) + "\\config.ini"));
	const IniGroup* ini_general = config->getGroup("");

	if (ini_general)
	{
		mSplitScreen = ini_general->getBool("SplitScreen", mSplitScreen);
		mIndicator = ini_general->getBool("Indicator", mIndicator);
	}

	const IniGroup* ini_netplay = config->getGroup("Netplay");

	if (ini_netplay)
	{
		netplay.mDefaultAddress = ini_netplay->getString("DefaultAddress", netplay.mDefaultAddress);
		netplay.mDefaultPort = ini_netplay->getInt("DefaultPort", netplay.mDefaultPort);
	}

	const IniGroup* ini_cheats = config->getGroup("Cheats");

	if (ini_cheats)
	{
#define READ_CHEAT(NAME) cheats.m##NAME##[0] = ini_cheats->getBool(###NAME "P1", false); \
		cheats.m##NAME##[1] = ini_cheats->getBool(###NAME "P2", false); \
		cheats.m##NAME##[2] = ini_cheats->getBool(###NAME "P3", false); \
		cheats.m##NAME##[3] = ini_cheats->getBool(###NAME "P4", false);

		READ_CHEAT(InfiniteLives);
		READ_CHEAT(InfiniteRings);
	}
}

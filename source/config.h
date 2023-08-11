#pragma once

struct Config
{
	bool mSplitScreen = true;
	bool mHorizontalLayout = false;
	bool mIndicator = false;

	struct
	{
		std::string mDefaultAddress = "localhost";
		int mDefaultPort = 27015;
	} netplay;

	struct
	{
		bool mInfiniteLives[PLAYER_MAX] = {};
		bool mInfiniteRings[PLAYER_MAX] = {};
	} cheats;

	void read(const char* path);
};

extern Config config;
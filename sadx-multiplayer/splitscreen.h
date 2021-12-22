#pragma once


namespace SplitScreen
{
	struct ScreenRatio
	{
		float x, y, w, h;
	};

	extern unsigned int numScreen;
	extern signed int numViewPort;

	const ScreenRatio* GetScreenRatio(int num);
	bool ChangeViewPort(int num);
}

void InitSplitScreen();
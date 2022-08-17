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
	void SaveViewPort();
	void RestoreViewPort();
	bool IsActive();
	unsigned int GetCurrentScreenNum();
	bool IsScreenEnabled(int num);
	bool ChangeViewPort(int num);
	bool IsVisible(int num, NJS_POINT3* pos, Float radius);
	bool IsVisible(NJS_POINT3* pos, Float radius);
}

void InitSplitScreen();
#pragma once

namespace splitscreen
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
	signed int GetCurrentViewPortNum();
	bool IsActive();
	bool IsEnabled();
	void Enable();
	void Disable();
	unsigned int GetCurrentScreenNum();
	bool IsScreenEnabled(int num);
	bool ChangeViewPort(int num);
}

void InitSplitScreen();
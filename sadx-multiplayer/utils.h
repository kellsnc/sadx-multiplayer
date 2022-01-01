#pragma once

void DrawSADXText(const char* text, __int16 y);
int MenuSelectButtonsPressed_r(int pnum);
int MenuBackButtonsPressed_r(int pnum);
short ConvertLevelActsID_ToLevel(short level);
short ConvertLevelActsID_ToAct(short act);

enum Align : Uint8
{
	Align_Default = 0,
	Align_Left = 1 << 1,
	Align_Right = 1 << 2,
	Align_Center_Horizontal = Align_Left | Align_Right,

	Align_Automatic_Horizontal = 1 << 3,
	Align_Top = 1 << 4,
	Align_Bottom = 1 << 5,
	Align_Center_Vertical = Align_Top | Align_Bottom,
	Align_Automatic_Vertical = 1 << 6,

	Align_Center = Align_Center_Horizontal | Align_Center_Vertical,
	Align_Automatic = Align_Automatic_Horizontal | Align_Automatic_Vertical
};
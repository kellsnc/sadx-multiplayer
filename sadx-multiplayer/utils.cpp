#include "pch.h"

void DrawSADXText(const char* text, __int16 y)
{
	sub_40BC80();
	SetMessageSettings(&stru_3ABDC18, 0, y, 0, 0, 0xFF000020);
	unk_3C49C23 = 0x80;
	DoSomethingRelatedToText_(text);
	MSG_LoadTexture2(&stru_3ABDC18);
}

int MenuSelectButtonsPressed_r(int pnum)
{
	return PressedButtons[pnum] & (Buttons_Start | Buttons_A);

}

int MenuBackButtonsPressed_r(int pnum)
{
	return PressedButtons[pnum] & (Buttons_X | Buttons_B);
}

short ConvertLevelActsID_ToLevel(short level) {

	return level >> 8;
}

short ConvertLevelActsID_ToAct(short act) {

	return act & 0xf;
}

#include "pch.h"

void DrawSADXText(const char* text, __int16 x, __int16 y, int width, int height) {

	if (!text)
		return;

	sub_40BC80();
	SetMessageSettings(&stru_3ABDC18, x, y, width, height, 0xFF000020);
	unk_3C49C23 = 0x80;
	DoSomethingRelatedToText_(text);
	MSG_LoadTexture2(&stru_3ABDC18);

	//MSG_LoadTexture2(subthing);
}

__int32 MenuSelectButtonsPressed_r(char pnum)
{
	return PressedButtons[pnum] & (Buttons_Start | Buttons_A);

}

__int32 MenuBackButtonsPressed_r(char pnum)
{
	return PressedButtons[pnum] & (Buttons_X | Buttons_B);
}
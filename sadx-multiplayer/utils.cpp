#include "pch.h"

void DrawSADXText(const char* text, __int16 y)
{
	MSG_Close(&jimakumsgc);
	DialogJimakuInit();
	NH_MSG_Open(&jimakumsgc, 0, y, 0, 0, 0xFF000020, jimakubuf);
	DoSomethingRelatedToText_(text);
	MSG_LoadTexture(&jimakumsgc);
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

#include "pch.h"
#include "splitscreen.h"

TaskFunc(ObjectBeachWaveBG1Display, 0x501130); // unnamed in symbols
TaskFunc(ObjectBeachWaveBG2Display, 0x4F76C0); // unnamed in symbols
TaskFunc(ObjectBeachWaveBG3Display, 0x4F7760); // unnamed in symbols

void __cdecl ObjectBeachWaveBGExec_multi(task* tp)
{
	auto twp = tp->twp;

	NJS_VECTOR plpos;
	GetPlayerPosition(twp->id, 0, &plpos, 0);
	twp->pos.x = floorf((plpos.x - twp->pos.x) * 0.04f) * 25.0f + twp->pos.x;
	twp->pos.z = floorf((plpos.z - twp->pos.z) * 0.04f) * 25.0f + twp->pos.z;
	*(float*)0x3C5E78C = twp->pos.x;
	*(float*)0x3C5E788 = twp->pos.z;

	tp->disp(tp);
}

void __cdecl ObjectBeachWaveBG1Display_multi(task* tp)
{
	// Draw only if object is coded for current screen id
	if (tp->twp->id == SplitScreen::GetCurrentScreenNum())
	{
		ObjectBeachWaveBG1Display(tp);
	}
}

void __cdecl ObjectBeachWaveBG1_r(task* tp);
Trampoline ObjectBeachWaveBG1_t(0x501970, 0X501977, ObjectBeachWaveBG1_r);
void __cdecl ObjectBeachWaveBG1_r(task* tp)
{
	TARGET_STATIC(ObjectBeachWaveBG1)(tp);
	tp->disp = ObjectBeachWaveBG1Display_multi;

	// Load one more per player since movement is calculated in main
	for (unsigned int i = 1ui32; i < multiplayer::GetPlayerCount(); ++i)
	{
		auto tp2 = CreateElementalTask(LoadObj_UnknownA | LoadObj_Data1, LEV_6, ObjectBeachWaveBGExec_multi);
		tp2->twp->id = i; // screen ID, only display for that screen
		tp2->twp->pos.y = -1.5f;
		tp2->disp = ObjectBeachWaveBG1Display_multi;
	}
}

void __cdecl ObjectBeachWaveBG2Display_Multi(task* tp)
{
	// Draw only if object is coded for current screen id
	if (tp->twp->id == SplitScreen::GetCurrentScreenNum())
	{
		ObjectBeachWaveBG2Display(tp);
	}
}

void __cdecl ObjectBeachWaveBG2_r(task* tp);
Trampoline ObjectBeachWaveBG2_t(0x4F79C0, 0x4F79C7, ObjectBeachWaveBG2_r);
void __cdecl ObjectBeachWaveBG2_r(task* tp)
{
	TARGET_STATIC(ObjectBeachWaveBG2)(tp);
	tp->disp = ObjectBeachWaveBG2Display_Multi;

	// Load one more per player since movement is calculated in main
	for (unsigned int i = 1ui32; i < multiplayer::GetPlayerCount(); ++i)
	{
		auto tp2 = CreateElementalTask(LoadObj_UnknownA | LoadObj_Data1, LEV_6, ObjectBeachWaveBGExec_multi);
		tp2->twp->id = i; // screen ID, only display for that screen
		tp2->twp->pos.y = -1.5f;
		tp2->disp = ObjectBeachWaveBG2Display_Multi;
	}
}

void __cdecl ObjectBeachWaveBG3Display_Multi(task* tp)
{
	// Draw only if object is coded for current screen id
	if (tp->twp->id == SplitScreen::GetCurrentScreenNum())
	{
		ObjectBeachWaveBG3Display(tp);
	}
}

void __cdecl ObjectBeachWaveBG3_r(task* tp);
Trampoline ObjectBeachWaveBG3_t(0x4F7A00, 0x4F7A07, ObjectBeachWaveBG3_r);
void __cdecl ObjectBeachWaveBG3_r(task* tp)
{
	TARGET_STATIC(ObjectBeachWaveBG3)(tp);
	tp->disp = ObjectBeachWaveBG3Display_Multi;

	// Load one more per player since movement is calculated in main
	for (unsigned int i = 1ui32; i < multiplayer::GetPlayerCount(); ++i)
	{
		auto tp2 = CreateElementalTask(LoadObj_Data1, LEV_6, ObjectBeachWaveBGExec_multi);
		tp2->twp->id = i; // screen ID, only display for that screen
		tp2->twp->pos.y = -1.5f;
		tp2->disp = ObjectBeachWaveBG3Display_Multi;
	}
}
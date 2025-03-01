#include "pch.h"
#include "splitscreen.h"

TaskFunc(ObjectBeachWaveBG_Disp, 0x501130);
TaskFunc(ObjectBeachWaveBG_Disp2, 0x4F76C0);
TaskFunc(ObjectBeachWaveBG_Disp3, 0x4F7760);

void __cdecl ObjectBeachWaveBG_r(task* tp);
void __cdecl ObjectBeachWaveBG2_r(task* tp);
void __cdecl ObjectBeachWaveBG3_r(task* tp);

FastFunctionHookPtr<decltype(&ObjectBeachWaveBG_r)> ObjectBeachWaveBG_h(0x501970);
FastFunctionHookPtr<decltype(&ObjectBeachWaveBG2_r)> ObjectBeachWaveBG2_h(0x4F79C0);
FastFunctionHookPtr<decltype(&ObjectBeachWaveBG3_r)> ObjectBeachWaveBG3_h(0x4F7A00);

void __cdecl ObjectBeachWaveBG_Exec_m(task* tp)
{
	auto twp = tp->twp;

	if (SplitScreen::IsScreenEnabled(twp->id))
	{
		NJS_VECTOR plpos;
		GetPlayerPosition(twp->id, 0, &plpos, 0);
		twp->pos.x = floorf((plpos.x - twp->pos.x) * 0.04f) * 25.0f + twp->pos.x;
		twp->pos.z = floorf((plpos.z - twp->pos.z) * 0.04f) * 25.0f + twp->pos.z;
		*(float*)0x3C5E78C = twp->pos.x;
		*(float*)0x3C5E788 = twp->pos.z;
		tp->disp(tp);
	}
}

void __cdecl ObjectBeachWaveBG_Disp_m(task* tp)
{
	// Draw only if object is coded for current screen id
	if (SplitScreen::IsScreenEnabled(tp->twp->id) && tp->twp->id == SplitScreen::GetCurrentScreenNum())
	{
		ObjectBeachWaveBG_Disp(tp);
	}
}

void __cdecl ObjectBeachWaveBG_r(task* tp)
{
	ObjectBeachWaveBG_h.Original(tp);
	tp->disp = ObjectBeachWaveBG_Disp_m;

	// Load one more per player since movement is calculated in main
	for (int i = 1; i < PLAYER_MAX; ++i)
	{
		auto tp2 = CreateElementalTask(LoadObj_UnknownA | LoadObj_Data1, LEV_6, ObjectBeachWaveBG_Exec_m);
		tp2->twp->id = i; // screen ID, only display for that screen
		tp2->twp->pos.y = -1.5f;
		tp2->disp = ObjectBeachWaveBG_Disp_m;
	}
}

void __cdecl ObjectBeachWaveBG_Disp2_m(task* tp)
{
	// Draw only if object is coded for current screen id
	if (SplitScreen::IsScreenEnabled(tp->twp->id) && tp->twp->id == SplitScreen::GetCurrentScreenNum())
	{
		ObjectBeachWaveBG_Disp2(tp);
	}
}

void __cdecl ObjectBeachWaveBG2_r(task* tp)
{
	ObjectBeachWaveBG2_h.Original(tp);
	tp->disp = ObjectBeachWaveBG_Disp2_m;

	// Load one more per player since movement is calculated in main
	for (int i = 1; i < PLAYER_MAX; ++i)
	{
		auto tp2 = CreateElementalTask(LoadObj_UnknownA | LoadObj_Data1, LEV_6, ObjectBeachWaveBG_Exec_m);
		tp2->twp->id = i; // screen ID, only display for that screen
		tp2->twp->pos.y = -1.5f;
		tp2->disp = ObjectBeachWaveBG_Disp2_m;
	}
}

void __cdecl ObjectBeachWaveBG_Disp3_m(task* tp)
{
	// Draw only if object is coded for current screen id
	if (SplitScreen::IsScreenEnabled(tp->twp->id) && tp->twp->id == SplitScreen::GetCurrentScreenNum())
	{
		ObjectBeachWaveBG_Disp3(tp);
	}
}

void __cdecl ObjectBeachWaveBG3_r(task* tp)
{
	ObjectBeachWaveBG3_h.Original(tp);
	tp->disp = ObjectBeachWaveBG_Disp3_m;

	// Load one more per player since movement is calculated in main
	for (int i = 1; i < PLAYER_MAX; ++i)
	{
		auto tp2 = CreateElementalTask(LoadObj_Data1, LEV_6, ObjectBeachWaveBG_Exec_m);
		tp2->twp->id = i; // screen ID, only display for that screen
		tp2->twp->pos.y = -1.5f;
		tp2->disp = ObjectBeachWaveBG_Disp3_m;
	}
}

void patch_beach_wavebg_init()
{
	ObjectBeachWaveBG_h.Hook(ObjectBeachWaveBG_r);
	ObjectBeachWaveBG2_h.Hook(ObjectBeachWaveBG2_r);
	ObjectBeachWaveBG3_h.Hook(ObjectBeachWaveBG3_r);
}

RegisterPatch patch_beach_wavebg(patch_beach_wavebg_init);
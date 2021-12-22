#include "pch.h"
#include "ObjectBeachWave.h"
#include "splitscreen.h"

void __cdecl ObjectBeachWave_act1_disp_j(task* tp)
{
	if (tp->twp->btimer == SplitScreen::numScreen)
	{
		Obj_EC1Water_Display((ObjectMaster*)tp);
	}
}

void __cdecl ObjectBeachWave_act1_exec_j(task* tp)
{
	Obj_EC1Water_Main((ObjectMaster*)tp);

	auto twp = tp->twp;

	NJS_VECTOR plpos;
	GetPlayerPosition(twp->btimer, 0, &plpos, 0);
	twp->pos.x = floorf((plpos.x - twp->pos.x) * 0.04f) * 25.0f + twp->pos.x;
	twp->pos.z = floorf((plpos.z - twp->pos.z) * 0.04f) * 25.0f + twp->pos.z;
	*(float*)0x3C5E78C = twp->pos.x;
	*(float*)0x3C5E788 = twp->pos.z;
}

void __cdecl ObjectBeachWave_act1_r(task* tp);
Trampoline ObjectBeachWave_act1_t(0x501970, 0X501977, ObjectBeachWave_act1_r);
void __cdecl ObjectBeachWave_act1_r(task* tp)
{
	TARGET_STATIC(ObjectBeachWave_act1)(tp);
	tp->exec = ObjectBeachWave_act1_exec_j;
	tp->disp = ObjectBeachWave_act1_disp_j;

	// Load one more per player since movement is calculated in main
	for (int i = 1; i < player_count; ++i)
	{
		auto tp2 = CreateElementalTask(LoadObj_UnknownA | LoadObj_Data1, LEV_6, ObjectBeachWave_act1_exec_j);
		tp2->twp->btimer = 1;
		tp2->twp->pos.y = -1.5f;
		tp2->exec = ObjectBeachWave_act1_exec_j;
		tp2->disp = ObjectBeachWave_act1_disp_j;
	}
}
#include "pch.h"

TaskHook MilesJiggle_t((intptr_t)Tails_Jiggle_Main);
TaskHook MilesDirectAhead_t(0x45DAD0);
TaskHook MilesDirectAhead2_t(0x45DE20);

static void __cdecl MilesDirectAhead2_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	MilesDirectAhead2_t.Original(tp);
}

static void __cdecl MilesDirectAhead_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	MilesDirectAhead_t.Original(tp);
}


static void __cdecl MilesJiggle_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	MilesJiggle_t.Original(tp);
}

void initMilesPatches()
{
	MilesJiggle_t.Hook(MilesJiggle_r);
	MilesDirectAhead_t.Hook(MilesDirectAhead_r);
	MilesDirectAhead2_t.Hook(MilesDirectAhead2_r);
}
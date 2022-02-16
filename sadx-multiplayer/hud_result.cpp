#include "pch.h"
#include "hud_result.h"
#include "result.h"

static void late_DisplayTotalScoreM(task* tp)
{
	if (MissedFrames)
	{
		return;
	}

	// draw
}

static void __cdecl CalcTotalScoreM(task* tp)
{
	StartLevelCutscene(1);

	tp->disp(tp);
}

void LoadMultiplayerResult()
{
	auto tp = CreateElementalTask(LoadObj_Data1, LEV_5, CalcTotalScoreM);
	tp->disp = late_DisplayTotalScoreM;
}
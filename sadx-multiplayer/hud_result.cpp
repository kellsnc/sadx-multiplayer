#include "pch.h"
#include "hud_result.h"
#include "result.h"
#include "splitscreen.h"

static void DisplayMultiResultScreeen(taskwk* twp, int pnum)
{
	auto ratio = SplitScreen::GetScreenRatio(pnum);

	float screenX = HorizontalResolution * ratio->x;
	float screenY = VerticalResolution * ratio->y;
	float scaleY = VerticalStretch * ratio->h;
	float scaleX = HorizontalStretch * ratio->w;
	float scale = min(scaleX, scaleY);

	Spr_MRaceDisp.tlist = &MILESRACE_TEXLIST;
	Spr_MRaceDisp.sx = Spr_MRaceDisp.sy = scale * twp->value.f;
	Spr_MRaceDisp.tanim = &TailsRace_TEXANIM;
	Spr_MRaceDisp.p.y = screenY + 240.0f * scaleY;
	Spr_MRaceDisp.p.x = screenX + 320.0f * scaleX;

	for (int i = 0; i < 4; ++i)
	{
		njDrawSprite2D_Queue(&Spr_MRaceDisp, (GetWinnerMulti() == pnum ? 3 : 7) + i, 22046.4f, NJD_SPRITE_ALPHA, QueuedModelFlagsB_SomeTextureThing);
	}
}

static void late_DisplayTotalScoreM(task* tp)
{
	if (MissedFrames)
	{
		return;
	}

	njSetTexture(&MILESRACE_TEXLIST);
	ghDefaultBlendingMode();

	if (SplitScreen::IsActive())
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		DisplayMultiResultScreeen(tp->twp, SplitScreen::GetCurrentScreenNum());
		SplitScreen::RestoreViewPort();
	}
	else
	{
		// todo: get real player id when online
		DisplayMultiResultScreeen(tp->twp, 0);
	}
}

static void __cdecl CalcTotalScoreM(task* tp)
{
	auto twp = tp->twp;

	switch (twp->mode)
	{
	case 0:
		twp->value.f += 0.1f;
		if (twp->value.f > 2.0f)
		{
			twp->value.f = 2.0f;
			twp->mode = 1;
		}
		break;
	case 1:
		break;
	case 2:
		njReleaseTexture(&MILESRACE_TEXLIST);
		SetChangeGameMode(1);
		break;
	}
	
	tp->disp(tp);
}

void LoadMultiplayerResult()
{
	auto tp = CreateElementalTask(LoadObj_Data1, LEV_5, CalcTotalScoreM);
	tp->disp = late_DisplayTotalScoreM;

	LoadPVM("MILESRACE", &MILESRACE_TEXLIST);
}
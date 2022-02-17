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

	// Dim screen
	DrawRect_Queue(screenX, screenY, screenX + 640.0f * scaleX, screenY + 480.0f * scaleY, 22048.0, static_cast<int>(twp->value.f * 30.0f) << 24, QueuedModelFlagsB_EnableZWrite);

	// Draw win/lose status
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

static void __cdecl late_DisplayTotalScoreM(task* tp)
{
	if (MissedFrames)
	{
		return;
	}

	njSetTexture(&MILESRACE_TEXLIST);
	ghDefaultBlendingMode();

	if (SplitScreen::IsActive())
	{
		auto pnum = SplitScreen::GetCurrentScreenNum();

		if (SplitScreen::IsScreenEnabled(pnum))
		{
			SplitScreen::SaveViewPort();
			SplitScreen::ChangeViewPort(-1);
			DisplayMultiResultScreeen(tp->twp, pnum);
			SplitScreen::RestoreViewPort();
		}
	}
	else
	{
		// todo: get real player id when online
		DisplayMultiResultScreeen(tp->twp, 0);
	}
}

static void __cdecl CalcTotalScoreM_dest(task* tp)
{
	njReleaseTexture(&MILESRACE_TEXLIST);
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
		if (++twp->timer.l > 500 || MenuSelectButtonsPressed())
		{
			twp->mode = 2;
		}
		break;
	case 2:
		SetChangeGameMode(1);
		twp->mode = 3;
		break;
	}
	
	tp->disp(tp);
}

void LoadMultiplayerResult()
{
	auto tp = CreateElementalTask(LoadObj_Data1, LEV_5, CalcTotalScoreM);
	tp->disp = late_DisplayTotalScoreM;
	tp->dest = CalcTotalScoreM_dest;

	LoadPVM("MILESRACE", &MILESRACE_TEXLIST);
}
#include "pch.h"
#include <splitscreen.h>
#include "patches/pinball.h"

static uint8_t BallCount[PLAYER_MAX - 1]; //we don't need to do Player 1
FastFunctionHook<void> Casino_Setup_h(0x5C0960);
FunctionPointer(int, initMissionCtrl, (), 0x5919E0);

#define BallMax 3

static bool SubBallCount(uint8_t pnum)
{
	if (pnum > multiplayer::GetPlayerCount())
		return false;

	if (!pnum)
		pnum++;

	return --BallCount[pnum - 1] > 0;
}

void ResetBallCount()
{
	memset(BallCount, BallMax, sizeof(uint8_t) * PLAYER_MAX - 1);
}

static int8_t GetLastPlayerAlive()
{
	int count = 0;
	uint8_t id = 0;
	const int playerCount = multiplayer::GetPlayerCount();
	for (uint8_t i = 0; i < playerCount; i++)
	{
		if (i > 0 && BallCount[i - 1] == 0)
		{
			count++;
		}
		else if (!i && Casino_BallCount == 0)
		{
			count++;
		}
		else
		{
			id = i; //alive
		}
	}

	if (count != playerCount)
		return -1;

	return id;
}

void __cdecl Casino_Setup_r()
{
	Casino_Setup_h.Original();
	ResetBallCount();
	memset(PinballCameraMode, 0, sizeof(uint8_t) * PLAYER_MAX);
}

FastFunctionHook<void, task*> RdCasinoCheckAct3toAct12_h(0x5C0700);

static void RdCasinoCheckAct3toAct12_r(task* tp)
{

	if (multiplayer::IsActive() == false)
	{
		return RdCasinoCheckAct3toAct12_h.Original(tp);
	}


	if (ChkGameMode())
	{
		for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
		{
			auto p = playertwp[i];
			if (p->pos.y < -10.0f)
			{
				dsPlay_oneshot(SE_CA_OUTFALL, 0, 0, 0);

				if (!i && --Casino_BallCount || i && SubBallCount(i))
				{
					playerwk* pl_pwp = playerpwp[i];
					Angle3* ang = &playertwp[i]->ang;
					SetPositionP(i, 137.0f, 35.0f, 64.0f);
					pl_pwp->free.sb[0] &= ~2;
					ang->y = 0;
					ang->z = 0;
					ang->x = 0;
					pl_pwp->spd.z = 0.0f;
					pl_pwp->spd.y = 0.0f;
					pl_pwp->spd.x = 0.0f;
				}
				else if (GetLastPlayerAlive() != -1)
				{
					if (ulGlobalMode == MD_MISSION)
					{
						initMissionCtrl();
					}
					ADX_Close();
					ReleaseTextureOnCasino(1);
					pause_flg = 1;
					CameraReleaseEventCamera();
					SetInputP(0, PL_OP_LETITGO);
					// Don't dump in sewers if over 100 Rings.
					if (GetTotalRingsM() >= 100)
					{
						LandChangeStage(-2);
						AddSetStage(-2);
						AddCameraStage(-2);
						AdvanceAct(-2);
						Casino_Got100Rings = 1;
					}
					else
					{
						LandChangeStage(-1);
						AddSetStage(-1);
						AddCameraStage(-1);
						AdvanceAct(-1);
					}

					for (int j = 0; j < multiplayer::GetPlayerCount(); j++)
					{
						SetPlayerInitialPosition(playertwp[j]);
					}
					tp->twp->mode = 0;
					break;
				}
				else
				{
					SetPositionP(i, 0.0f, 50000.0f, 0.0f);
					SetInputP(i, PL_OP_PLACEON);
				}
			}
		}
	}
	else
	{
		Casino_BallCount = BallMax;
		ResetBallCount();
	}
}

static NJS_TEXANIM anim_balls[1] = {
	{ 0x40, 0x20, 0x20, 0x10, 0, 0, 0x100, 0x100, 0, 0 }
};

/**_SC_NUMBERS ballscount[PLAYER_MAX] =
{
	{9, 0x90, 0, 3, 0, {0}, 1.4f, 0xFFFFFFFF},
	{9, 0x90, 0, 3, 0, {0}, 1.4f, 0xFFFFFFFF},
	{9, 0x90, 0, 3, 0, {0}, 1.4f, 0xFFFFFFFF},
	{9, 0x90, 0, 3, 0, {0}, 1.4f, 0xFFFFFFFF},
};*/

DataPointer(_SC_NUMBERS, ballscount, 0x1C46CB8);
DataPointer(_SC_NUMBERS, ballscolon, 0x1C46CD8);

FastFunctionHook<void, task*> late_DisplayBalls_h(0x5C0B70);

static void FUCK(int pnum)
{

	auto ratio = splitscreen::GetScreenRatio(pnum);

	Float screenX = HorizontalResolution * ratio->x;
	Float screenY = VerticalResolution * ratio->y;
	Float scaleY = VerticalStretch * ratio->h;
	Float scaleX = HorizontalStretch * ratio->w;
	Float scale = min(scaleX, scaleY);


	SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
	njSetPerspective(0x31C7);
	___njFogDisable();
	njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
	njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
	NJS_SPRITE _sp;
	_sp.p.x = screenX * 478.0 * scaleX;
	_sp.p.y = 22.0f * scale + 16.0f * scale;
	_sp.tlist = &hyoji_balls_TEXLIST;
	_sp.tanim = anim_balls;
	_sp.sx = scale;
	_sp.sy = scale;

	late_DrawSprite2D(&_sp, 0, 22046.9f, 0x20, LATE_LIG);

	Float scr_x = screenX * 478.0f * scaleX;
	if (pnum)
	{
		ballscount.value = 1;
	}
	else
	{
		ballscount.value = 0;
	}
	ballscolon.scl = scale * 1.4f;
	ballscolon.pos.x = scr_x + 29.0f * scale;
	ballscolon.pos.y = 22.0f * scale;

	ballscount.scl = scale * 1.4f;
	ballscount.pos.x = scr_x + 48.0f * scale;
	ballscount.pos.y = ballscolon.pos.y;

	DrawSNumbers(&ballscolon);
	DrawSNumbers(&ballscount);
	___njFogEnable();
	njSetPerspective(LastHorizontalFOV_BAMS);
	ResetMaterial();

}

static void late_DisplayBalls(task* tp)
{
	if (multiplayer::IsActive() == false)
	{
		return late_DisplayBalls_h.Original(tp);
	}

	if (hyoji_balls_TEXLIST.textures->texaddr)
	{
		if (splitscreen::IsActive())
		{
			for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
			{
				if (splitscreen::GetCurrentScreenNum() != 0)
				{

					auto ratio = splitscreen::GetScreenRatio(i);

					Float screenX = HorizontalResolution * ratio->x;
					Float screenY = VerticalResolution * ratio->y;
					Float scaleY = VerticalStretch * ratio->h;
					Float scaleX = HorizontalStretch * ratio->w;
					Float scale = min(scaleX, scaleY);


					SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
					njSetPerspective(0x31C7);
					___njFogDisable();
					njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
					njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
					NJS_SPRITE _sp;
					_sp.p.y = ScreenRaitoY * 22.0f + 16.0f;
					_sp.tlist = &hyoji_balls_TEXLIST;
					_sp.tanim = anim_balls;
					_sp.sx = 1.0f;
					_sp.sy = 1.0f;
					_sp.p.x = ScreenRaitoX * 478.0f;
					late_DrawSprite2D(&_sp, 0, 22046.9f, 0x20, LATE_LIG);
					Float scr_x = ScreenRaitoX * 478.0f;
					ballscount.value = Casino_BallCount - 1;
					ballscolon.scl = 1.4f;
					ballscolon.pos.x = scr_x + 29.0f;
					ballscount.scl = 1.4f;
					ballscolon.pos.y = ScreenRaitoY * 22.0f;
					ballscount.pos.y = ballscolon.pos.y;
					ballscount.pos.x = scr_x + 48.0f;
					DrawSNumbers(&ballscolon);
					DrawSNumbers(&ballscount);
					___njFogEnable();
					njSetPerspective(LastHorizontalFOV_BAMS);
				}
			}
		}
	}
}


void patch_ball_init()
{
	Casino_Setup_h.Hook(Casino_Setup_r);
	RdCasinoCheckAct3toAct12_h.Hook(RdCasinoCheckAct3toAct12_r);
	late_DisplayBalls_h.Hook(late_DisplayBalls);
}

RegisterPatch patch_ball(patch_ball_init);
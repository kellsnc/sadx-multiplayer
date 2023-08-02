#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "VariableHook.hpp"
#include "utils.h"
#include "multiplayer.h"
#include "camera.h"

static void Normal()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (!playerpwp[i])
		{
			continue;
		}

		auto speed = playerpwp[i]->spd.x;
		float scalor = speed < 7.82f ? njSqrt(speed * 0.12787724f) : 1.0f;

		njSetPerspective_m(i, (Angle)(scalor * 5461.0f + 12743.0f));

		if (playertwp[i]->pos.y < -7405.0f)
		{
			MirenSoundFinishBgm();
			KillHimByFallingDownP(i);
		}
	}
}

static void InitPlayers()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			auto char_id = TASKWK_CHARID(ptwp);

			switch (char_id)
			{
			case Characters_Sonic:
				SetInputP(i, PL_OP_SNOWBOARDING);
				CreateElementalTask(3, 2, (TaskFuncPtr)0x4959E0)->twp->counter.b[0] = i;
				break;
			case Characters_Tails:
				SetInputP(i, PL_OP_SNOWBOARDING);
				CreateElementalTask(3, 2, (TaskFuncPtr)0x461510)->twp->counter.b[0] = i;
				break;
			}
		}
	}
}

static void execRound_m(task* tp)
{
	auto mwp = tp->mwp;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		SetFreeCameraMode_m(i, FALSE);
	}

	auto& mode = mwp->work.b[0];

	switch (mode)
	{
	case 0:
		SetDefaultNormalCameraMode(CAMMD_SNOWBOARD, CAMADJ_RELATIVE3C);
		InitPlayers();
		mode = 1;
		break;
	case 1:
		if (MirenSoundSetStartBgmTask(MusicIDs_SandHill, 8) != FALSE)
		{
			mode = 2;
		}
		break;
	case 2:
		SetViewAngle(0x31C7);
		SandObjSetManageTask();
		PadReadOn();
		EnablePause();
		mode = 3;
		break;
	case 3:
		Normal();
		break;
	}
}

static void __cdecl execRound_Sandboard_r(task* tp);
Trampoline execRound_Sandboard_t(0x597BD0, 0x597BD6, execRound_Sandboard_r);
static void __cdecl execRound_Sandboard_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		execRound_m(tp);
	}
	else
	{
		TARGET_STATIC(execRound_Sandboard)(tp);
	}
}
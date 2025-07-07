#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "patches_egm2.h"
#include <camera.h>

// Egg Walker

enum
{
	MD_RDBOSS_INIT = 0,
	MD_RDBOSS_INIT2 = 1,
	MD_RDBOSS_WAIT = 2,
	MD_RDBOSS_END = 3
};

FastFunctionHook<void, task*> RdEgm2_h(0x5758D0);

FastUsercallHookPtr<Bool(*)(task* tp), rEAX, rEAX> egm2_top_damage_check_h(0x578930);
Bool egm2_top_damage_check(task* tp)
{
	if (multiplayer::IsActive() == false)
	{
		return egm2_top_damage_check_h.Original(tp);
	}


	NJS_POINT3 v_3 = { 0.0f, 0.5f, 0.0f };
	taskwk* twp = tp->twp;
	bossextwk2* egm2wk = (bossextwk2*)tp->awp;

	if ((twp->cwp->info[1].damage & 0x10) == 0)
	{
		return FALSE;
	}

	egm2wk->HitPoint = egm2wk->HitPoint - 1.0f;
	PlayEggmanVoice(2);
	if (egm2wk->HitPoint <= 0.0f)
	{
		egm2wk->HitPoint = 0.0f;
	}
	dsPlay_oneshot_Dolby(480, 0, 0, 64, 120, twp);

	auto player = CCL_IsHitPlayer(twp);
	if (player)
	{
		auto pnum = player->counter.b[0];
		CreateBomb(&playertwp[pnum]->pos, 0.5f);
		PConvertVector_P2G(playertwp[pnum], &v_3);
		SetVelocityP(pnum, v_3.x, v_3.y, v_3.z);
	}
	twp->cwp->info[1].damage &= 0xF3u;
	twp->cwp->info[1].damage |= 0xCu;
	return TRUE;
}

FastFunctionHook<void, NJS_POINT3*, NJS_POINT3*> CameraSetEGM2_h(0x578270);
FunctionPointer(void, egm2_camera, (_OBJ_CAMERAPARAM * param), 0x5781F0);

void CameraSetEGM2_r(NJS_POINT3* a1, NJS_POINT3* a2)
{
	CameraSetEGM2_h.Original(a1, a2);

	if (multiplayer::IsActive())
	{
		if (ccsi_flag)
		{
			for (int i = 1; i < PLAYER_MAX; ++i) //todo rework to avoid this hacky stuff
			{
				CameraSetEventCameraFunc_m(i, egm2_camera, 0, CAMADJ_NORMAL_S);
			}
		}
	}
}

void Rd_Bossegm2(task* tp)
{
	Float x, y, z;
	taskwk* twp = tp->twp;
	SetFreeCameraMode(0);

	switch (twp->mode)
	{
	default:
		RdEgm2_h.Original(tp);
		break;
	case MD_RDBOSS_WAIT:

		z = 930.0f;

		for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
		{
			if (!playertwp[i])
				continue;

			float offset = i * 5.0f;
			auto p = playertwp[i];

			if (p->pos.z < 930.0f || (z = 1480.0f, p->pos.z > 1480.0f))
				playertwp[i]->pos.z = z;
			
			x = -515.0f;

			if (playertwp[i]->pos.x < -515.0f || (x = -375.0f, playertwp[i]->pos.x > -375.0f))
				playertwp[i]->pos.x = x;

			y = -3.0f;
			if (playertwp[i]->pos.y < -3.0f || (y = 45.0f, playertwp[i]->pos.y > 45.0f))
				playertwp[i]->pos.y = y;
		}

		break;
	}
}

void patch_egm2_init()
{
	egm2_top_damage_check_h.Hook(egm2_top_damage_check);
	RdEgm2_h.Hook(Rd_Bossegm2);
	CameraSetEGM2_h.Hook(CameraSetEGM2_r);
}

#ifdef MULTI_TEST
	RegisterPatch patch_egm2(patch_egm2_init);
#endif
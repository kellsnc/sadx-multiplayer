#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "camera.h"
#include "camerafunc.h"
#include "splitscreen.h"

FastFunctionHook<void, taskwk*, chaoswk*> turnToPlayer_h(0x7AE660);
FastFunctionHook<void, task*> BossChaos0_h(0x548640);
FastFunctionHook<void, task*> Bg_Chaos0_h(0x545DF0);
FastUsercallHookPtr<Angle(*)(chaoswk* cwk), rEAX, rEDI> setApartTargetPos_h(0x546460);
FastUsercallHookPtr<void(*)(chaoswk* cwk, taskwk* twp), noret, rECX, rESI> chaos0Pole_h(0x547260);
FastUsercallHookPtr<void(*)(chaoswk* cwk, taskwk* twp, bosswk* bwk), noret, rEDI, rESI, stack4> chaos0Punch_h(0x546790);

UsercallFuncVoid(SetChaos0LandFlags, (Bool flag), (flag), 0x5485E0, rEDX);

//Patches Chaos effects to make them display on other player screens, it is done by manually setting a disp function that is lacking in vanilla.

#pragma region drawEffectChaos0EffectB
void __cdecl drawEffectChaos0EffectBDestroy(task* tp)
{
	tp->awp = nullptr;
}

void __cdecl drawEffectChaos0EffectBDisplay(task* tp)
{
	if (!loop_count)
	{
		NJS_ARGB color = { 0.5f, 0.5f, 0.5f, 0.5f };
		___njSetConstantMaterial(&color);
		njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_ONE);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
		njSetTexture(&CHAOS_EFFECT_TEXLIST);

		for (int i = 0; i < reinterpret_cast<int>(tp->awp); ++i)
		{
			auto ctwp = (taskwk*)BigEntityArray[i];
			njPushMatrixEx();
			njTranslateV(0, &ctwp->pos);
			ROTATEY(0, camera_twp->ang.y);
			ROTATEX(0, camera_twp->ang.x);
			njScale(0, ctwp->scl.x, ctwp->scl.x, ctwp->scl.x);
			late_DrawSprite3D((NJS_SPRITE*)0x3D0D714, (Sint32)ctwp->counter.f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, LATE_LIG);
			njPopMatrixEx();
		}

		ResetMaterial();
		njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
	}
}

void __cdecl drawEffectChaos0EffectB_r(task* tp)
{
	if (!tp->awp)
	{
		tp->awp = reinterpret_cast<anywk*>(BigEntityArrayLength);
		tp->dest = drawEffectChaos0EffectBDestroy;
		tp->disp = drawEffectChaos0EffectBDisplay;
	}

	tp->disp(tp);

	BigEntityArrayLength = 0;
	FreeTask(tp);
}
#pragma endregion

#pragma region drawEffectChaos0LightParticle
void __cdecl drawEffectChaos0LightParticleDestroy(task* tp)
{
	tp->awp = nullptr;
}

void __cdecl drawEffectChaos0LightParticleDisplay(task* tp)
{
	if (!loop_count)
	{
		njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_ONE);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
		njSetTexture(&CHAOS_EFFECT_TEXLIST);

		for (int i = 0; i < reinterpret_cast<int>(tp->awp); ++i)
		{
			auto ctwp = (taskwk*)BigEntityArrayB[i];
			njPushMatrixEx();

			Float blend = max(0.0f, min((ctwp->timer.f - 0.25f) * 2, 1.0f));

			NJS_ARGB color;

			color.r = 0.5f;
			color.g = blend * 0.5f + 0.25f;
			color.b = blend * 0.098f + 0.25f;
			color.a = 0.75f - 0.5f * blend;

			___njSetConstantMaterial(&color);

			njTranslateV(0, &ctwp->pos);
			ROTATEY(0, camera_twp->ang.y);
			ROTATEX(0, camera_twp->ang.x);
			njScale(0, ctwp->scl.x, ctwp->scl.x, ctwp->scl.x);
			late_DrawSprite3D((NJS_SPRITE*)0x3D0D73C, (Sint32)ctwp->counter.f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, LATE_LIG);
			njPopMatrixEx();
		}

		ResetMaterial();
		njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
	}
}

void __cdecl drawEffectChaos0LightParticle_r(task* tp)
{
	if (!tp->awp)
	{
		tp->awp = reinterpret_cast<anywk*>(BigEntityArrayBLength);
		tp->dest = drawEffectChaos0LightParticleDestroy;
		tp->disp = drawEffectChaos0LightParticleDisplay;
	}

	tp->disp(tp);

	BigEntityArrayBLength = 0;
	FreeTask(tp);
}
#pragma endregion

#pragma region dispEffectChaos0AttackA
void __cdecl dispEffectChaos0AttackADisplay(task* tp)
{
	if (!loop_count)
	{
		auto twp = tp->twp;

		if (twp->counter.f < 15.0f)
		{
			NJS_ARGB color = { 0.5f, -0.5f, -0.5f, -0.5f };
			___njSetConstantMaterial(&color);

			njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_ONE);
			njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
			njSetTexture(&CHAOS_EFFECT_TEXLIST);

			njPushMatrixEx();
			njTranslateV(0, &twp->pos);
			ROTATEY(0, camera_twp->ang.y);
			ROTATEX(0, camera_twp->ang.x);
			auto scl = twp->scl.x + 0.5f;
			njScale(0, scl, scl, scl);
			late_DrawSprite3D((NJS_SPRITE*)0x3D0D56C, (Sint32)twp->counter.f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, LATE_LIG);
			njPopMatrixEx();

			ResetMaterial();
			njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
			njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
		}
	}
}

void __cdecl dispEffectChaos0AttackA_r(task* tp)
{
	if (!tp->disp)
	{
		tp->disp = dispEffectChaos0AttackADisplay;
	}

	auto twp = tp->twp;

	twp->counter.f += 0.75f;

	if (twp->counter.f < 15.0f)
	{
		twp->pos = *reinterpret_cast<NJS_POINT3*>(twp->timer.ptr);
	}
	else
	{
		FreeTask(tp);
	}

	tp->disp(tp);
}
#pragma endregion

// series of hacks to make Chaos able to attack other players

void turnToPlayer_r(taskwk* twp, chaoswk* bwp)
{
	if (!multiplayer::IsActive())
	{
		return turnToPlayer_h.Original(twp, bwp);
	}

	float dist = 0.0f;

	auto ptwp = playertwp[GetBossTargetPlayer()];

	if (ptwp)
	{
		twp->ang.y = AdjustAngle(
			twp->ang.y,
			njArcTan2(ptwp->pos.z - twp->pos.z, ptwp->pos.x - twp->pos.x) - bwp->attack_yang,
			chaosparam->turn_spd);
	}
}

Angle setApartTargetPos_r(chaoswk* cwk)
{
	auto ptwp = playertwp[GetBossTargetPlayer()];

	if (!multiplayer::IsEnabled() || !ptwp)
	{
		return setApartTargetPos_h.Original(cwk);
	}

	Angle ang = njArcTan2(chaosparam->field_center_pos.z - ptwp->pos.z, chaosparam->field_center_pos.x - ptwp->pos.x);

	cwk->tgtpos.x = njCos(ang) * 60.0f + chaosparam->field_center_pos.x;
	cwk->tgtpos.z = njSin(ang) * 60.0f + chaosparam->field_center_pos.z;

	return ang;
}

void chaos0Pole_r(chaoswk* cwk, taskwk* twp)
{
	auto ptwp = playertwp[GetBossTargetPlayer()];

	if (!multiplayer::IsEnabled() || !ptwp)
	{
		return chaos0Pole_h.Original(cwk, twp);
	}

	if (twp->smode)
	{
		if (twp->smode != 1)
		{
			return;
		}
	}
	else
	{
		cwk->bwk.req_action = MD_CHAOS_STND;
		bossmtn_flag &= 0xFDu;
		if (twp->counter.b[0] == 2)
		{
			chaos_pole_punch_num = 5;
			twp->counter.b[0] = 3;
		}
		twp->smode = MD_CHAOS_STND;
	}

	turnToPlayer_r(twp, cwk);

	if (chaos_pole_punch_num > 0)
	{
		float diffX = ptwp->pos.x - twp->pos.x;
		float diffZ = ptwp->pos.z - twp->pos.z;

		if (diffZ * diffZ + diffX * diffX >= 6400.0f)
		{
			if (twp->mode != 4)
			{
				chaos_reqmode = 4;
				chaos_nextmode = twp->mode;
				chaos_oldmode = chaos_nextmode;
			}
			twp->counter.b[0] = 3;
		}
		else
		{
			chaos_punch_num = MD_CHAOS_STND;
			--chaos_pole_punch_num;
			if (twp->mode != 6)
			{
				chaos_reqmode = 6;
				chaos_nextmode = twp->mode;
				chaos_oldmode = chaos_nextmode;
			}
		}
	}
	else
	{
		if (twp->mode != 4)
		{
			chaos_reqmode = 4;
			chaos_nextmode = twp->mode;
			chaos_oldmode = chaos_nextmode;
		}

		twp->counter.b[0] = MD_CHAOS_STND;
	}
}

void chaos0Punch_r(chaoswk* cwk, taskwk* twp, bosswk* bwk)
{
	auto ptwp = playertwp[GetBossTargetPlayer()];

	if (twp->smode != 3)
	{
		return chaos0Punch_h.Original(cwk, twp, bwk);
	}
	else
	{
		if (twp->wtimer > 6)
		{
			float x, y, z;
			turnToPlayer_r(twp, cwk);

			x = ptwp->pos.x - twp->pos.x;
			z = ptwp->pos.z - twp->pos.z;
			cwk->attack_zang = -njArcTan2(ptwp->pos.y - twp->pos.y, njSqrt(z * z + x * x));

			x = twp->pos.x - ptwp->pos.x;
			y = twp->pos.y - ptwp->pos.y;
			z = twp->pos.z - ptwp->pos.z;
			cwk->attack_dist = njSqrt(x * x + y * y + z * z);

			SetEffectPunchTameParticle((NJS_POINT3*)0x3C63DDC, 0);
		}

		if (!--twp->wtimer)
		{
			bossmtn_flag &= 0xFCu;
			bwk->work.l = 0;
			twp->smode = 4;
			cwk->etcflag |= 2u;
			setDrop(twp, 10, 0.5f, 2.0f);
			setShakeHeadParam(1.5f);
			dsPlay_oneshot_Dolby(334, 0, 0, 207, 120, twp);
			flt_3C63CEC = 2.9166667f / cwk->attack_dist;

			if (twp->pos.y > 60.0f)
			{
				flt_3C63CEC = flt_3C63CEC * 1.5f;
			}
		}
	}
}

void Bg_Chaos0_r(task* tp)
{
	if (!tp->disp)
	{
		tp->disp = Bg_Chaos0_r;
	}

	SetChaos0LandFlags(GetCameraMode_m(splitscreen::GetCurrentScreenNum()) != CAMMD_CHAOS_P);
	Bg_Chaos0_h.Original(tp);
}

void BossChaos0_r(task* tp)
{
	auto twp = tp->twp;

	auto wk = (chaoswk*)tp->awp;

	if (!twp || !wk)
	{
		SetDefaultNormalCameraMode(CAMMD_CHAOS, CAMADJ_NONE);
		BossChaos0_h.Original(tp);
		return;
	}

	auto oldcam = wk->camera_mode;

	BossChaos0_h.Original(tp);

	if (oldcam != wk->camera_mode)
	{
		for (int i = 1; i < PLAYER_MAX; ++i)
		{
			CameraSetEventCamera_m(i, wk->camera_mode, 0);
		}
	}
}

//This control when Chaos can be damaged, the game does not call this function when Chaos isn't vulnerable.
Bool chkChaos0Collision_r(taskwk* twp)
{
	if ((twp->flag & PL_FLAG_DAMAGE) == 0)
		return FALSE;

	NJS_POINT3 vel;

	vel.x = -2.0f;
	vel.y = 2.0f;
	vel.z = 0.0f;

	bossmtn_flag &= 0xFCu;

	if (twp->smode == 4 || twp->pos.y >= 60.0f)
	{
		if (twp->mode != MD_CHAOS_FALL)
		{
			chaos_reqmode = MD_CHAOS_FALL;
			chaos_oldmode = twp->mode;
			chaos_nextmode = chaos_oldmode;
		}
	}
	else
	{
		if (twp->mode != MD_CHAOS_DAMAGE)
		{
			chaos_reqmode = MD_CHAOS_DAMAGE;
			chaos_oldmode = twp->mode;
		}
		chaos_nextmode = 16;
	}

	auto player = CCL_IsHitPlayer(twp);
	if (player)
	{
		PConvertVector_P2G(player, &vel);
		SetVelocityP(player->counter.b[0], vel.x, vel.y, vel.z);
	}
	return TRUE;
}

void patch_chaos0_init()
{
	WriteJump(drawEffectChaos0EffectB, drawEffectChaos0EffectB_r);
	WriteJump(drawEffectChaos0LightParticle, drawEffectChaos0LightParticle_r);
	WriteJump(dispEffectChaos0AttackA, dispEffectChaos0AttackA_r);
	turnToPlayer_h.Hook(turnToPlayer_r);
	BossChaos0_h.Hook(BossChaos0_r);
	Bg_Chaos0_h.Hook(Bg_Chaos0_r);
	setApartTargetPos_h.Hook(setApartTargetPos_r);
	chaos0Pole_h.Hook(chaos0Pole_r);
	chaos0Punch_h.Hook(chaos0Punch_r);
	WriteJump((void*)0x54A360, chkChaos0Collision_r);
}

#ifdef MULTI_TEST
RegisterPatch patch_chaos0(patch_chaos0_init);
#endif
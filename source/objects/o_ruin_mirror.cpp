#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "VariableHook.hpp"
#include "multiplayer.h"
#include "splitscreen.h"
#include "camera.h"
#include "fog.h"

#define MIRROR_PNUM(twp) twp->btimer

static void __cdecl ObjectRuinFogSwitch_r(task* tp);
static void __cdecl DrawMirror_r(task* tp);
static void __cdecl ObjectRuinMirror_r(task* tp);
static void __cdecl BigMirrorDraw_r(task* tp);
static void __cdecl FogEnd_r(task* tp);
static void __cdecl ObjectRuinBigMirror_r(task* tp);
static void __cdecl ObjectRuinFogChange_r(task* tp);
static void __cdecl ObjectRuinFogLight_r(task* tp);
static void __cdecl DrawFogHasira_r(task* tp);

FastFunctionHookPtr<decltype(&ObjectRuinFogSwitch_r)> ObjectRuinFogSwitch_t(0x5E25A0, ObjectRuinFogSwitch_r);
FastFunctionHookPtr<decltype(&DrawMirror_r)> DrawMirror_t(0x5E2380, DrawMirror_r);
FastFunctionHookPtr<decltype(&ObjectRuinMirror_r)> ObjectRuinMirror_t(0x5E2850, ObjectRuinMirror_r);
FastFunctionHookPtr<decltype(&BigMirrorDraw_r)> BigMirrorDraw_t(0x5E2EA0, BigMirrorDraw_r);
FastFunctionHookPtr<decltype(&FogEnd_r)> FogEnd_t(0x5E2530, FogEnd_r);
FastFunctionHookPtr<decltype(&ObjectRuinBigMirror_r)> ObjectRuinBigMirror_t(0x5E3310, ObjectRuinBigMirror_r);
FastFunctionHookPtr<decltype(&ObjectRuinFogChange_r)> ObjectRuinFogChange_t(0x5E3020, ObjectRuinFogChange_r);
FastFunctionHookPtr<decltype(&ObjectRuinFogLight_r)> ObjectRuinFogLight_t(0x5E3240, ObjectRuinFogLight_r);
FastFunctionHookPtr<decltype(&DrawFogHasira_r)> DrawFogHasira_t(0x5E27A0, DrawFogHasira_r);

DataPointer(CCL_INFO, c_colli_mirror, 0x2038C38);
DataPointer(float, max_dist, 0x2038CA0);
DataPointer(NJS_POINT3, delete_point_a, 0x2038D04);

struct ruinfogwk
{
	float now_dist;
	int nocontimer;
	int name_flag;
	int other_flag;
	int discovery;
	int fog_switch;
	int ruin_m_flag;
	NJS_POINT3 aim_position;
};

static ruinfogwk ruinfogwp[PLAYER_MAX];

VariableHook<uint8_t, 0x2038C34> fog_switch_m;

static bool IsMirrorActive(int id)
{
	for (auto& i : ruinfogwp)
	{
		if (i.name_flag == id)
			return true;
	}
	return false;
}

#pragma region ObjectRuinFogSwitch
static void ObjectRuinFogSwitch_m(task* tp)
{
	if (!CheckRangeOut(tp))
	{
		auto twp = tp->twp;

		if (twp->mode)
		{
			if (twp->mode == 1i8)
			{
				if ((twp->cwp->flag & 1) != 0)
				{
					twp->cwp->flag &= ~1;

					auto pnum = TASKWK_PLAYERID(twp->cwp->hit_cwp->mytask->twp);
					if (pnum < PLAYER_MAX)
					{
						ruinfogwp[pnum].fog_switch = ruinfogwp[pnum].fog_switch != 1;
					}
				}

				EntryColliList(twp);
			}
			else if (twp->mode == 4i8)
			{
				FreeTask(tp);
			}
		}
		else
		{
			twp->btimer = 0;
			twp->wtimer = 0;
			CCL_Init(tp, (CCL_INFO*)0x2038CA4, 1, 4u);
			twp->cwp->info->a = (twp->scl.y + 1.0f) * 14.0f;
			twp->mode = 1i8;
		}
	}
}

static void __cdecl ObjectRuinFogSwitch_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectRuinFogSwitch_m(tp);
	}
	else
	{
		ObjectRuinFogSwitch_t.Original(tp);
	}
}
#pragma endregion

#pragma region DrawMirror
static void DrawMirror_m(task* tp)
{
	if (!loop_count)
	{
		auto twp = tp->twp;
		auto pnum = MIRROR_PNUM(twp);
		auto wk = &ruinfogwp[pnum];

		SetObjectTexture();
		njPushMatrixEx();
		njTranslateEx(&twp->pos);
		njRotateZ_(twp->ang.z);
		njRotateX_(twp->ang.x);
		njRotateY_(twp->ang.y);
		dsDrawObject((NJS_OBJECT*)0x2028BB8);
		if (wk->ruin_m_flag == 1)
		{
			auto cam_ang = GetCameraAngle(pnum);

			if (cam_ang)
			{
				njRotateZ_(-cam_ang->z);
				njRotateX_(-cam_ang->x);
			}
		}
		late_DrawModelClip((NJS_MODEL_SADX*)0x2028C98, LATE_MAT, 1.0f);
		njPopMatrixEx();
	}
}

static void __cdecl DrawMirror_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		DrawMirror_m(tp);
	}
	else
	{
		DrawMirror_t.Original(tp);
	}
}
#pragma endregion

#pragma region ObjectRuinMirror
enum : __int8
{
	MD_INIT,
	MD_1,
	MD_2,
	MD_3
};

static void ObjectRuinMirror_m(task* tp)
{
	if (!CheckRangeOutWithR(tp, 25000000.0f))
	{
		auto twp = tp->twp;
		auto pnum = MIRROR_PNUM(twp);
		auto wk = &ruinfogwp[pnum];

		switch (twp->mode)
		{
		case MD_INIT:
			CCL_Init(tp, &c_colli_mirror, 1, 4u);
			tp->disp = (TaskFuncPtr)0x5E2380;
			twp->timer.w[0] = 0i16;
			twp->timer.w[1] = 0i16;
			twp->mode = MD_1;
			break;
		case MD_1:
			if (twp->btimer == 2i8)
			{
				twp->timer.w[1] = 1i16;
			}

			if (wk->ruin_m_flag == 1)
			{
				auto aim_id = static_cast<int>(twp->scl.y);

				if (wk->other_flag == aim_id)
				{
					twp->btimer = 2i8;
					wk->aim_position.x = twp->pos.x;
					wk->aim_position.y = twp->pos.y;
					wk->aim_position.z = twp->pos.z;
				}

				if (wk->name_flag == aim_id && twp->timer.w[1] == 1i16)
				{
					NJS_POINT3 velo = { 0.0f, 0.0f, 0.0f };
					CreateSmoke(&twp->pos, &velo, 2.0f);
				}
			}

			if (twp->cwp->flag & 1)
			{
				twp->cwp->flag &= ~1;

				if (twp->smode != 1 && twp->cwp->hit_cwp->mytask)
				{
					auto hit_tp = twp->cwp->hit_cwp->mytask;

					if (hit_tp->twp)
					{
						pnum = TASKWK_PLAYERID(hit_tp->twp);
						twp->mode = MD_2;

						SetInputP(pnum, PL_OP_PLACEON);

						auto param = GetCamAnyParam(pnum);
						if (param)
						{
							param->camAnyParamPos.x = twp->pos.x;
							param->camAnyParamPos.y = twp->pos.y + 10.0f;
							param->camAnyParamPos.z = twp->pos.z;
							param->camAnyParamAng.x = 0;
							param->camAnyParamAng.y = twp->ang.y;
							param->camAnyParamAng.z = 0;
							SetFreeCameraMode_m(pnum, FALSE);
							CameraSetEventCamera_m(pnum, CAMMD_TAIHO, CAMADJ_NONE);
						}

						PClearSpeed(playermwp[pnum], playerpwp[pnum]);
						MIRROR_PNUM(twp) = pnum;
					}
				}
			}
			else
			{
				EntryColliList(twp);
			}
			break;
		case MD_2:
			wk->ruin_m_flag = 1;
			wk->name_flag = static_cast<int>(twp->scl.x);
			SetPositionP(pnum, twp->pos.x + njCos(twp->ang.y) * 10.0f, twp->pos.y, twp->pos.z - njSin(twp->ang.y) * 10.0f);

			{
				auto cam_rot = GetCameraAngle(pnum);
				twp->ang.y = cam_rot ? cam_rot->y + 0x8000 : 0;
			}

			if (wk->discovery == 1ui8)
			{
				twp->smode = 1i8;
				twp->mode = MD_1;
				wk->nocontimer = 120;
				wk->ruin_m_flag = 0;
				wk->discovery = 0;

				if (twp->scl.x == 5.0f)
				{
					max_dist = -175.0f;
				}

				CameraReleaseEventCamera_m(pnum);
				dsStop_num(197);
				SetInputP(pnum, PL_OP_LETITGO);
			}
			break;
		case MD_3:
			if (++twp->timer.w[0] > 20)
			{
				twp->timer.w[0] = 0i16;
				NJS_POINT3 velo = { 0.0f, 0.0f, 0.0f };
				CreateSmoke(&twp->pos, &velo, 2.0f);
			}
			break;
		}

		tp->disp(tp);
	}
}

static void __cdecl ObjectRuinMirror_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectRuinMirror_m(tp);
	}
	else
	{
		ObjectRuinMirror_t.Original(tp);
	}
}
#pragma endregion

#pragma region ObjectRuinBigMirror
static void HitMirror_m(taskwk* twp, ruinfogwk* wk, int pnum)
{
	const NJS_POINT3 pos = { twp->pos.x, twp->pos.y + 12.0f, twp->pos.z };
	NJS_POINT2 p2;
	njProjectScreen(0, &pos, &p2);

	auto ratio = SplitScreen::GetScreenRatio(pnum);
	float w = HorizontalStretch * ratio->w * 340.0f;
	float h = VerticalStretch * ratio->h * 240.0f;
	float dist = sqrtf((w - p2.x) * (w - p2.x) + (h - p2.y) * (h - p2.y));

	if (dist < 40.0f)
	{
		wk->other_flag = static_cast<int>(twp->scl.y);
		wk->fog_switch = 0;
		twp->smode = 1i8;
		wk->discovery = 1;
	}
}

static void __cdecl BigMirrorDraw_r(task* tp)
{
	// Exec moved to display because we need projection:
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		auto pnum = SplitScreen::GetCurrentScreenNum();
		auto ptwp = playertwp[pnum];
		auto wk = &ruinfogwp[pnum];

		if (ptwp && wk->ruin_m_flag == 1 && wk->name_flag == (int)twp->scl.y)
		{
			HitMirror_m(twp, wk, pnum);
		}
	}

	BigMirrorDraw_t.Original(tp);
}

static void ObjectRuinBigMirror_m(task* tp)
{
	if (!CheckRangeOutWithR(tp, 1000000.0f))
	{
		auto twp = tp->twp;

		if (twp->mode == 0i8)
		{
			tp->disp = (TaskFuncPtr)0x5E2EA0;
			twp->mode = 1i8;
		}
		else
		{
			// Exec moved to display because we need projection

			tp->disp(tp);
		}
	}
}

static void __cdecl ObjectRuinBigMirror_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectRuinBigMirror_m(tp);
	}
	else
	{
		ObjectRuinBigMirror_t.Original(tp);
	}
}
#pragma endregion

#pragma region ObjectRuinFogChange
static void __cdecl FogEnd_r(task* tp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto wk = &ruinfogwp[i];
		wk->now_dist = -110.0f;
		wk->aim_position = { 62.0f, 127.0f, -56.0f };
		wk->discovery = 0;
		wk->nocontimer = 0;
		wk->name_flag = 0;
		wk->other_flag = 0;
		wk->fog_switch = 1;
		wk->ruin_m_flag = 0;
		fog::ResetUserFog(i);
	}

	FogEnd_t.Original(tp);
}

static void GetClosestActiveMirror(NJS_POINT3* plpos, NJS_POINT3* aim_pos)
{
	float distance = 1000000.0f;
	for (auto& i : ruinfogwp)
	{
		float d = GetDistance(plpos, &i.aim_position);

		if (d < distance)
		{
			distance = d;
			aim_pos = &i.aim_position;
		}
	}
}

static void ChangeFogDensity_m(float dist, float density_speed, int num)
{
	auto wk = &ruinfogwp[num];

	if (dist <= wk->now_dist)
		wk->now_dist -= density_speed;
	else
		wk->now_dist += density_speed;

	if (wk->now_dist > max_dist)
	{
		wk->now_dist = max_dist;
	}

	___stcFog fog;
	fog.f32StartZ = 8.0f;
	fog.f32EndZ = 210.0f - wk->now_dist; // newer ver, 2004 version is: 410.0f - wk->now_dist * 2.5f
	fog.Col = 0xFF000000;
	fog.u8Enable = TRUE;
	fog::SetUserFog(num, &fog);
}

static void ObjectRuinFogChange_m(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 0i8)
	{
		// Set default values
		for (auto& i : ruinfogwp)
		{
			i.now_dist = -110.0f;
			i.aim_position = { 62.0f, 127.0f, -56.0f };
			i.discovery = 0;
			i.nocontimer = 0;
			i.name_flag = 0;
			i.other_flag = 0;
			i.fog_switch = 1;
			i.ruin_m_flag = 0;
		}

		max_dist = -110.0f;
		tp->dest = (TaskFuncPtr)0x5E2530;

		twp->timer.b[0] = 0;
		twp->timer.b[1] = 1;
		twp->wtimer = 1;
		twp->counter.w[1] = GetStageNumber();
		twp->mode = 1i8;
	}
	else
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto ptwp = playertwp[i];
			auto wk = &ruinfogwp[i];

			if (ptwp)
			{
				if (wk->nocontimer > 0 && !--wk->nocontimer)
				{
					SetFreeCameraMode_m(i, 1);
				}

				if (twp->counter.w[1] != GetStageNumber())
				{
					FreeTask(tp);
					return;
				}

				auto dist = GetDistance(&ptwp->pos, &delete_point_a) - 8.0f;

				if ((twp->timer.b[1] && dist <= 800.0f) || (!twp->timer.b[1] && dist <= 850.0f))
				{
					twp->timer.b[1] = 0i8;

					if (wk->fog_switch || wk->ruin_m_flag)
					{
						ChangeFogDensity_m(-70.0f, 10.0f, i);
					}
					else
					{
						NJS_POINT3 aim_pos = wk->aim_position;
						GetClosestActiveMirror(&ptwp->pos, &aim_pos);
						auto aim_dist = GetDistance(&ptwp->pos, &aim_pos);
						ChangeFogDensity_m(-aim_dist, 10.0f, i);
					}
				}
				else
				{
					twp->timer.b[1] = 1;
					fog::ResetUserFog(i);
				}
			}
		}
	}
}

static void __cdecl ObjectRuinFogChange_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectRuinFogChange_m(tp);
	}
	else
	{
		ObjectRuinFogChange_t.Original(tp);
	}
}
#pragma endregion

#pragma region ObjectRuinFogLight
static void __cdecl DrawFogLitht(task* tp)
{
	if (!loop_count)
	{
		auto twp = tp->twp;

		if (IsMirrorActive(static_cast<int>(twp->scl.y)))
		{
			SetObjectTexture();
			___njFogDisable();
			njPushMatrixEx();
			njTranslateEx(&twp->pos);
			njRotateZ_(twp->ang.z);
			njRotateX_(twp->ang.x);
			njRotateY_(twp->ang.y);
			late_DrawModelClip((NJS_MODEL_SADX*)0x20275AC, LATE_MAT, 1.0f);
			njPopMatrixEx();
			___njFogEnable();
		}
	}
}

static void __cdecl ObjectRuinFogLight_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (CheckRangeOut(tp))
		{
			if (!tp->disp)
				tp->disp = DrawFogLitht;
			else
				tp->disp(tp);
		}
	}
	else
	{
		ObjectRuinFogLight_t.Original(tp);
	}
}
#pragma endregion

#pragma region DrawFogHasira
static void DrawFogHasira_m(task* tp)
{
	if (!loop_count)
	{
		auto twp = tp->twp;

		if (IsMirrorActive(static_cast<int>(twp->scl.y)))
			___njFogDisable();

		SetObjectTexture();
		njPushMatrixEx();
		njTranslateEx(&twp->pos);
		njRotateZ_(twp->ang.z);
		njRotateX_(twp->ang.x);
		njRotateY_(twp->ang.y);
		ds_DrawModelClip((NJS_MODEL_SADX*)0x20062E0, 1.0f);
		njPopMatrixEx();

		if (!gu8FogEnbale)
			___njFogEnable();
	}
}

static void __cdecl DrawFogHasira_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		DrawFogHasira_m(tp);
	}
	else
	{
		DrawFogHasira_t.Original(tp);
	}
}
#pragma endregion
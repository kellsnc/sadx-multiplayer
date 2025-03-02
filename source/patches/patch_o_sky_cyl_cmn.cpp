#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "sadx_utils.h"
#include "patch_player.h"
#include "patch_o_sky_cyl_cmn.h"

UsercallFunc(Bool, sub_5EDD60, (task* tp, Sint32 pnum), (tp, pnum), 0x5EDD60, rEAX, rEAX, stack4);
FastFunctionHook<void, task*, Angle> dsHangPoleCom_h(0x5EDDE0);

signed int SetCylinderNextAction(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (twp->mode >= MD_MULTI_S6A1_WAIT && twp->mode <= MD_MULTI_S6A1_RROT)
	{
		twp->smode = 0;
		return 0;
	}
	else
	{
		if (pwp->htp->twp->pos.y + pwp->htp->twp->cwp->info->center.y - pwp->htp->twp->cwp->info->b >= twp->pos.y)
		{
			twp->mode = MD_MULTI_S6A1_WAIT;
		}
		else
		{
			twp->mode = MD_MULTI_S6A1_SLID;
		}

		PClearSpeed(mwp, pwp);
		twp->flag &= 0xDAu;
		pwp->free.sw[3] = 0;
	}

	return 1;
}

void HoldOnPillar(playerwk* pwp, taskwk* twp)
{
	float v5 = 0.0f;
	int v6 = 0;
	float v7 = 0.0f;

	auto htp = pwp->htp;

	if (htp)
	{
		auto htpData = htp->twp;
		auto mwpData = htp->mwp;

		if (htpData)
		{
			if (mwpData)
			{
				v7 = mwpData->rad;
			}
			else
			{
				v7 = 30.0;
			}
			v5 = njCos(twp->ang.y);
			v6 = twp->ang.y;
			twp->pos.x = htpData->pos.x - v5 * v7;
			twp->pos.z = htpData->pos.z - njSin(v6) * v7;
		}
	}
	else
	{
		twp->mode = 1;
	}
}

//Functions used in character main
void Mode_SDCylinderStd(taskwk* twp, playerwk* pwp)
{
	HoldOnPillar(pwp, twp);
}

void Mode_SDCylinderDown(taskwk* twp, playerwk* pwp)
{
	auto v18 = twp->pos.y - 0.5f;
	auto v19 = pwp->htp->twp;
	twp->pos.y = v18;

	auto v20 = v19->cwp->info->center.y + v19->pos.y - v19->cwp->info->b;
	if (v20 >= v18)
	{
		twp->pos.y = v20;
	}

	HoldOnPillar(pwp, twp);
}

void Mode_SDCylinderLeft(taskwk* twp, playerwk* pwp)
{
	twp->ang.y += SonicGetPillarRotSpeed(pwp);
	HoldOnPillar(pwp, twp);
}

void Mode_SDCylinderRight(taskwk* twp, playerwk* pwp)
{
	twp->ang.y -= SonicGetPillarRotSpeed(pwp);
	HoldOnPillar(pwp, twp);
}

//Functions used in character run actions (chk_mode)
void Mode_MD_MULTI_S6A1_WAITChanges(taskwk* twp, playerwk* pwp)
{
	if (twp->mode < MD_MULTI_S6A1_WAIT || twp->mode > MD_MULTI_S6A1_RROT)
	{
		pwp->htp = 0;
		return;
	}

	if (!GetAnalog((EntityData1*)twp, 0, 0))
	{
		return;
	}

	auto controllerAng = Controllers[(unsigned __int8)twp->counter.b[0]].LeftStickX << 8;

	if (controllerAng >= -3072)
	{
		twp->mode = MD_MULTI_S6A1_RROT;
	}
	else if (controllerAng <= 3072)
	{
		twp->mode = MD_MULTI_S6A1_LROT;
	}

	return;
}

void Mode_MD_MULTI_S6A1_SLIDChanges(taskwk* twp, playerwk* pwp)
{
	auto htpTsk = pwp->htp;

	if (!htpTsk)
	{
		twp->mode = 1;
		pwp->mj.reqaction = 0;
		return;
	}

	auto htpData = htpTsk->twp;

	if (htpData)
	{
		auto htpCol = htpData->cwp;

		if (htpCol)
		{
			auto math = htpCol->info->center.y + htpData->pos.y - htpCol->info->b;

			if (math < twp->pos.y)
			{
				if (twp->mode < MD_MULTI_S6A1_WAIT || twp->mode > MD_MULTI_S6A1_RROT)
				{
					pwp->htp = 0;
				}
			}
			else
			{
				twp->mode = MD_MULTI_S6A1_WAIT;
			}
		}
	}
	else
	{
		twp->mode = 1;
		pwp->mj.reqaction = 0;
	}

	return;
}

void __cdecl dsHangPoleCom_r(task* tp, Angle rotspd)
{
	if (!multiplayer::IsActive())
	{
		dsHangPoleCom_h.Original(tp, rotspd);
		return;
	}

	taskwk* twp = tp->twp;
	int cdt[PLAYER_MAX] = { 0 };

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		taskwk* pltwp = playertwp[i];
		if (pltwp)
		{
			cdt[i] = 0;

			if (--twp->counter.b[i] == 0)
				twp->counter.b[i] = 1;

			if (sub_5EDD60(tp, i) && twp->counter.b[i] == 1)
			{
				cdt[i] = -1;

				NJS_VECTOR dir;
				dir.x = pltwp->pos.x - twp->pos.x;
				dir.y = 0.0f;
				dir.z = pltwp->pos.z - twp->pos.z;
				njUnitVector(&dir);

				motionwk2* mwp = playermwp[i];
				NJS_VECTOR spd;
				spd.x = mwp->spd.x;
				spd.y = 0.0f;
				spd.z = mwp->spd.z;
				njUnitVector(&spd);
				if (njInnerProduct(&spd, &dir) < 0.5f)
				{
					cdt[i] = 1;
				}
			}
		}
	}

	EntryColliList(twp);
	ObjectSetupInput(twp, 0);

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		taskwk* pltwp = playertwp[i];
		if (pltwp)
		{
			if (twp->value.l & (1 << i))
			{
				NJS_VECTOR dir;
				dir.x = pltwp->pos.x - twp->pos.x;
				dir.y = 0.0f;
				dir.z = pltwp->pos.z - twp->pos.z;
				Float dist = njSqrt(dir.z * dir.z + dir.x * dir.x);
				Angle angY = twp->ang.x - -njArcTan2(-dir.x, -dir.z);
				SetRotationP(i, 0, angY, 0);
				SetPositionP(i, njCos(-0x4000 - angY) * dist + twp->pos.x, pltwp->pos.y, njSin(-0x4000 - angY) * dist + twp->pos.z);
			}

			if (!cdt[i] || playerpwp[i]->htp != tp)
			{
				if (twp->value.l & (1 << i))
				{
					twp->value.l &= ~(1 << i);
					twp->counter.b[i] = 60;

					NJS_VECTOR dir;
					dir.x = pltwp->pos.x - twp->pos.x;
					dir.y = 0.0f;
					dir.z = pltwp->pos.z - twp->pos.z;
					njUnitVector(&dir);

					Angle angY = njArcTan2(-dir.x, -dir.z);
					SetRotationP(i, 0, angY, 0);
					SetVelocityP(i, njCos(-0x4000 - angY) * 1.2f, 2.2f, njSin(-0x4000 - angY) * 1.2f);
				}
			}

			if (cdt[i] == 1 && twp->counter.b[i] == 1 && !(twp->value.l & (1 << i)))
			{
				HoldOnPillarP(i, tp);

				NJS_VECTOR dir;
				dir.x = pltwp->pos.x - twp->pos.x;
				dir.y = 0.0f;
				dir.z = pltwp->pos.z - twp->pos.z;
				njUnitVector(&dir);
				SetRotationP(i, 0, njArcTan2(-dir.x, -dir.z), 0);

				twp->value.l |= 1 << i;
			}
		}
	}
}

void patch_sky_cyl_cmn_init()
{
	dsHangPoleCom_h.Hook(dsHangPoleCom_r);
}

RegisterPatch patch_sky_cyl_cmn(patch_sky_cyl_cmn_init);
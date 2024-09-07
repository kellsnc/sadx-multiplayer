#include "pch.h"
#include "SADXModLoader.h"
#include "FunctionHook.h"
#include "sadx_utils.h"

signed int SetCylinderNextAction(taskwk* data, motionwk2* data2, playerwk* co2)
{
	if (data->mode >= SDCylStd && data->mode <= SDCylRight)
	{
		data->smode = 0;
		return 0;
	}
	else
	{
		if (co2->htp->twp->pos.y + co2->htp->twp->cwp->info->center.y - co2->htp->twp->cwp->info->b >= data->pos.y)
		{
			data->mode = SDCylStd;
		}
		else
		{
			data->mode = SDCylDown;
		}

		PClearSpeed(data2, co2);
		data->flag &= 0xDAu;
		co2->free.sw[3] = 0;
	}

	return 1;
}

void HoldOnPillar(playerwk* co2, taskwk* data)
{
	float v5 = 0.0f;
	int v6 = 0;
	float v7 = 0.0f;

	auto htp = co2->htp;

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
			v5 = njCos(data->ang.y);
			v6 = data->ang.y;
			data->pos.x = htpData->pos.x - v5 * v7;
			data->pos.z = htpData->pos.z - njSin(v6) * v7;
		}
	}
	else
	{
		data->mode = 1;
	}
}

//Functions used in character main
void Mode_SDCylinderStd(taskwk* data, playerwk* co2)
{
	HoldOnPillar(co2, data);
}

void Mode_SDCylinderDown(taskwk* data, playerwk* co2)
{
	auto v18 = data->pos.y - 0.5f;
	auto v19 = co2->htp->twp;
	data->pos.y = v18;

	auto v20 = v19->cwp->info->center.y + v19->pos.y - v19->cwp->info->b;
	if (v20 >= v18)
	{
		data->pos.y = v20;
	}

	HoldOnPillar(co2, data);
}

void Mode_SDCylinderLeft(taskwk* data, playerwk* co2)
{
	data->ang.y += SonicGetPillarRotSpeed(co2);
	HoldOnPillar(co2, data);
}

void Mode_SDCylinderRight(taskwk* data, playerwk* co2)
{
	data->ang.y -= SonicGetPillarRotSpeed(co2);
	HoldOnPillar(co2, data);
}

//Functions used in character run actions (chk_mode)
void Mode_SDCylStdChanges(taskwk* data1, playerwk* co2)
{
	if (data1->mode < SDCylStd || data1->mode > SDCylRight)
	{
		co2->htp = 0;
		return;
	}

	if (!GetAnalog((EntityData1*)data1, 0, 0))
	{
		return;
	}

	auto controllerAng = Controllers[(unsigned __int8)data1->counter.b[0]].LeftStickX << 8;

	if (controllerAng >= -3072)
	{
		data1->mode = SDCylRight;
	}
	else if (controllerAng <= 3072)
	{
		data1->mode = SDCylLeft;
	}

	return;
}

void Mode_SDCylDownChanges(taskwk* data1, playerwk* co2)
{
	auto htpTsk = co2->htp;

	if (!htpTsk)
	{
		data1->mode = 1;
		co2->mj.reqaction = 0;
		return;
	}

	auto htpData = htpTsk->twp;

	if (htpData)
	{
		auto htpCol = htpData->cwp;

		if (htpCol)
		{
			auto math = htpCol->info->center.y + htpData->pos.y - htpCol->info->b;

			if (math < data1->pos.y)
			{
				if (data1->mode < SDCylStd || data1->mode > SDCylRight)
				{
					co2->htp = 0;
				}
			}
			else
			{
				data1->mode = SDCylStd;
			}
		}
	}
	else
	{
		data1->mode = 1;
		co2->mj.reqaction = 0;
	}

	return;
}

UsercallFunc(BOOL, sub_5EDD60, (task* tp, Sint32 pnum), (tp, pnum), 0x5EDD60, rEAX, rEAX, stack4);

void __cdecl dsHangPoleCom_r(task* tp, Angle rotspd);
FunctionHook<void, task*, Angle> dsHangPoleCom_h(0x5EDDE0, dsHangPoleCom_r);
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

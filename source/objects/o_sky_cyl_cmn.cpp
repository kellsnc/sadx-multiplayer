#include "pch.h"

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

void Mode_SDCylinderStd(taskwk* data, playerwk* co2)
{
	SonicHoldOnPillar(co2, data);
}

void Mode_SDCylinderDown(taskwk* data, playerwk* co2)
{
	auto v30 = data->pos.y - 0.5f;
	auto v31 = co2->htp->twp;
	data->pos.y = v30;
	auto v32 = v31->cwp->info->center.y + v31->pos.y - v31->cwp->info->b;

	if (v32 >= v30)
	{
		data->pos.y = v32;
	}

	SonicHoldOnPillar(co2, data);
}

void Mode_SDCylinderLeftRight(taskwk* data, playerwk* co2)
{
	auto v33 = SonicGetPillarRotSpeed(co2) + data->ang.y;
	data->ang.y = v33;
	SonicHoldOnPillar(co2, data);
}


void init_SDCylinderPatches()
{

}


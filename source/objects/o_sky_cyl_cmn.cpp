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

void init_SDCylinderPatches()
{

}


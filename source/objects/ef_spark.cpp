#include "pch.h"

DataArray(NJS_POINT3, l_point, 0x3C5C484, 2);
DataPointer(NJS_POINT3COL, spark_point3col, 0x9894C0);

static void EffectSparkDisplay(task* tp)
{
	if (!MissedFrames)
	{
		auto twp = tp->twp;

		if (dsCheckViewV(&twp->pos, 5.0))
		{
			twp->scl.z = twp->scl.z * 0.975f;
			twp->scl.y = (twp->scl.y * 0.975f) - 0.085f;
			twp->scl.x = twp->scl.x * 0.975f;
			twp->pos.x += twp->scl.x;
			twp->pos.y += twp->scl.y;
			twp->pos.z += twp->scl.z;

			auto scl = sqrtf(((twp->scl.x * twp->scl.x) + ((twp->scl.z * twp->scl.z) + (twp->scl.y * twp->scl.y))));
			l_point[0].x = twp->pos.x + (((1.0f / scl) * twp->scl.x) * 0.3f);
			l_point[0].y = twp->pos.y + (((1.0f / scl) * twp->scl.y) * 0.3f);
			l_point[0].z = twp->pos.z + (((1.0f / scl) * twp->scl.z) * 0.3f);
			l_point[1].x = twp->pos.x - (((1.0f / scl) * twp->scl.x) * 0.3f);
			l_point[1].y = twp->pos.y - (((1.0f / scl) * twp->scl.y) * 0.3f);
			l_point[1].z = twp->pos.z - (((1.0f / scl) * twp->scl.z) * 0.3f);

			njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
			njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_ONE);
			njDrawLine3D(&spark_point3col, 1, 0);
			ghDefaultBlendingMode();
		}
	}
}

void CreateSpark_m(NJS_POINT3* pos, NJS_POINT3* velo)
{
	auto tp = CreateElementalTask(2u, 6, EffectSpark);
	if (tp)
	{
		tp->disp = EffectSparkDisplay;

		auto twp = tp->twp;
		twp->wtimer = 0;
		twp->pos.x = pos->x;
		twp->pos.y = pos->y;
		twp->pos.z = pos->z;
		twp->scl.x = velo->x;
		twp->scl.y = velo->y;
		twp->scl.z = velo->z;
	}
}

void PatchEffectSpark()
{
	WriteJump((void*)0x4CEA00, CreateSpark_m);
}
#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "splitscreen.h"
#include "camera.h"

FastFunctionHook<void> ListGroundForDrawing_t(0x43A900);

void __cdecl ListGroundForDrawing_r()
{
	if (SplitScreen::IsActive())
	{
		auto cam_pos = GetCameraPosition(SplitScreen::GetCurrentScreenNum());
		auto cam_ang = GetCameraAngle(SplitScreen::GetCurrentScreenNum());

		if (!cam_pos || !cam_ang)
		{
			return;
		}

		numDisplayEntry = 0;

		NJS_POINT3 center = { 0.0f, 0.0f, MaxDrawDistance * -0.5f };

		njPushMatrix(_nj_unit_matrix_);
		njRotateY_(cam_ang->y);
		njRotateX_(cam_ang->x);
		njCalcPoint(0, &center, &center);
		njPopMatrixEx();

		center.x += cam_pos->x;
		center.y += cam_pos->y;
		center.z += cam_pos->z;

		for (int i = 0; i < pObjLandTable->ssCount; ++i)
		{
			auto col = &pObjLandTable->pLandEntry[i];

			if (!col->blockbit || col->blockbit & MaskBlock)
			{
				if (col->slAttribute & ColFlags_Visible)
				{
					if (col->slAttribute & ColFlags_UseSkyDrawDist || GetDistance(&center, (NJS_POINT3*)&col->xCenter) < col->xWidth + MaxDrawDistance)
					{
						pDisplayEntry[numDisplayEntry++] = col;
					}
				}
			}
		}
	}
	else
	{
		ListGroundForDrawing_t.Original();
	}
}

void patch_land_init()
{
	ListGroundForDrawing_t.Hook(ListGroundForDrawing_r);
}

RegisterPatch patch_land(patch_land_init);
#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "splitscreen.h"
#include "camera.h"

FastFunctionHook<void> ListGroundForDrawing_h(0x43A900);

void __cdecl ListGroundForDrawing_r()
{
	if (splitscreen::IsActive())
	{
		auto cam_pos = GetCameraPosition(splitscreen::GetCurrentScreenNum());
		auto cam_ang = GetCameraAngle(splitscreen::GetCurrentScreenNum());

		if (!cam_pos || !cam_ang)
		{
			return;
		}

		numDisplayEntry = 0;

		NJS_POINT3 center = { 0.0f, 0.0f, MaxDrawDistance * -0.5f };

		njPushMatrix(_nj_unit_matrix_);
		ROTATEY(0, cam_ang->y);
		ROTATEX(0, cam_ang->x);
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
		ListGroundForDrawing_h.Original();
	}
}

void patch_land_init()
{
	ListGroundForDrawing_h.Hook(ListGroundForDrawing_r);
}

RegisterPatch patch_land(patch_land_init);
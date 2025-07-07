#include "pch.h"
#include <teleport.h>


FastUsercallHookPtr<Sint32(*)(taskwk* twp, Sint32 a2), rEAX, rESI, stack4> chk_getin_h(0x529520);
static Sint32 chk_getin(taskwk* twp, Sint32 a2)
{
	if (multiplayer::IsActive() == false)
	{
		return 	chk_getin_h.Original(twp, a2);
	}

	auto id = GetTheNearestPlayerNumber(&twp->pos);
	Sint32 res = 0;
	auto player = playertwp[id];
	if (player)
	{

		NJS_POINT3 a3;
		njPushMatrix(0);
		njUnitMatrix(0);
		njTranslateV(0, &twp->pos);
		njRotateXYZ(0, twp->ang.x, twp->ang.y, twp->ang.z);
		njInvertMatrix(0);
		njCalcPoint(0, &player->pos, &a3);
		if (a3.x <= -20.0f || a3.x >= 20.0f)
		{
			njPopMatrix(1u);
			return res;
		}


		if (a2)
		{
			if (a3.y <= -45.60f || a3.y >= -30.0f)
			{
				njPopMatrix(1u);
				return res;
			}
		}
		else if (a3.y > -45.30f)
		{
			if (a3.y >= -30.0f)
			{
				njPopMatrix(1u);
				return 0;
			}
		}

		if (a3.z > -10.0f && a3.z < 6.0f)
		{
			res = 1;
		}

		njPopMatrix(1u);


		if (res)
		{
			NJS_POINT3 pPos = player->pos;
			pPos.x += 5.0f;
			TeleportPlayers(&pPos);
		}
	}
	return res;
}


void patch_ec_monorail_init()
{
	chk_getin_h.Hook(chk_getin);
}

RegisterPatch patch_ec_monorail(patch_ec_monorail_init);

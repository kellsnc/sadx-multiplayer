#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

FastUsercallHookPtr<Bool(*)(taskwk* twp), rEAX, rESI> CollisionSceneChangerSS_CheckCollision_h(0x640550);

Bool __cdecl CollisionSceneChangerSS_CheckCollision_r(taskwk* twp)
{
	if (!multiplayer::IsActive())
	{
		return CollisionSceneChangerSS_CheckCollision_h.Original(twp);
	}

	NJS_POINT3 ppos;

	int pnum = GetTheNearestPlayerNumber(&twp->pos);
	GetPlayerPosition(pnum, 0, &ppos, 0);

	Float x = ppos.x - twp->pos.x;
	Float z = ppos.z - twp->pos.z;

	if (fabs(ppos.y - (twp->pos.y + twp->timer.f) + 10.0f) <= twp->timer.f)
	{
		Float sin = njSin(-twp->ang.y);
		Float cos = njCos(-twp->ang.y);
		if (fabs(sin * z + cos * x) < twp->counter.f &&
			fabs(cos * z - sin * x) < twp->value.f)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void patch_c_ss_scenechanger_init()
{
	CollisionSceneChangerSS_CheckCollision_h.Hook(CollisionSceneChangerSS_CheckCollision_r);
}

RegisterPatch patch_c_ss_scenechanger(patch_c_ss_scenechanger_init);
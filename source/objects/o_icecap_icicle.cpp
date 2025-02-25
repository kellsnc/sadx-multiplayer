#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"

static void ObjectIcicleExecute_m(task* tp)
{
	if (!CheckRangeOut(tp))
	{
		auto twp = tp->twp;
		auto player = CCL_IsHitPlayer(twp);

		if (player)
		{
			if (twp->cwp->my_num == 3)
			{
				auto pnum = TASKWK_PLAYERID(player);

				if (playermwp[pnum]->spd.y < 0.0f && playerpwp[pnum]->jumptimer > 5)
				{
					NJS_VECTOR v;
					v.x = player->pos.x - twp->pos.x;
					v.y = player->pos.y - twp->pos.y;
					v.z = player->pos.z - twp->pos.z;

					if (njScalor(&v) < 1.0f || njInnerProduct(&playermwp[pnum]->spd, &v) < 0.5f)
					{
						twp->btimer = pnum + 1;
						HoldOnIcicleP(pnum, tp);
						SetRotationP(pnum, twp->ang.x, njArcTan2(-v.x, -v.z), twp->ang.z);
					}
				}
			}
		}

		if (!twp->btimer || playerpwp[twp->btimer - 1]->htp != tp)
		{
			twp->btimer = 0;
			EntryColliList(twp);
			ObjectSetupInput(twp, nullptr);
		}

		tp->disp(tp);
	}
}

static void __cdecl ObjectIcicleExecute_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectIcicleExecute_r)> ObjectIcicleExecute_t(0x4F4C20, ObjectIcicleExecute_r);
static void __cdecl ObjectIcicleExecute_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectIcicleExecute_m(tp);
	}
	else
	{
		ObjectIcicleExecute_t.Original(tp);
	}
}
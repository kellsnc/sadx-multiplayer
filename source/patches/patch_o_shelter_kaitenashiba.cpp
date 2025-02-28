#include "pch.h"
#include "multiplayer.h"

enum : __int8
{
	MODE_NORMAL,
	MODE_END
};

FunctionPointer(void, GetKaitenKeyCount, (int no, Angle* r, Angle* l), 0x5A27F0);

static void __cdecl execShelterKaitenashiba_r(task* tp); // "Exec"
FastFunctionHookPtr<decltype(&execShelterKaitenashiba_r)> execShelterKaitenashiba_t(0x59CB40);

static void execShelterKaitenashiba_m(task* tp)
{
	if (!CheckRangeOut(tp))
	{
		auto twp = tp->twp;
		auto mwp = tp->mwp;

		LoopTaskC(tp);

		if (twp->mode == MODE_NORMAL)
		{
			Angle r, l;
			GetKaitenKeyCount(mwp->work.l, &r, &l);

			if (l > twp->value.l)
			{
				twp->value.l += 1;
				mwp->weight += mwp->height;

				if (mwp->weight > mwp->rad)
				{
					mwp->weight = mwp->rad;
				}
			}

			if (r > twp->timer.l)
			{
				twp->timer.l += 1;

				mwp->weight -= mwp->height;

				if (mwp->weight < mwp->spd.y)
				{
					mwp->weight = mwp->spd.y;
				}
			}

			float new_pos;
			if (twp->pos.y < mwp->weight)
			{
				new_pos = twp->pos.y + 0.5f;

				if (new_pos > mwp->weight)
				{
					new_pos = mwp->weight;
				}
			}
			else
			{
				new_pos = twp->pos.y - 0.75f;

				if (new_pos < mwp->weight)
				{
					new_pos = mwp->weight;
				}
			}

			if (new_pos != twp->pos.y)
			{
				((NJS_OBJECT*)twp->counter.ptr)->pos[1] = new_pos;
				tp->ctp->twp->pos.y = new_pos + 5.0f;

				for (int i = 0; i < PLAYER_MAX; ++i)
				{
					if (CheckPlayerRideOnMobileLandObjectP(i, tp))
					{
						NJS_POINT3 v;
						GetPlayerPosition(i, 0, &v, 0);
						v.y += new_pos - twp->pos.y;
						PositionPlayer(i, v.x, v.y, v.z);
					}
				}

				twp->pos.y = new_pos;
			}
		}

		MirenObjCheckCollisionP(twp, 60.0f);

		tp->disp(tp);
	}
}

static void __cdecl execShelterKaitenashiba_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		execShelterKaitenashiba_m(tp);
	}
	else
	{
		execShelterKaitenashiba_t.Original(tp);
	}
}

void patch_shelter_kaitenashiba_init()
{
	execShelterKaitenashiba_t.Hook(execShelterKaitenashiba_r);
}

RegisterPatch patch_shelter_kaitenashiba(patch_shelter_kaitenashiba_init);
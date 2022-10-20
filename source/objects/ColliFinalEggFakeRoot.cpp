#include "pch.h"
#include "multiplayer.h"

DataPointer(int, fakeroot_correct, 0x3C7457C);

static void ColliFinalEggFakeRoot_m(task* tp)
{
	if (!CheckRangeOut(tp))
	{
		auto twp = tp->twp;

		if (twp->mode)
		{
			if (fakeroot_correct == twp->btimer)
			{
				auto ptwp = CCL_IsHitPlayer(twp);

				if (ptwp)
				{
					SetPositionP(TASKWK_PLAYERID(ptwp), 2033.0f, 5239.0f, -2330.0f);
					ptwp->ang.y = 0x4000;
				}
			}
		}
		else
		{
			twp->btimer = static_cast<char>(twp->ang.x);
			twp->ang.x = 0;
			twp->ang.z = 0;
			CCL_Init(tp, (CCL_INFO*)0x1AC41A8, 1, 4u);
			twp->mode = 1i8;
		}

		EntryColliList(twp);
	}
}

static void __cdecl ColliFinalEggFakeRoot_r(task* tp);
Trampoline ColliFinalEggFakeRoot_t(0x5B07A0, 0x5B07A8, ColliFinalEggFakeRoot_r);
static void __cdecl ColliFinalEggFakeRoot_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ColliFinalEggFakeRoot_m(tp);
	}
	else
	{
		TARGET_STATIC(ColliFinalEggFakeRoot)(tp);
	}
}
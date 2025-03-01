#include "pch.h"
#include "multiplayer.h"

DataPointer(int, fakeroot_correct, 0x3C7457C);

static void __cdecl ColliFinalEggFakeRoot_r(task* tp);
static void __cdecl ObjectFinalEggRainbowDoor_r(task* tp);

FastFunctionHookPtr<decltype(&ColliFinalEggFakeRoot_r)> ColliFinalEggFakeRoot_h(0x5B07A0);
FastFunctionHookPtr<decltype(&ObjectFinalEggRainbowDoor_r)> ObjectFinalEggRainbowDoor_h(0x5B0910);

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

static void __cdecl ColliFinalEggFakeRoot_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ColliFinalEggFakeRoot_m(tp);
	}
	else
	{
		ColliFinalEggFakeRoot_h.Original(tp);
	}
}

static void __cdecl ObjectFinalEggRainbowDoor_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 1)
	{
		auto twp = tp->twp;

		if (IsPlayerInSphere(&twp->pos, 40.0f))
		{
			twp->mode = 4;
			dsPlay_oneshot_Dolby(283, 0, 0, 0, 120, twp);
		}

		EntryColliList(twp);
		tp->disp(tp);
	}
	else
	{
		ObjectFinalEggRainbowDoor_h.Original(tp);
	}
}

void patch_finalegg_rainbowdoor_init()
{
	ColliFinalEggFakeRoot_h.Hook(ColliFinalEggFakeRoot_r);
	ObjectFinalEggRainbowDoor_h.Hook(ObjectFinalEggRainbowDoor_r);
}

RegisterPatch patch_finalegg_rainbowdoor(patch_finalegg_rainbowdoor_init);
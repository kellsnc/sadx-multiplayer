#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "multiplayer.h"

FastFunctionHook<void, task*> Bg_MysticRuin_h(0x530670);

void __cdecl Bg_MysticRuin_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		Bg_MysticRuin_h.Original(tp);
		return;
	}

	auto twp = tp->twp;

	switch (twp->mode)
	{
	case 0:
		SkyBox_MysticRuins_TimeOfDayLightDirection((ObjectMaster*)tp);
		twp->mode++;
		break;
	case 1:
		tp->disp = (TaskFuncPtr)SkyBox_MysticRuins_Display;
		twp->mode++;
		break;
	case 2:
		if (ssActNumber == twp->btimer)
		{
			if (twp->btimer == 2)
			{
				GetTimeOfDay();
			}

			tp->disp(tp);
		}
		else
		{
			twp->mode = 3;
		}
		break;
	case 3:
		SkyBox_MysticRuins_TimeOfDayLightDirection((ObjectMaster*)tp);
		twp->mode = 2;
		break;
	default:
		return;
	}
}

void patch_rd_mysticruin_init()
{
	Bg_MysticRuin_h.Hook(Bg_MysticRuin_r);
}

RegisterPatch patch_rd_mysticruin(patch_rd_mysticruin_init);
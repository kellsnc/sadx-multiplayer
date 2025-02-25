#include "pch.h"
#include "multiplayer.h"
#include "gravity.h"

// Gravity panels in Lost World

static auto CommonExec = GenerateUsercallWrapper<TaskFuncPtr>(noret, 0x5E8B40, rEAX); // custom name
static auto PanelExecMove = GenerateUsercallWrapper<TaskFuncPtr>(noret, 0x5E8E70, rEAX); // custom name
static auto PanelExecBlink = GenerateUsercallWrapper<TaskFuncPtr>(noret, 0x5E8F90, rEAX); // custom name

DataPointer(uint8_t, gMode, 0x3C7EDE4);
DataPointer(uint8_t, gState, 0x3C7EDE5);

static void PanelExec_m(task* tp)
{
	if (!CheckRangeOut(tp))
	{
		auto twp = tp->twp;

		CommonExec(tp);

		if (gMode)
		{
			if (gMode == 1)
			{
				PanelExecMove(tp);
			}
		}
		else
		{
			PanelExecBlink(tp);
		}

		if (((1 << gState) & twp->wtimer))
		{
			twp->btimer |= 0x1;

			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				auto ptwp = playertwp[i];

				if (ptwp)
				{
					auto cos = njCos(twp->ang.y);
					auto sin = njSin(twp->ang.y);

					auto zdiff = ptwp->pos.z - twp->scl.z;
					auto ydiff = ptwp->pos.y - twp->scl.y;
					auto xdiff = ptwp->pos.x - twp->scl.x;

					auto x = xdiff * cos - zdiff * sin;
					auto z = xdiff * sin + zdiff * cos;

					if (x > -25.0f && x < 25.0f && ydiff > -25.0f && ydiff < 25.0f && z > 4.0f && z < 15.0f)
					{
						gravity::SetUserGravity(twp->ang.y + 0x4000, 0x4000, i);
					}
				}
			}
		}
		else
		{
			twp->btimer &= ~0x1;
		}

		auto object = (NJS_OBJECT*)twp->value.ptr;
		object->pos[0] = twp->scl.x;
		object->pos[1] = twp->scl.y;
		object->pos[2] = twp->scl.z;

		tp->disp(tp);
	}
}

static void __cdecl PanelExec_r(task* tp);
FastFunctionHookPtr<decltype(&PanelExec_r)> PanelExec_t(0x5E92A0, PanelExec_r);
static void __cdecl PanelExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		PanelExec_m(tp);
	}
	else
	{
		PanelExec_t.Original(tp);
	}
}
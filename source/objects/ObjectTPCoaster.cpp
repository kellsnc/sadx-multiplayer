#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"

TaskFunc(dispTPCoaster, 0x5F45C0);

enum {
	MD_INIT,
	MD_WAIT,
	MD_WARP,
	MD_MOVE,
	MD_OUT,
	MD_STOP
};

void __cdecl execTPCoaster_r(task* tp);
FastFunctionHook<void, task*> execTPCoaster_h(0x61D6E0, execTPCoaster_r);
void __cdecl execTPCoaster_r(task* tp)
{
	taskwk* twp = tp->twp;

	auto id = twp->value.b[0];
	taskwk* player = playertwp[id];

	switch (twp->mode)
	{
	case MD_INIT:
		execTPCoaster_h.Original(tp); // In case a mod edits the track / model
		twp->mode = MD_WAIT;
		break;
	case MD_WAIT:
		if (tp->ptp->twp->mode != 2)
		{
			break;
		}

		twp->mode = MD_WARP;

		if (player)
		{
			twp->scl.x = (twp->pos.x - player->pos.x) * 0.05f;
			twp->scl.y = (twp->pos.y - player->pos.y) * 0.05f;
			twp->scl.z = (twp->pos.z - player->pos.z) * 0.05f;
			SetInputP(id, PL_OP_PLACEWITHFALL);
		}

		break;
	case MD_WARP:
		if (tp->ptp->twp->mode == 3)
		{
			twp->mode = MD_MOVE;
			break;
		}

		if (player)
		{
			if (++twp->wtimer <= 0x14ui16)
			{
				player->pos.x += twp->scl.x;
				player->pos.y += twp->scl.y;
				player->pos.z += twp->scl.z;
			}
			else
			{
				SetInputP(id, PL_OP_PLACEWITHCART);

				switch (TASKWK_CHARID(player))
				{
				case Characters_Sonic:
					tp->ptp->twp->btimer |= 1u;
					break;
				case Characters_Amy:
					tp->ptp->twp->btimer |= 4u;
					break;
				}
			}
		}

		break;
	case MD_MOVE:
		execTPCoaster_h.Original(tp);
		if (player)
		{
			SetPositionP(id, twp->pos.x, twp->pos.y, twp->pos.z);
			SetRotationP(id, twp->ang.x, twp->ang.y - 0x4000, twp->ang.z);
		}
		break;
	case MD_OUT:
		if (player)
		{
			VibShot(id, 0);
			GetOutOfCartP(id, -1.2f - (Float)id * 0.8f, 1.5f + (Float)id * 0.3f, 0.0f);
			CharColliOff(twp);

			// Restart position since the roller coaster is no longer at the start
			if (id == 0)
			{
				NJS_VECTOR restartpos = twp->pos;
				restartpos.x += njSin(twp->ang.y - 0x4000) * 25.0f;
				restartpos.z += njCos(twp->ang.y - 0x4000) * 25.0f;
				restartpos.y += 2.0f;
				updateContinueData(&restartpos, &twp->ang);
			}
		}
		
		twp->wtimer = 0;
		twp->mode = MD_STOP;
		break;
	case MD_STOP:
		if (twp->wtimer > 60)
		{
			EntryColliList(tp->twp);
		}
		else
		{
			++twp->wtimer;

			if (twp->wtimer == 60)
				CharColliOn(twp);
		}
		break;
	}

	dispTPCoaster(tp);
}
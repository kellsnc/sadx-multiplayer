#include "pch.h"
#include "SADXModLoader.h"
#include "FunctionHook.h"

void __cdecl PathworkGoneWithTheWind_r(task* tp);
TaskHook PathworkGoneWithTheWind_h(0x4DF020, PathworkGoneWithTheWind_r);
void __cdecl PathworkGoneWithTheWind_r(task* tp)
{
	auto twp = tp->twp;
	auto path = (pathtag*)twp->value.ptr;

	// Originally a switch but who cares
	if (twp->mode == 1)
	{
		if (GetStageNumber() != twp->timer.w[0])
		{
			FreeTask(tp);
			return;
		}

		for (int pnum = 0; pnum < PLAYER_MAX; ++pnum)
		{
			auto ptwp = playertwp[pnum];

			if (!ptwp)
				continue;

			auto mask = (1 << pnum);

			if (twp->btimer & mask)
			{
				if (!(ptwp->flag & Status_OnPath))
				{
					twp->counter.b[pnum] = 0;
					twp->btimer &= ~mask;
				}
			}
			else
			{
				if (++twp->counter.b[pnum] < 60)
				{
					continue;
				}

				twp->counter.b[pnum] = 60;

				pathtbl* pt = &path->tblhead[0];
				NJS_POINT3 pos = { pt->xpos, pt->ypos, pt->zpos };
				njSubVector(&pos, &ptwp->pos);
				if (njScalor(&pos) < 50.0f)
				{
					dsPlay_timer(59, (int)twp, 1, 0, 60);
					RunAlongPathP(pnum, path);
					twp->btimer |= mask;
					break;
				}
			}
		}

	}
	else if (twp->mode == 0)
	{
		twp->btimer = 0;
		twp->counter.b[0] = 60;
		twp->counter.b[1] = 60;
		twp->counter.b[2] = 60;
		twp->counter.b[3] = 60;
		twp->mode = 1;
	}

	if (++twp->wtimer > 5u)
	{
		twp->wtimer = 0;
		twp->smode += 1;
		if (ClipLevel < 2)
		{
			task* leaftp = CreateElementalTask(0x2, 6, (TaskFuncPtr)0x4E4430);
			if (leaftp)
			{
				taskwk* leaftwp = leaftp->twp;
				leaftwp->counter.l = twp->value.l;
				leaftwp->timer.b[2] = twp->smode;
				if (twp->smode > 15)
				{
					twp->smode = 0;
				}
			}
		}
	}
}

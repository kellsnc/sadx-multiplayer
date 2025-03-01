#include "pch.h"
#include "multiplayer.h"
#include "result.h"
#include "utils.h"

void __cdecl relbox_switch_exec_r(task* task_p);
FastFunctionHookPtr<decltype(&relbox_switch_exec_r)> relbox_switch_exec_h(0x46AE60);

bool relbox_switch_test_riding_m(taskwk* twp, taskwk* parent_twp)
{
	for (int i = 0; i < 8; ++i)
	{
		if (playertwp[i])
		{
			NJS_VECTOR* pos = &twp->pos;
			NJS_VECTOR* ppos = &playertwp[i]->pos;

			if (((pos->z - ppos->z) * (pos->z - ppos->z) + (pos->x - ppos->x) * (pos->x - ppos->x) < 144.0f) &&
				pos->y + 27.0f < ppos->y && pos->y + 29.6f > ppos->y)
			{
				if (GetStageNumber() != LevelAndActIDs_LostWorld3)
					SetWinnerMulti(i);

				parent_twp->btimer = i;
				return true;
			}
		}
	}

	return false;
}

void __cdecl relbox_switch_exec_r(task* task_p)
{
	if (multiplayer::IsActive())
	{
		auto twp = task_p->twp;
		auto parent_twp = task_p->ptp->twp;

		if (twp->mode >= 1 || relbox_switch_test_riding_m(twp, parent_twp))
		{
			if (!twp->mode)
			{
				PadReadOff();
				PadReadOffP(-1);
				MovePlayersToWinnerPos(&twp->pos);
				twp->mode = 1;
				dsPlay_oneshot(13, 0, 0, 0);
			}

			twp->counter.f -= 0.25f;

			if (twp->counter.f < -3.0f)
			{
				twp->counter.f = -3.0f;

				if (twp->mode == 1)
				{
					if (parent_twp->mode < 1)
					{
						parent_twp->mode = 1;
					}

					twp->mode = 2;
					VibShot(parent_twp->btimer, 0);
				}
			}
		}

		twp->pos.y = parent_twp->pos.y + twp->counter.f;

		EntryColliList(twp);
		task_p->disp(task_p);
	}
	else
	{
		relbox_switch_exec_h.Original(task_p);
	}
}

void patch_releasebox_init()
{
	relbox_switch_exec_h.Hook(relbox_switch_exec_r);
}

RegisterPatch patch_releasebox(patch_releasebox_init);
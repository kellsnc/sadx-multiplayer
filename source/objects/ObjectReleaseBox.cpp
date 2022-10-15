#include "pch.h"
#include "multiplayer.h"
#include "result.h"
#include "utils.h"

bool relbox_switch_test_riding_m(taskwk* twp, taskwk* parent_twp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
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

void __cdecl relbox_switch_exec_r(task* task_p);
Trampoline relbox_switch_exec_t(0x46AE60, 0x46AE66, relbox_switch_exec_r);
void __cdecl relbox_switch_exec_r(task* task_p)
{
	if (multiplayer::IsActive())
	{
		char pmax = multiplayer::GetPlayerCount();
		auto twp = task_p->twp;
		auto parent_twp = task_p->ptp->twp;

		if (twp->mode >= 1 || relbox_switch_test_riding_m(twp, parent_twp))
		{
			if (!twp->mode)
			{
				PadReadOff();
				PadReadOffP(-1);

				for (uint8_t i = 0; i < pmax; ++i)
				{
					if (playertwp[i]) {

						PClearSpeed(playermwp[i], playerpwp[i]);
						MovePlayerToWinnerPos(i, twp);
					}
				}

				twp->mode = 1;
				dsPlay_oneshot(13, 0, 0, 0);
			}

			twp->counter.f -= 0.25f;

			if (twp->counter.f < -2.0f)
			{
				for (int i = 0; i < pmax; i++) {
					auto ptwp = playertwp[i];
					CharColliOn(ptwp); //restore col to stand on the capsule
				}
			}

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
					VibShot((char)parent_twp->btimer, 0);
				}
			}
		}

		twp->pos.y = parent_twp->pos.y + twp->counter.f;

		EntryColliList(twp);
		task_p->disp(task_p);
	}
	else
	{
		TARGET_STATIC(relbox_switch_exec)(task_p);
	}
}
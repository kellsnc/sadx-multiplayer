#include "pch.h"
#include "SADXModLoader.h"
#include "FunctionHook.h"

static FunctionHook<void, task*> PondExecutor_hook(0x7AA870);
static FunctionHook<void, task*> PondDisplayer_hook(0x7AA3D0);

static void __cdecl PondDisplayer_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        if (tp->twp->mode < 4)
        {
            PondDraw(tp->twp, (enemywk*)tp->mwp);
        }
    }
    else
    {
		PondDisplayer_hook.Original(tp);
    }
}

void PondExecutor_r(task* tp)
{
	auto twp = tp->twp;
	auto ewp = (enemywk*)tp->mwp;

	if (twp && ewp)
	{
		if (twp->mode < 3)
		{
			if (EnemyCheckDamage(twp, ewp))
			{
				twp->mode = 3;
				twp->counter.b[1] = 0;
				twp->counter.b[2] = 1;
				twp->wtimer = 0;
				twp->scl.z = 0.35f;
				return;
			}
		}
	}

	PondExecutor_hook.Original(tp);
}

void PatchPondSkater()
{
	PondDisplayer_hook.Hook(PondDisplayer_r);
	PondExecutor_hook.Hook(PondExecutor_r);
}

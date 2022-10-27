#include "pch.h"

static void Shat_Exec_r(task* tp);
TaskHook Shat_Exec_t(0x5CD540, Shat_Exec_r);

static void Shat_Exec_r(task* tp)
{
	auto twp = tp->twp;
	if (multiplayer::IsActive())
	{
		if (twp->mode >= 2)
		{
			twp->mode = 1;
			twp->btimer = 0;
			twp->wtimer = 0;
		}
	}

	Shat_Exec_t.Original(tp);
}
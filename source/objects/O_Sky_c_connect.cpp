#include "pch.h"

void O_Sky_c_connect_r(task* a1);
Trampoline O_Sky_c_connect_t(0x5FAE90, 0x5FAE98, O_Sky_c_connect_r);
void O_Sky_c_connect_r(task* a1)
{
	if (multiplayer::IsActive())
	{
		if (a1->twp->mode >= 1)
		{
			a1->exec = (TaskFuncPtr)0x5FB130;
			return;
		}
	}

	TaskFunc(origin, O_Sky_c_connect_t.Target());
	origin(a1);
}
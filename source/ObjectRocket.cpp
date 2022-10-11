#include "pch.h"

void ObjectRocket_Wait_r(task* a1);
static FunctionHook<void, task*> ObjectRocket_Wait_t(0x4CA1F0, ObjectRocket_Wait_r);

void ObjectRocket_Wait_r(task* a1)
{
	auto data = a1->twp;

	if (data->mode == 2)
	{
		for (int i = 0; i < multiplayer::GetPlayerCount(); i++)
		{
			if (ChkPlayerCanHold(a1, i))
			{
				EntryColliList(data);
				a1->disp(a1);
				return;
			}
		}
	}

	ObjectRocket_Wait_t.Original(a1);
}
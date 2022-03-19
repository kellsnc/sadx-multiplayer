#include "pch.h"

TaskFunc(ObjectMountainAsiba, 0x60E150);
TaskFunc(OMasiba_Display, 0x60DEA0);

static void __cdecl ObjectMountainAsiba_r(task* obj);
Trampoline ObjectMountainAsiba_t(0x60E150, 0x60E156, ObjectMountainAsiba_r);

static auto Masiba_InitDyncol = GenerateUsercallWrapper<void (*)(task* a1)>(noret, 0x60DDB0, rEDI); // custom name original is "init_9"

static void __cdecl ObjectMountainAsiba_r(task* obj)
{

	if (!multiplayer::IsActive())
	{
		TARGET_STATIC(ObjectMountainAsiba);
		return;
	}


	if (CheckRangeOut(obj))
		return;

	taskwk* data = obj->twp;

	if (data->mode)
	{
		if (data->mode == 1 && !ObjectSelectedDebug((ObjectMaster*)obj))
		{
			//OMasiba_Exec(obj);
			OMasiba_Display(obj);
			return;
		}
	}
	else
	{
		obj->dest = (TaskFuncPtr)0x541360;
		obj->disp = OMasiba_Display;
		Masiba_InitDyncol(obj);
		data->mode = 1;
		if (rd_mountain_twp)
		{
			if (!ObjectSelectedDebug((ObjectMaster*)obj))
			{
				data->pos.y = rd_mountain_twp->pos.y;
			}
		}
	}

	OMasiba_Display(obj);

}
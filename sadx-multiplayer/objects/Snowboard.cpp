#include "pch.h"
#include "multiplayer.h"

Trampoline* SetPlayerSnowBoard_t = nullptr;
Trampoline* SetPlayerSnowBoardSand_t = nullptr;

static void SetPlayerSnowBoard_m()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			auto char_id = TASKWK_CHARID(ptwp);

			switch (char_id)
			{
			case Characters_Sonic:
				SetInputP(i, 44);
				CreateElementalTask(3u, 2, (TaskFuncPtr)0x4959E0)->twp->counter.b[0] = i;
				break;
			case Characters_Tails:
				SetInputP(i, 44);
				CreateElementalTask(3u, 2, (TaskFuncPtr)0x461510)->twp->counter.b[0] = i;
				break;
			}
		}
	}
}

static void __cdecl SetPlayerSnowBoard_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		SetPlayerSnowBoard_m();
		FreeTask(tp);
	}
	else
	{
		TARGET_DYNAMIC(SetPlayerSnowBoard)(tp);
	}
}

static void __cdecl SetPlayerSnowBoardSand_r()
{
	if (multiplayer::IsActive())
	{
		SetPlayerSnowBoard_m();
	}
	else
	{
		TARGET_DYNAMIC(SetPlayerSnowBoardSand)();
	}
}

void InitSnowBoardPatches()
{
	SetPlayerSnowBoard_t = new Trampoline(0x4E9660, 0x4E9669, SetPlayerSnowBoard_r);
	WriteCall((void*)((int)SetPlayerSnowBoard_t->Target() + 4), SetInputP); // Patch trampoline

	SetPlayerSnowBoardSand_t = new Trampoline(0x597B10, 0x597B19, SetPlayerSnowBoardSand_r);
	WriteCall((void*)((int)SetPlayerSnowBoardSand_t->Target() + 4), SetInputP); // Patch trampoline
}
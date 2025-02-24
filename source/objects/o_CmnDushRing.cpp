#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "sadx_utils.h"
#include "utils.h"

FunctionPointer(float, GetTargetDist, (task*), 0x7A2280);
FunctionPointer(Bool, CalcDashPosition, (task*), 0x7A2320);
FunctionPointer(Bool, CreateChkDashPlayer, (task* tp, Sint32 pl_num, Float dist), 0x7A2610);

void ChkDashPlayer_r(task* tp);
Bool InitCmnDushRing_r(task* tp);

FastFunctionHook<void, task*> ChkDashPlayer_h(0x7A2500, ChkDashPlayer_r);
FastFunctionHook<Bool, task*> InitCmnDushRing_h(0x7A2660, InitCmnDushRing_r);

// The original code checks for all player despite being a per-player subtask
void ChkDashPlayer_m(task* tp)
{
	taskwk* twp = tp->twp;
	
	auto pnum = tp->twp->value.l;
	auto ptwp = playertwp[pnum];

	if (!ptwp)
	{
		return;
	}

	switch (twp->mode)
	{
	case 0:		
		if (GetDistance2(&twp->pos, &ptwp->pos) < 900.0f)
		{
			twp->mode = 1;
			twp->counter.ptr = GetPlayerTaskPointer(pnum);
			PClearSpeed(playermwp[pnum], playerpwp[pnum]);
			dsPlay_oneshot(SE_M_ACCGATE, 0, 0, 0);

			switch (TASKWK_CHARID(ptwp))
			{
			case Characters_Tails:
				SetInputP(pnum, PL_OP_PROP);
				break;
			case Characters_Knuckles: // it sets a dummy action for Knuckles
				SetInputP(pnum, PL_OP_PLACEON);
				break;
			}
		}
		break;
	case 1:
		if (CalcDashPosition(tp))
		{
			twp->mode = 0;
			SetInputP(pnum, PL_OP_LETITGO);
			twp->scl.y = 0.0f;
			twp->timer.l = 0;
			if (TASKWK_CHARID(ptwp) == Characters_Tails)
			{
				SetInputP(pnum, PL_OP_FLY);
			}
		}
		break;
	}
}

void ChkDashPlayer_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ChkDashPlayer_m(tp);
	}
	else if (tp->twp->value.l < 2)
	{
		ChkDashPlayer_h.Original(tp);
	}
}

Bool InitCmnDushRing_m(task* tp)
{
	taskwk* twp = tp->twp;
	twp->counter.ptr = *(NJS_OBJECT**)0x7A266F;
	twp->mode = 1;
	tp->disp = *(TaskFuncPtr*)0x7A2677;
	tp->dest = *(TaskFuncPtr*)0x7A267E;

	Float dist = GetTargetDist(tp);

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		CreateChkDashPlayer(tp, i, dist);
	}

	return TRUE;
}

Bool InitCmnDushRing_r(task* tp)
{
	if (multiplayer::IsEnabled)
	{
		return InitCmnDushRing_m(tp);
	}
	else
	{
		return InitCmnDushRing_h.Original(tp);
	}
}

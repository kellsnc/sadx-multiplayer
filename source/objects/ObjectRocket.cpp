#include "pch.h"
#include "SADXModLoader.h"
#include "FunctionHook.h"
#include "multiplayer.h"
#include "camera.h"

static FunctionHook<void, task*> ObjectRocket_Wait_t(0x4CA1F0);
static FunctionHook<void, task*> ObjectRocket_Hold_V_t(0x4C9C60);
UsercallFuncVoid(ObjectRocket_Hold_H, (task* tp), (tp), 0x4C9BC0, rEDX);

static void __cdecl ObjectRocket_Hold_V_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto player = *(taskwk**)(twp->value.l + 32);
		auto pnum = TASKWK_PLAYERID(player);

		if (++twp->wtimer == 15)
		{
			twp->btimer |= 0x4;

			auto param = GetExternCameraParam(pnum);
			if (param)
			{
				param->xDirPos = twp->scl.x;
				param->yDirPos = twp->scl.y;
				param->zDirPos = twp->scl.z;
				CameraSetEventCameraFunc_m(pnum, CameraEventPoint, CAMADJ_NONE, CDM_LOOKAT);
			}
		}

		// Check detach
		if (((perG[pnum].press & (AttackButtons | JumpButtons)) && (twp->btimer & 0x4))
			|| twp->scl.y + 300.0f < twp->pos.y)
		{
			tp->exec = (TaskFuncPtr)0x4C9B40;
			twp->mode = 6;
			twp->wtimer = 0;
			SetInputP(pnum, PL_OP_LETITGO);
			CameraReleaseEventCamera_m(pnum);
			CharColliOn(player);
		}
	}
	else
	{
		ObjectRocket_Hold_V_t.Original(tp);
	}
}

static void __cdecl ObjectRocket_Hold_H_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto mwp = tp->mwp;
		auto fwp = tp->fwp;
		auto player = *(taskwk**)(twp->value.l + 32);
		auto pnum = TASKWK_PLAYERID(player);

		if (++twp->wtimer == 10)
		{
			twp->btimer |= 0x4;
		}

		auto x = twp->pos.x - fwp->pos_spd.x;
		auto z = twp->pos.z - fwp->pos_spd.z;

		// Check detach
		if (((perG[pnum].press & (AttackButtons | JumpButtons)) && (twp->btimer & 0x4)) || x * x + z * z > mwp->rad)
		{
			tp->exec = (TaskFuncPtr)0x4C9B00;
			twp->mode = 6;
			twp->wtimer = 0;
			SetInputP(pnum, PL_OP_LETITGO);
			CharColliOn(player);
		}
	}
	else
	{
		ObjectRocket_Hold_H.Original(tp);
	}
}

static void __cdecl ObjectRocket_Wait_r(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 2)
	{
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (playertwp[i] && ChkPlayerCanHold(tp, i))
			{
				EntryColliList(twp);
				tp->disp(tp);
				return;
			}
		}
	}

	ObjectRocket_Wait_t.Original(tp);
}

void PatchRocket()
{
	ObjectRocket_Wait_t.Hook(ObjectRocket_Wait_r);
	ObjectRocket_Hold_V_t.Hook(ObjectRocket_Hold_V_r);
	ObjectRocket_Hold_H.Hook(ObjectRocket_Hold_H_r);
}
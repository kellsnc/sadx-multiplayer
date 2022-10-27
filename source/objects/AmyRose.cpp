#include "pch.h"
#include "ObjCylinderCmn.h"
#include "e_cart.h"

UsercallFunc(signed int, Amy_CheckInput_t, (playerwk* a1, motionwk2* a2, taskwk* a3), (a1, a2, a3), 0x487810, rEAX, rECX, rEDI, rESI);
TaskHook AmyExec_t((intptr_t)Amy_Main);
static FunctionHook<void, taskwk*, motionwk2*, playerwk*> Amy_RunsActions_t((intptr_t)0x488880);
TaskHook AmyJiggle_t((intptr_t)0x485C50);
TaskHook AmySkirtShape_t(0x485F40);
TaskHook AmyEyeTracker_t(0x486410);
TaskHook AmyBirdExe_t(0x4C63F0);
TaskHook LoadAmyBird_t(0x4C6790);

task* AmyBirdM[PLAYER_MAX] = { 0 };

void AmyBird_Del(task* obj)
{
	if (obj->twp)
	{
		char pnum = obj->twp->smode;
		auto task = AmyBirdM[pnum];

		if (task)
		{
			FreeTask(task);
		}

		AmyBirdM[pnum] = nullptr;
	}
}

void __cdecl AmyBird_Main_r(task* obj)
{
	if (!multiplayer::IsActive() || EV_MainThread_ptr)
	{
		return AmyBirdExe_t.Original(obj);
	}

	auto data = obj->twp;
	auto pnum = data->smode;
	auto Player = playertwp[pnum];

	if (!Player)
	{
		FreeTask(obj);
		return;
	}

	data->ang.y = BAMS_SubWrap(data->ang.y, 0x8000 - Player->ang.y, 1024);
	data->ang.x += 16;
	data->ang.z += 1024;

	NJS_VECTOR dest;

	dest.x = njCos(data->ang.x) * 3.0f + Player->pos.x;
	dest.y = njSin(data->ang.z) + 12.0f + Player->pos.y;
	dest.z = njSin(data->ang.x) * 3.0f + Player->pos.z;

	float distance = sqrtf(powf(dest.x - data->pos.x, 2) + powf(dest.y - data->pos.y, 2) + powf(dest.z - data->pos.z, 2));

	if (distance >= 200.0f) {
		data->pos.x = dest.x;
		data->pos.y = dest.y;
		data->pos.z = dest.z;
	}
	else
	{
		data->pos.x = (dest.x - data->pos.x) * 0.25f + data->pos.x;
		data->pos.y = (dest.y - data->pos.y) * 0.25f + data->pos.y;
		data->pos.z = (dest.z - data->pos.z) * 0.25f + data->pos.z;
	}

	data->counter.f = FrameCounterUnpaused % Birdie_Anim.motion->nbFrame;
	obj->disp(obj);

	DrawShadow((EntityData1*)&data, 0.40000001f);
	LoopTaskC(obj);
}

void Load_AmyBird_r(task* obj)
{
	obj->exec = AmyBird_Main_r;
	obj->disp = (TaskFuncPtr)0x4C62D0;
	obj->dest = AmyBird_Del;
	auto data = obj->twp;
	auto pnum = data->smode;
	data->pos = playertwp[pnum]->pos;
	data->pos.y += 12.0f;
	data->ang.y = 0x8000 - data->ang.y;
}

void __cdecl Init_AmyBird(task* tp)
{
	if (!multiplayer::IsActive() || EV_MainThread_ptr)
	{
		return LoadAmyBird_t.Original(tp);
	}

	for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
	{
		if (!AmyBirdM[i] && playertwp[i] && playertwp[i]->counter.b[1] == Characters_Amy)
		{
			AmyBirdM[i] = CreateElementalTask(LoadObj_Data1, 3, Load_AmyBird_r);
			AmyBirdM[i]->twp->smode = i;
		}
	}
}

static void __cdecl AmyEyeTracker_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	AmyEyeTracker_t.Original(tp);
}

static void __cdecl AmyJiggle_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	AmyJiggle_t.Original(tp);
}

static void __cdecl AmySkirtShape_r(task* tp)
{
	if (DeleteJiggle(tp))
	{
		return;
	}

	AmySkirtShape_t.Original(tp);
}

signed int Amy_CheckInput_r(playerwk* co2, motionwk2* data2, taskwk* data)
{
	auto even = data->ewp;

	if (even->move.mode || even->path.list || ((data->flag & Status_DoNextAction) == 0))
	{
		return Amy_CheckInput_t.Original(co2, data2, data);
	}

	switch (data->smode)
	{
	case 5:
		if (CurrentLevel == LevelIDs_SkyDeck)
		{
			data->mode = SDCannonMode;
			co2->mj.reqaction = 18; //falling
			return 1;
		}
		break;
	case 32:

		if (SetCylinderNextAction(data, data2, co2))
			return 1;

		break;
	}

	return Amy_CheckInput_t.Original(co2, data2, data);
}

void Amy_RunsActions_r(taskwk* data1, motionwk2* data2, playerwk* co2) {
	switch (data1->mode)
	{
	case 48: //cart
		KillPlayerInKart(data1, co2, 47, 26);
		break;
	case SDCannonMode:
		if (!AmyCheckInput(co2, data2, data1) && (data1->flag & 3) != 0)
		{
			if (PCheckBreak(data1) && co2->spd.x > 0.0f)
			{
				data1->mode = 9;
			}
			if (!AmyCheckStop(data1, co2))
			{
				data1->mode = 2;
			}

			data1->ang.x = data2->ang_aim.x;
			data1->ang.z = data2->ang_aim.z;
			co2->mj.reqaction = 2;
		}
		return;
	case SDCylStd:
		if (AmyCheckInput(co2, data2, data1) || AmyCheckJump(co2, data1, data2))
		{
			co2->htp = 0;
			return;
		}

		Mode_SDCylStdChanges(data1, co2);
		return;
	case SDCylDown:

		if (AmyCheckInput(co2, data2, data1) || AmyCheckJump(co2, data1, data2))
		{
			co2->htp = 0;
			return;
		}

		Mode_SDCylDownChanges(data1, co2);

		return;
	case SDCylLeft:
		if (AmyCheckInput(co2, data2, data1) || AmyCheckJump(co2, data1, data2))
		{
			co2->htp = 0;
			return;
		}

		if (Controllers[(unsigned __int8)data1->counter.b[0]].LeftStickX << 8 <= -3072)
		{
			if (data1->mode < SDCylStd || data1->mode > SDCylRight)
			{
				co2->htp = 0;
			}

			return;
		}
		data1->mode = SDCylStd;

		return;
	case SDCylRight:
		if (AmyCheckInput(co2, data2, data1) || AmyCheckJump(co2, data1, data2))
		{
			co2->htp = 0;
			return;
		}

		if (Controllers[(unsigned __int8)data1->counter.b[0]].LeftStickX << 8 >= 3072)
		{
			if (data1->mode < SDCylStd || data1->mode > SDCylRight)
			{
				co2->htp = 0;
			}
			return;
		}

		data1->mode = SDCylStd;
		return;
	}

	Amy_RunsActions_t.Original(data1, data2, co2);
}

void AmyExec_r(task* obj)
{
	auto data = obj->twp;
	motionwk2* data2 = (motionwk2*)obj->mwp;
	playerwk* co2 = (playerwk*)obj->mwp->work.l;

	if (co2)
	{
		co2 = co2;
	}

	switch (data->mode)
	{
	case SDCannonMode:
		CannonModePhysics(data, data2, co2);
		break;
	case SDCylStd:
		Mode_SDCylinderStd(data, co2);
		break;
	case SDCylDown:
		Mode_SDCylinderDown(data, co2);
		break;
	case SDCylLeft:
		Mode_SDCylinderLeft(data, co2);
		break;
	case SDCylRight:
		Mode_SDCylinderRight(data, co2);
		break;
	}

	AmyExec_t.Original(obj);
}

void Init_AmyPatches()
{
	AmyExec_t.Hook(AmyExec_r);
	Amy_CheckInput_t.Hook(Amy_CheckInput_r);
	Amy_RunsActions_t.Hook(Amy_RunsActions_r);
	AmyJiggle_t.Hook(AmyJiggle_r);
	AmySkirtShape_t.Hook(AmySkirtShape_r);
	AmyEyeTracker_t.Hook(AmyEyeTracker_r);

	LoadAmyBird_t.Hook(Init_AmyBird);
	AmyBirdExe_t.Hook(AmyBird_Main_r);
}
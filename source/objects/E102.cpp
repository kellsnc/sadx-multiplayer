#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "VariableHook.hpp"
#include "multiplayer.h"
#include "splitscreen.h"
#include "ObjCylinderCmn.h"

DataPointer(NJS_MATRIX, head_matrix, 0x3C53AD8); // static to E102.c

VariableHook<char, 0x3C53C40> e102_hover_flag_m;
VariableHook<char, 0x3C53C41> e102_hover_flag_p_m;

static FunctionHook<void, task*, motionwk2*, playerwk*> E102_RunsActions_t((intptr_t)0x481460);
UsercallFunc(signed int, E102_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x480870, rEAX, rEDI, rESI, stack4);
TaskHook E102_t((intptr_t)0x483430);
TaskHook E102DispTimeUpWarning_t(0x4C51D0);

static void E102DispTimeUpWarning_r(task* tp)
{
	if (multiplayer::IsActive() && CurrentCharacter != Characters_Gamma)
	{
		FreeTask(tp);
		return;
	}

	E102DispTimeUpWarning_t.Original(tp);
}

void E102_RunActions_r(task* tsk, motionwk2* data2, playerwk* co2) {

	auto data1 = tsk->twp;

	switch (data1->mode)
	{
	case SDCannonMode:
		if (!E102CheckInput(co2, data1, data2) && (data1->flag & 3) != 0)
		{
			if (PCheckBreak(data1) && co2->spd.x > 0.0f)
			{
				data1->mode = 8;
			}
			if (!E102CheckStop(data1, co2))
			{
				data1->mode = 2;
			}

			data1->ang.x = data2->ang_aim.x;
			data1->ang.z = data2->ang_aim.z;
			co2->mj.reqaction = 2;
		}
		return;
	case SDCylStd:
		if (E102CheckInput(co2, data1, data2) || E102CheckJump(co2, data1))
		{
			co2->htp = 0;
			return;
		}

		Mode_SDCylStdChanges(data1, co2);
		return;
	case SDCylDown:

		if (E102CheckInput(co2, data1, data2) || E102CheckJump(co2, data1))
		{
			co2->htp = 0;
			return;
		}


		Mode_SDCylDownChanges(data1, co2);

		return;
	case SDCylLeft:
		if (E102CheckInput(co2, data1, data2) || E102CheckJump(co2, data1))
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
		if (E102CheckInput(co2, data1, data2) || E102CheckJump(co2, data1))
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

	E102_RunsActions_t.Original(tsk, data2, co2);
}

signed int E102_CheckInput_r(playerwk* co2, taskwk* data, motionwk2* data2)
{
	auto even = data->ewp;

	if (even->move.mode || even->path.list || ((data->flag & Status_DoNextAction) == 0))
	{
		return E102_CheckInput_t.Original(co2, data, data2);
	}

	switch (data->smode)
	{
	case 5:
		data->mode = SDCannonMode;
		co2->mj.reqaction = 7;
		return 1;
	case 32:

		if (SetCylinderNextAction(data, data2, co2))
			return 1;

		break;

	}

	return E102_CheckInput_t.Original(co2, data, data2);
}

static void __cdecl E102Display_r(task* tp);
Trampoline E102Display_t(0x47FD50, 0x47FD57, E102Display_r);
static void __cdecl E102Display_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pwp = (playerwk*)tp->mwp->work.ptr;
		auto ewk = (E102WK*)pwp->free.ptr[0];

		// SplitScreen compatibility patches
		if (SplitScreen::IsActive())
		{
			// Set global variables again
			e102_work_ptr = ewk;
			e102_hover_flag = perG[TASKWK_PLAYERID(twp)].on & JumpButtons && (twp->mode == 44 || twp->mode == 7 || twp->mode == 22 || twp->mode == 36);

			// Recalculate submodel positions:

			if (pwp->mj.mtnmode == 2)
			{
				CalcMMMatrix(_nj_unit_matrix_, pwp->mj.actwkptr, pwp->mj.nframe, pwp->mj.plactptr[pwp->mj.reqaction].objnum, nullptr);
			}
			else
			{
				CalcMMMatrix(_nj_unit_matrix_, pwp->mj.plactptr[pwp->mj.reqaction].actptr, pwp->mj.nframe, pwp->mj.plactptr[pwp->mj.reqaction].objnum, 0);
			}

			GetMMMatrix(54, head_matrix);
			GetMMMatrix(30, (NJS_MATRIX_PTR)0x3C53C48);
			GetMMMatrix(39, (NJS_MATRIX_PTR)0x3C53B78);
			njRotateZ((NJS_MATRIX_PTR)0x3C53B78, 0x4000);
			GetMMMatrix(2, (NJS_MATRIX_PTR)0x3C53B20);
			GetMMMatrix(37, (NJS_MATRIX_PTR)0x3C53C00);
		}

		// Hide normal arm *before* rendering the model...
		if ((ewk->gun_mode < 2 || ewk->gun_mode > 5) && (ewk->laser_mode < 2 || ewk->laser_mode > 3))
		{
			clrObjFlags(&e102_arm, 8u);
		}
		else
		{
			setObjFlags(&e102_arm, 8u);
		}
	}

	TARGET_STATIC(E102Display)(tp);
}


static void __cdecl E102_r(task* tp)
{
	auto data = tp->twp;
	auto data2 = (motionwk2*)tp->mwp;
	auto co2 = (playerwk*)tp->mwp->work.l;

	if (multiplayer::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(tp->twp);

		// Patch global variables:
		if (pnum > 0)
		{
			auto backup = e102_hover_flag;
			auto backup_p = e102_hover_flag_p;
			e102_hover_flag = e102_hover_flag_m[pnum];
			e102_hover_flag_p = e102_hover_flag_p_m[pnum];
			E102_t.Original(tp);
			e102_hover_flag_m[pnum] = e102_hover_flag;
			e102_hover_flag_p_m[pnum] = e102_hover_flag_p;
			e102_hover_flag = backup;
			e102_hover_flag_p = backup_p;
			return;
		}
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

	E102_t.Original(tp);
}

static void __cdecl E102LockOnCursor_r(task* tp);
Trampoline E102LockOnCursor_t(0x4CF090, 0x4CF097, E102LockOnCursor_r);
static void __cdecl E102LockOnCursor_r(task* tp)
{
	e102_work_ptr = (E102WK*)tp->awp[1].work.ul[0];
	TARGET_STATIC(E102LockOnCursor)(tp);
}

static void __cdecl E102Beam_r(task* tp);
Trampoline E102Beam_t(0x4C40B0, 0x4C40B5, E102Beam_r);
static void __cdecl E102Beam_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (twp->mode == 0)
		{
			// Beams have the player number stored in them, but it's immediately overwritten
			// Let's store it somewhere else before it gets overwritten
			twp->btimer = TASKWK_PLAYERID(twp);
		}
	}

	TARGET_STATIC(E102Beam)(tp);
}

static void __cdecl E102AddSecTotalNewDisplay_r(task* tp);
Trampoline E102AddSecTotalNewDisplay_t(0x49FDA0, 0x49FDA5, E102AddSecTotalNewDisplay_r);
static void __cdecl E102AddSecTotalNewDisplay_r(task* tp)
{
	if (multiplayer::IsCoopMode() && SplitScreen::IsActive())
	{
		SplitScreen::SaveViewPort();
		SplitScreen::ChangeViewPort(-1);
		TARGET_STATIC(E102AddSecTotalNewDisplay)(tp);
		SplitScreen::RestoreViewPort();
	}
	else if (!multiplayer::IsActive())
	{
		TARGET_STATIC(E102AddSecTotalNewDisplay)(tp);
	}
}

static void __cdecl E102AddSecTotalNew_r(task* tp);
Trampoline E102AddSecTotalNew_t(0x49FF10, 0x49FF16, E102AddSecTotalNew_r);
static void __cdecl E102AddSecTotalNew_r(task* tp)
{
	if (!multiplayer::IsBattleMode())
	{
		TARGET_STATIC(E102AddSecTotalNew)(tp);
	}
}

void initGammaPatch()
{
	E102_t.Hook(E102_r);
	E102_CheckInput_t.Hook(E102_CheckInput_r);
	E102_RunsActions_t.Hook(E102_RunActions_r);
	E102DispTimeUpWarning_t.Hook(E102DispTimeUpWarning_r);
}
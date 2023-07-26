#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "VariableHook.hpp"
#include "multiplayer.h"
#include "splitscreen.h"
#include "gravity.h"
#include "e_cart.h"
#include "result.h"
#include "ObjCylinderCmn.h"

DataPointer(NJS_MATRIX, head_matrix, 0x3C53AD8); // static to E102.c

VariableHook<char, 0x3C53C40> e102_hover_flag_m;
VariableHook<char, 0x3C53C41> e102_hover_flag_p_m;

static FunctionHook<void, task*, motionwk2*, playerwk*> E102_RunsActions_t((intptr_t)0x481460);
UsercallFunc(signed int, E102_CheckInput_t, (playerwk* a1, taskwk* a2, motionwk2* a3), (a1, a2, a3), 0x480870, rEAX, rEDI, rESI, stack4);
TaskHook E102_t((intptr_t)0x483430);
TaskHook E102DispTimeUpWarning_t(0x4C51D0);

bool IsCountingDown()
{
	return CurrentCharacter == Characters_Gamma && !multiplayer::IsBattleMode();
}

void __cdecl E102TimeOverHook(Uint8 pno)
{
	if (IsCountingDown())
	{
		if (multiplayer::IsCoopMode())
		{
			SetChangeGameMode(GAMEMD_MISS);
		}
		else
		{
			KillHimP(pno);
		}
	}
}

void __cdecl E102AddSeconds_r(int seconds)
{
	if (IsCountingDown())
	{
		AddSeconds(seconds);
	}
}

static void E102DispTimeUpWarning_r(task* tp)
{
	if (IsCountingDown())
	{
		if (SplitScreen::IsActive())
		{
			SplitScreen::SaveViewPort();
			SplitScreen::ChangeViewPort(-1);
			E102DispTimeUpWarning_t.Original(tp);
			SplitScreen::RestoreViewPort();
		}
		else
		{
			E102DispTimeUpWarning_t.Original(tp);
		}
	}
}

void E102_RunActions_r(task* tp, motionwk2* mwp, playerwk* pwp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		switch (twp->mode)
		{
		case 53: //cart
			KillPlayerInKart(twp, pwp, 51, 69);
			break;
		case SDCannonMode:
			if (!E102CheckInput(pwp, twp, mwp) && (twp->flag & 3) != 0)
			{
				if (PCheckBreak(twp) && pwp->spd.x > 0.0f)
				{
					twp->mode = 8;
				}
				if (!E102CheckStop(twp, pwp))
				{
					twp->mode = 2;
				}

				twp->ang.x = mwp->ang_aim.x;
				twp->ang.z = mwp->ang_aim.z;
				pwp->mj.reqaction = 2;
			}
			return;
		case SDCylStd:
			if (E102CheckInput(pwp, twp, mwp) || E102CheckJump(pwp, twp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_SDCylStdChanges(twp, pwp);
			return;
		case SDCylDown:

			if (E102CheckInput(pwp, twp, mwp) || E102CheckJump(pwp, twp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_SDCylDownChanges(twp, pwp);

			return;
		case SDCylLeft:
			if (E102CheckInput(pwp, twp, mwp) || E102CheckJump(pwp, twp))
			{
				pwp->htp = 0;
				return;
			}

			if (Controllers[TASKWK_PLAYERID(twp)].LeftStickX << 8 <= -3072)
			{
				if (twp->mode < SDCylStd || twp->mode > SDCylRight)
				{
					pwp->htp = 0;
				}

				return;
			}

			twp->mode = SDCylStd;
			return;
		case SDCylRight:
			if (E102CheckInput(pwp, twp, mwp) || E102CheckJump(pwp, twp))
			{
				pwp->htp = 0;
				return;
			}

			if (Controllers[TASKWK_PLAYERID(twp)].LeftStickX << 8 >= 3072)
			{
				if (twp->mode < SDCylStd || twp->mode > SDCylRight)
				{
					pwp->htp = 0;
				}
				return;
			}

			twp->mode = SDCylStd;
			return;
		}
	}

	E102_RunsActions_t.Original(tp, mwp, pwp);
}

signed int E102_CheckInput_r(playerwk* pwp, taskwk* twp, motionwk2* mwp)
{
	if (multiplayer::IsActive())
	{
		auto even = twp->ewp;
		auto pnum = TASKWK_PLAYERID(twp);

		if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
		{
			return E102_CheckInput_t.Original(pwp, twp, mwp);
		}

		switch (twp->smode)
		{
		case PL_OP_PARABOLIC:
			if (CurrentLevel != LevelIDs_Casinopolis)
			{
				twp->mode = SDCannonMode;
				pwp->mj.reqaction = 7;
				return TRUE;
			}
			break;
		case PL_OP_PLACEWITHKIME:
			if (CheckDefeat(pnum))
			{
				twp->mode = 52;
				pwp->mj.reqaction = 69;
				twp->ang.z = 0;
				twp->ang.x = 0;
				PClearSpeed(mwp, pwp);
				twp->flag &= ~0x2500;
				CancelLookingAtP(pnum);
				return TRUE;
			}
			break;
		case PL_OP_HOLDONPILLAR:
			if (SetCylinderNextAction(twp, mwp, pwp))
				return TRUE;
			break;
		}
	}

	return E102_CheckInput_t.Original(pwp, twp, mwp);
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

static void E102_m(task* tp)
{
	auto twp = tp->twp;
	auto mwp = (motionwk2*)tp->mwp;
	auto pwp = (playerwk*)mwp->work.ptr;

	auto pnum = TASKWK_PLAYERID(tp->twp);

	gravity::SaveGlobalGravity();
	gravity::SwapGlobalToUserGravity(pnum);

	if (pnum >= 1)
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
	}
	else
	{
		E102_t.Original(tp);
	}

	gravity::RestoreGlobalGravity();
}

void __cdecl E102_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		E102_m(tp);
	}
	else
	{
		E102_t.Original(tp);
	}
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
	if (IsCountingDown())
	{
		if (SplitScreen::IsActive())
		{
			SplitScreen::SaveViewPort();
			SplitScreen::ChangeViewPort(-1);
			TARGET_STATIC(E102AddSecTotalNewDisplay)(tp);
			SplitScreen::RestoreViewPort();
		}
		else
		{
			TARGET_STATIC(E102AddSecTotalNewDisplay)(tp);
		}
	}
}

static void __cdecl E102AddSecTotalNew_r(task* tp);
Trampoline E102AddSecTotalNew_t(0x49FF10, 0x49FF16, E102AddSecTotalNew_r);
static void __cdecl E102AddSecTotalNew_r(task* tp)
{
	if (IsCountingDown())
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
	WriteCall((void*)0x49FD54, E102AddSeconds_r);
	WriteCall((void*)0x47FC17, E102TimeOverHook);
}
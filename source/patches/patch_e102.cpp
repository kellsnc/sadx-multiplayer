#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "VariableHook.hpp"
#include "multiplayer.h"
#include "splitscreen.h"
#include "gravity.h"
#include "patch_e_cart.h"
#include "result.h"
#include "patch_player.h"
#include "patch_o_sky_cyl_cmn.h"

DataPointer(NJS_MATRIX, head_matrix, 0x3C53AD8); // static to E102.c

VariableHook<char, 0x3C53C40> e102_hover_flag_m;
VariableHook<char, 0x3C53C41> e102_hover_flag_p_m;

FastFunctionHook<void, task*, motionwk2*, playerwk*> E102_RunsActions_h(0x481460);
FastUsercallHookPtr<Bool(*)(taskwk* twp, playerwk* pwp, motionwk2* mwp), rEAX, rESI, rEDI, stack4> E102_CheckInput_h(0x480870);
FastFunctionHook<void, task*> E102_h(0x483430);
FastFunctionHook<void, task*> E102DispTimeUpWarning_h(0x4C51D0);
FastFunctionHookPtr<TaskFuncPtr> E102LockOnCursor_h(0x4CF090);
FastFunctionHookPtr<TaskFuncPtr> E102Beam_h(0x4C40B0);
FastFunctionHookPtr<TaskFuncPtr> E102AddSecTotalNewDisplay_h(0x49FDA0);
FastFunctionHookPtr<TaskFuncPtr> E102AddSecTotalNew_h(0x49FF10);

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

static Bool __cdecl GammaTickTimePatch()
{
	return IsCountingDown() ? Characters_Gamma : Characters_Sonic;
}

static void E102DispTimeUpWarning_r(task* tp)
{
	if (IsCountingDown())
	{
		if (splitscreen::IsActive())
		{
			splitscreen::SaveViewPort();
			splitscreen::ChangeViewPort(-1);
			E102DispTimeUpWarning_h.Original(tp);
			splitscreen::RestoreViewPort();
		}
		else
		{
			E102DispTimeUpWarning_h.Original(tp);
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
		case MD_E102_CART: // Allow death incarts
			if ((twp->flag & Status_DoNextAction) && twp->smode == PL_OP_KILLED)
			{
				twp->mode = MD_E102_KILL;
				pwp->mj.reqaction = 69;
			}
			break;
		case MD_MULTI_PARA:
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
		case MD_MULTI_S6A1_WAIT:
			if (E102CheckInput(pwp, twp, mwp) || E102CheckJump(pwp, twp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_MD_MULTI_S6A1_WAITChanges(twp, pwp);
			return;
		case MD_MULTI_S6A1_SLID:

			if (E102CheckInput(pwp, twp, mwp) || E102CheckJump(pwp, twp))
			{
				pwp->htp = 0;
				return;
			}

			Mode_MD_MULTI_S6A1_SLIDChanges(twp, pwp);

			return;
		case MD_MULTI_S6A1_LROT:
			if (E102CheckInput(pwp, twp, mwp) || E102CheckJump(pwp, twp))
			{
				pwp->htp = 0;
				return;
			}

			if (Controllers[TASKWK_PLAYERID(twp)].LeftStickX << 8 <= -3072)
			{
				if (twp->mode < MD_MULTI_S6A1_WAIT || twp->mode > MD_MULTI_S6A1_RROT)
				{
					pwp->htp = 0;
				}

				return;
			}

			twp->mode = MD_MULTI_S6A1_WAIT;
			return;
		case MD_MULTI_S6A1_RROT:
			if (E102CheckInput(pwp, twp, mwp) || E102CheckJump(pwp, twp))
			{
				pwp->htp = 0;
				return;
			}

			if (Controllers[TASKWK_PLAYERID(twp)].LeftStickX << 8 >= 3072)
			{
				if (twp->mode < MD_MULTI_S6A1_WAIT || twp->mode > MD_MULTI_S6A1_RROT)
				{
					pwp->htp = 0;
				}
				return;
			}

			twp->mode = MD_MULTI_S6A1_WAIT;
			return;
		}
	}

	E102_RunsActions_h.Original(tp, mwp, pwp);
}

signed int E102_CheckInput_r(taskwk* twp, playerwk* pwp, motionwk2* mwp)
{
	if (multiplayer::IsActive())
	{
		auto even = twp->ewp;
		auto pnum = TASKWK_PLAYERID(twp);

		if (even->move.mode || even->path.list || ((twp->flag & Status_DoNextAction) == 0))
		{
			return E102_CheckInput_h.Original(twp, pwp, mwp);
		}

		switch (twp->smode)
		{
		case PL_OP_PARABOLIC:
			if (CurrentLevel != LevelIDs_Casinopolis)
			{
				twp->mode = MD_MULTI_PARA;
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

	return E102_CheckInput_h.Original(twp, pwp, mwp);
}

static void __cdecl E102Display_r(task* tp);
FastFunctionHookPtr<decltype(&E102Display_r)> E102Display_h(0x47FD50, E102Display_r);
static void __cdecl E102Display_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pwp = (playerwk*)tp->mwp->work.ptr;
		auto ewk = (E102WK*)pwp->free.ptr[0];

		// splitscreen compatibility patches
		if (splitscreen::IsActive())
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

	E102Display_h.Original(tp);
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
		E102_h.Original(tp);
		e102_hover_flag_m[pnum] = e102_hover_flag;
		e102_hover_flag_p_m[pnum] = e102_hover_flag_p;
		e102_hover_flag = backup;
		e102_hover_flag_p = backup_p;
	}
	else
	{
		E102_h.Original(tp);
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
		E102_h.Original(tp);
	}
}

static void __cdecl E102LockOnCursor_r(task* tp)
{
	e102_work_ptr = (E102WK*)tp->awp[1].work.ul[0];
	E102LockOnCursor_h.Original(tp);
}

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

	E102Beam_h.Original(tp);
}

static void __cdecl E102AddSecTotalNewDisplay_r(task* tp)
{
	if (IsCountingDown())
	{
		if (splitscreen::IsActive())
		{
			splitscreen::SaveViewPort();
			splitscreen::ChangeViewPort(-1);
			E102AddSecTotalNewDisplay_h.Original(tp);
			splitscreen::RestoreViewPort();
		}
		else
		{
			E102AddSecTotalNewDisplay_h.Original(tp);
		}
	}
}

static void __cdecl E102AddSecTotalNew_r(task* tp)
{
	if (IsCountingDown())
	{
		E102AddSecTotalNew_h.Original(tp);
	}
}

void patch_e102_init()
{
	E102_h.Hook(E102_r);
	E102_CheckInput_h.Hook(E102_CheckInput_r);
	E102_RunsActions_h.Hook(E102_RunActions_r);
	E102DispTimeUpWarning_h.Hook(E102DispTimeUpWarning_r);
	E102LockOnCursor_h.Hook(E102LockOnCursor_r);
	E102Beam_h.Hook(E102Beam_r);
	E102AddSecTotalNewDisplay_h.Hook(E102AddSecTotalNewDisplay_r);
	E102AddSecTotalNew_h.Hook(E102AddSecTotalNew_r);
	WriteCall((void*)0x49FD54, E102AddSeconds_r);
	WriteCall((void*)0x47FC17, E102TimeOverHook);
	WriteCall((void*)0x426081, GammaTickTimePatch);
}

RegisterPatch patch_e102(patch_e102_init);
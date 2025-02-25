#include "pch.h"
#include "multiplayer.h"

// Patch ObjectJumpPanel for multiplayer
// Had to patch some player logic too
// To-do: simultaneity

static void __cdecl ObjectJumpPanelCollision_r(task* tp);
static signed int CanIMakePanelJump_r(taskwk* twp);
static void StartPlayerPanelJump_r(taskwk* twp);
static Bool CheckCollisionedForJumpPanel_r(taskwk* twp);

FastUsercallHookPtr<decltype(&ObjectJumpPanelCollision_r), noret, rEAX> ObjectJumpPanelCollision_t(0x4B8600, ObjectJumpPanelCollision_r);
FastFunctionHookPtr<decltype(&CanIMakePanelJump_r)> CanIMakePanelJump_t(0x4B83F0, CanIMakePanelJump_r);
FastFunctionHookPtr<decltype(&StartPlayerPanelJump_r)> StartPlayerPanelJump_t(0x4B8470, StartPlayerPanelJump_r);
FastFunctionHookPtr<decltype(&CheckCollisionedForJumpPanel_r)> CheckCollisionedForJumpPanel_t(0x4B83C0, CheckCollisionedForJumpPanel_r);

DataArray(task*, jumppanel_tp_list, 0x3C5A27C, 10);

static taskwk* CCL_IsHitPanel(taskwk* twp)
{
	auto cwp = twp->cwp;

	for (int i = 0; i < 16; ++i)
	{
		auto hit_info = &cwp->hit_info[i];

		if (hit_info->hit_num == -1)
		{
			return nullptr;
		}

		auto hit_twp = hit_info->hit_twp;

		if (hit_twp->id == 8)
		{
			return hit_twp;
		}
	}

	return nullptr;
}

static Bool AreJumpPanelsActive()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			switch (TASKWK_CHARID(ptwp))
			{
			case Characters_Sonic:
			case Characters_Tails:
				if (ptwp->mode == 40 || ptwp->mode == 41)
				{
					return true;
				}
				break;
			case Characters_Knuckles:
			case Characters_Amy:
				if (ptwp->mode == 46 || ptwp->mode == 47)
				{
					return true;
				}
				break;
			}
		}
	}

	return false;
}

#pragma region CheckCollisionedForJumpPanel
static Bool __cdecl CheckCollisionedForJumpPanel_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		return CCL_IsHitPanel(twp) != nullptr;
	}
	else
	{
		return CheckCollisionedForJumpPanel_t.Original(twp);
	}
}
#pragma endregion

#pragma region StartPlayerPanelJump
static void StartPlayerPanelJump_m(taskwk* twp)
{
	auto cwp = twp->cwp;

	ResetJumpPanels();

	auto panel_twp = CCL_IsHitPanel(twp);

	if (panel_twp)
	{
		auto panel_num = panel_twp->wtimer;

		panel_twp->mode = 1;

		if (panel_num == 9 || !JumpPanelList[panel_num + 1])
		{
			jumppanel_tp_list[0]->twp->mode = 2;
		}
		else
		{
			jumppanel_tp_list[panel_num + 1]->twp->mode = 2;
		}
	}
}

static void __cdecl StartPlayerPanelJump_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		StartPlayerPanelJump_m(twp);
	}
	else
	{
		StartPlayerPanelJump_t.Original(twp);
	}
}
#pragma endregion

#pragma region CanIMakePanelJump
static signed int CanIMakePanelJump_m(taskwk* twp)
{
	auto panel_twp = CCL_IsHitPanel(twp);
	auto panel_active = AreJumpPanelsActive();

	if (panel_twp)
	{
		// Disable if a player is already on the jump panel chain
		if (panel_active && panel_twp->wtimer == 1)
		{
			return 0;
		}

		if (panel_twp->wtimer)
		{
			if (panel_twp->timer.l < 30)
			{
				return 1;
			}
		}
		else
		{
			ResetJumpPanels();
			return -1;
		}
	}

	// Only reset if no player are on the jump panel chain
	if (!panel_active)
		ResetJumpPanels();

	return 0;
}

static signed int __cdecl CanIMakePanelJump_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		return CanIMakePanelJump_m(twp);
	}
	else
	{
		return CanIMakePanelJump_t.Original(twp);
	}
}
#pragma endregion

#pragma region ObjectJumpPanelCollision
static void __cdecl ObjectJumpPanelCollision_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		if (twp->wtimer < 2)
		{
			auto ptwp = CCL_IsHitPlayer(twp);

			if (ptwp)
			{
				if (twp->timer.l < 3600)
				{
					++twp->timer.l;
				}
			}
			else
			{
				twp->timer.l = 0;
			}
		}
	}
	else
	{
		ObjectJumpPanelCollision_t.Original(tp);
	}
}
#pragma endregion
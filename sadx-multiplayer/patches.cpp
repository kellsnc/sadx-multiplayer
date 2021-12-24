#include "pch.h"
#include "deathzones.h"
#include "camera.h"

/*

General patches to allow compatibility for 4+ players

*/

Trampoline* PGetRotation_t        = nullptr;
Trampoline* GetPlayersInputData_t = nullptr;
Trampoline* PInitialize_t         = nullptr;
Trampoline* NpcMilesSet_t         = nullptr;
Trampoline* Ring_t                = nullptr;
Trampoline* EnemyCheckDamage_t    = nullptr;

// Patch forward calculation to use multiplayer cameras
void __cdecl PGetRotation_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (IsMultiplayerEnabled() && camera_twp)
	{
		auto backup = camera_twp->ang;
		camera_twp->ang = *GetCameraAngle(TASKWK_PLAYERID(twp));
		TARGET_DYNAMIC(PGetRotation)(twp, mwp, pwp);
		camera_twp->ang = backup;
	}
	else
	{
		TARGET_DYNAMIC(PGetRotation)(twp, mwp, pwp);
	}
}

// Patch analog forward calculation to use multiplayer cameras
void __cdecl GetPlayersInputData_r()
{
	if (!IsMultiplayerEnabled())
	{
		TARGET_DYNAMIC(GetPlayersInputData)();
		return;
	}

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto controller = per[i];
		float lx = (controller->x1 << 8); // left stick x
		float ly = (controller->y1 << 8); // left stick y

		int ang;
		float strk;

		if (lx > 3072.0f || lx < -3072.0f || ly > 3072.0f || ly < -3072.0f)
		{
			lx = lx <= 3072.0f ? (lx >= -3072.0f ? 0.0f : lx + 3072.0f) : lx - 3072.0f;
			ly = ly <= 3072.0f ? (ly >= -3072.0f ? 0.0f : ly + 3072.0f) : ly - 3072.0f;

			strk = atan2f(ly, lx) * 65536.0f;

			if (camera_twp)
			{
				ang = -(GetCameraAngle(i)->y) - (strk * -0.1591549762031479);
			}
			else
			{
				ang = (strk * 0.1591549762031479);
			}

			float magnitude = ly * ly + lx * lx;
			strk = sqrtf(magnitude) * magnitude * 3.9187027e-14;
			if (strk > 1.0f)
			{
				strk = 1.0f;
			}
		}
		else
		{
			strk = 0.0f;
			ang = 0;
		}

		input_data[i] = { ang, strk };

		if (ucInputStatus == 1 && (i >= 4 || ucInputStatusForEachPlayer[i] == 1))
		{
			input_dataG[i] = input_data[i];
		}
		else
		{
			input_dataG[i] = { 0, 0.0f };
		}
	}
}

// Remove ability to be hurt by players
void RemovePlayersDamage(taskwk* twp)
{
	if (twp && twp->cwp)
	{
		for (int i = 0; i < twp->cwp->nbInfo; i++)
		{
			twp->cwp->info[i].damage &= ~0x20u;
		}
	}
}

// Patch to prevent characters from hurting each others (todo: differenciate coop and battle)
void PInitialize_r(int no, task* tp)
{
	TARGET_DYNAMIC(PInitialize)(no, tp);

	if (IsMultiplayerEnabled())
	{
		RemovePlayersDamage(tp->twp);
	}
}

// Patch to prevent 2P Tails to load (note: charsel incompatible)
void __cdecl NpcMilesSet_r(task* tp)
{
	if (!IsMultiplayerEnabled())
	{
		TARGET_DYNAMIC(NpcMilesSet)(tp);
	}
}

// Patch for other players to collect rings
void __cdecl Ring_r(task* tp)
{
	if (IsMultiplayerEnabled())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1)
		{
			auto player = CCL_IsHitPlayer(twp);

			if (player)
			{
				int pID = TASKWK_PLAYERID(player);

				if (!(playerpwp[pID]->item & 0x4000))
				{
					twp->mode = 2;
					AddNumRingM(pID, 1);
					dsPlay_oneshot(7, 0, 0, 0);
					tp->disp = RingDoneDisplayer;
					return;
				}
			}
		}
	}

	TARGET_DYNAMIC(Ring)(tp);
}

// Patch for other players to get kill score
BOOL __cdecl EnemyCheckDamage_r(taskwk* twp, enemywk* ewp)
{
	if (!IsMultiplayerEnabled())
	{
		return TARGET_DYNAMIC(EnemyCheckDamage)(twp, ewp);
	}

	if (twp->flag & 4)
	{
		ewp->buyoscale = 0.35f;

		auto player = CCL_IsHitPlayer(twp);

		if (player)
		{
			int pID = TASKWK_PLAYERID(player);

			ewp->flag |= 0x1000u;
			AddEnemyScoreM(pID, 10);

			if ((twp->cwp->hit_cwp->info[twp->cwp->hit_num].attr & 0x1000) != 0)
			{
				ewp->flag |= 0x800u;
				ewp->velo.y = 0.7f;
			}
			else
			{
				ewp->flag &= ~0x400u;
				EnemyBumpPlayer(pID);
			}

			return TRUE;
		}
		else if (CCL_IsHitBullet(twp))
		{
			ewp->flag |= 0x1000u;

			if ((twp->cwp->hit_cwp->info[twp->cwp->hit_num].attr & 0x1000) != 0)
			{
				ewp->flag |= 0x800u;
			}
			else
			{
				ewp->flag &= ~0x400u;
			}

			return TRUE;
		}

		return FALSE;
	}
	else
	{
		return CheckItemExplosion(&twp->pos);
	}
}

// EBuyon is the only object to manually call AddEnemyScore
void __cdecl EBuyon_ScorePatch(task* tp)
{
	if (IsMultiplayerEnabled())
	{
		auto player = CCL_IsHitPlayer(tp->twp);

		if (player)
		{
			AddEnemyScoreM(TASKWK_PLAYERID(player), 100);
		}
	}
	else
	{
		AddEnemyScore(100);
	}
}

int __cdecl CheckCollisionP_r(NJS_POINT3* vp, float d)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto twp = playertwp[i];

		if (twp && twp->cwp)
		{
			NJS_VECTOR v = twp->cwp->info->center;
			njSubVector(&v, vp);

			if (njScalor(&v) < d)
			{
				return i + 1;
			}
		}
	}

	return 0;
}

int __cdecl CheckCollisionCylinderP_r(NJS_POINT3* vp, float r, float h)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto twp = playertwp[i];

		if (twp)
		{
			NJS_VECTOR v = twp->pos;
			njSubVector(&v, vp);

			if (v.x * v.x + v.z * v.z - r * r <= 0.0f)
			{
				if (fabsf(v.y) <= h)
				{
					return i + 1;
				}
			}
		}
	}

	return 0;
}

void InitPatches()
{
	PGetRotation_t        = new Trampoline(0x44BB60, 0x44BB68, PGetRotation_r);
	GetPlayersInputData_t = new Trampoline(0x40F170, 0x40F175, GetPlayersInputData_r);
	PInitialize_t         = new Trampoline(0x442750, 0x442755, PInitialize_r);
	//NpcMilesSet_t       = new Trampoline(0x47ED60, 0x47ED65, NpcMilesSet_r);
	Ring_t                = new Trampoline(0x450370, 0x450375, Ring_r);
	EnemyCheckDamage_t    = new Trampoline(0x4CE030, 0x4CE036, EnemyCheckDamage_r);

	// EBuyon score patch:
	WriteCall((void*)0x7B3273, EBuyon_ScorePatch);
	WriteData<5>((void*)0x7B326D, 0x90);

	// Collision checks:
	WriteJump(CheckCollisionP, CheckCollisionP_r);
	WriteJump(CheckCollisionCylinderP, CheckCollisionCylinderP_r);
}
#include "pch.h"
#include "deathzones.h"
#include "camera.h"
#include "splitscreen.h"

/*

General patches to allow compatibility for 4+ players

*/

Trampoline* PGetRotation_t         = nullptr;
Trampoline* GetPlayersInputData_t  = nullptr;
Trampoline* PInitialize_t          = nullptr;
Trampoline* NpcMilesSet_t          = nullptr;
Trampoline* Ring_t                 = nullptr;
Trampoline* EnemyCheckDamage_t     = nullptr;
Trampoline* ProcessStatusTable_t   = nullptr;
Trampoline* CheckRangeOutWithR_t   = nullptr;
Trampoline* EnemyDist2FromPlayer_t = nullptr;
Trampoline* EnemyCalcPlayerAngle_t = nullptr;
Trampoline* savepointCollision_t   = nullptr;
Trampoline* TikalDisplay_t         = nullptr;
Trampoline* CheckPlayerRideOnMobileLandObjectP_t = nullptr;

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

void CreateSetTask(OBJ_CONDITION* item, _OBJ_EDITENTRY* objentry, _OBJ_ITEMENTRY* objinfo, float distance)
{
	if (item->ssCondition & 1)
	{
		return;
	}

	auto tp = CreateElementalTask(objinfo->ucInitMode, objinfo->ucLevel, objinfo->fnExec);
	item->ssCondition |= 1; // we're loaded!
	++item->scCount;
	tp->ocp = item;
	auto twp = tp->twp;

	if (twp)
	{
		if (item->ssCondition & 2)
		{
			auto objsocd = item->unionStatus.pObjSleepCondition;
			twp->pos = objsocd->pos;
			twp->ang = objsocd->ang;
			twp->scl = objsocd->scl;
			FreeMemory(objsocd);
			item->ssCondition &= ~2u;
		}
		else
		{
			twp->pos.x = objentry->xpos;
			twp->pos.y = objentry->ypos;
			twp->pos.z = objentry->zpos;
			twp->ang.x = objentry->rotx;
			twp->ang.y = objentry->roty;
			twp->ang.z = objentry->rotz;
			twp->scl.x = objentry->xscl;
			twp->scl.y = objentry->yscl;
			twp->scl.z = objentry->zscl;
		}
	}

	item->unionStatus.fRangeOut = distance; // tell CheckRangeOut when to unload object
	item->pTask = tp;
}

// Load objects around every players
void __cdecl ProcessStatusTable_r()
{
	if (IsMultiplayerEnabled())
	{
		for (int i = 0; i < numStatusEntry; ++i)
		{
			auto& item = objStatusEntry[i];

			// Object already loaded
			if (item.ssCondition >= 0 || (item.ssCondition & 1))
			{
				continue;
			}

			auto objentry = item.pObjEditEntry;

			// If the object id is beyond the object count
			int count = objentry->usID & 0xFFF;
			if (count != 0 && pObjItemTable->ssCount <= count)
			{
				continue;
			}

			int cliplevel = (objentry->usID >> 12) & 7;

			if (cliplevel)
			{
				if (cliplevel == 1)
				{
					if (ClipLevel >= 1)
					{
						continue;
					}
				}
				else if (ClipLevel >= 2)
				{
					continue;
				}
			}

			auto& objinfo = pObjItemTable->pObjItemEntry[objentry->usID & 0xFFF];

			if (objinfo.ssAttribute & 2)
			{
				CreateSetTask(&item, objentry, &objinfo, 0.0f);
			}
			else
			{
				float dist = (objinfo.ssAttribute & 1) ? objinfo.fRange : 160000.0f;

				if (boolOneShot == FALSE && objinfo.ssAttribute & 4)
				{
					CreateSetTask(&item, objentry, &objinfo, dist);
					continue;
				}

				for (int i = 0; i < PLAYER_MAX; ++i)
				{
					if (!playertwp[i]) continue;

					NJS_POINT3 pos = camera_twp ? *GetCameraPosition(i) : playertwp[i]->pos;
					njSubVector(&pos, (NJS_POINT3*)&objentry->xpos);

					if (njScalor2(&pos) < dist)
					{
						CreateSetTask(&item, objentry, &objinfo, dist);
						break;
					}
				}
			}
		}

		boolOneShot = TRUE;
	}
	else
	{
		TARGET_DYNAMIC(ProcessStatusTable)();
	}
}

// Check object deletion for every player
BOOL __cdecl CheckRangeOutWithR_r(task* tp, float fRange)
{
	if (IsMultiplayerEnabled())
	{
		// Do not delete if the no delete flag is set
		if (tp->ocp && (tp->ocp->ssCondition & 8))
		{
			return FALSE;
		}

		// Do not delete if range is 0
		if (fRange == 0.0f)
		{
			return FALSE;
		}

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (playertp[i])
			{
				NJS_POINT3 pos = *GetCameraPosition(i);
				njSubVector(&pos, &tp->twp->pos);

				if (njScalor2(&pos) < fRange)
				{
					return FALSE;
				}
			}
		}

		tp->exec = FreeTask;
		return TRUE;
	}
	else
	{
		return TARGET_DYNAMIC(CheckRangeOutWithR)(tp, fRange);
	}
}

// Despite taking player id, it always gets 0 so let's check closest player
float __cdecl EnemyDist2FromPlayer_r(taskwk* twp, int num)
{
	if (IsMultiplayerEnabled() && num == 0)
	{
		return TARGET_DYNAMIC(EnemyDist2FromPlayer)(twp, GetTheNearestPlayerNumber(&twp->pos));
	}
	else
	{
		return TARGET_DYNAMIC(EnemyDist2FromPlayer)(twp, num);
	}
}

// Despite taking player id, it always gets 0 so let's check closest player
Angle __cdecl EnemyCalcPlayerAngle_r(taskwk* twp, enemywk* ewp, unsigned __int8 pnum)
{
	if (IsMultiplayerEnabled() && pnum == 0)
	{
		return TARGET_DYNAMIC(EnemyCalcPlayerAngle)(twp, ewp, GetTheNearestPlayerNumber(&twp->pos));
	}
	else
	{
		return TARGET_DYNAMIC(EnemyCalcPlayerAngle)(twp, ewp, pnum);
	}
}

// inlined in symbols
float savepointGetSpeedM(taskwk* twp, int pID)
{
	if (!playerpwp[pID])
	{
		return 0.0f;
	}
	
	float spd = njScalor(&playerpwp[pID]->spd);

	if (DiffAngle(0x4000 - EntityData1Ptrs[0]->Rotation.y, twp->ang.y) <= 0x4000)
	{
		return spd;
	}
	else
	{
		return -spd;
	}
}

// Patch checkpoints to work for every players
void __cdecl savepointCollision_r(task* tp, taskwk* twp)
{
	if (IsMultiplayerEnabled())
	{
		savepoint_data->tp[0]->twp->ang.x = twp->ang.x + savepoint_data->ang.x;
		savepoint_data->tp[0]->twp->ang.y = twp->ang.y + savepoint_data->ang.y;
		savepoint_data->tp[0]->twp->ang.z = twp->ang.z + savepoint_data->ang.z;
		savepoint_data->tp[1]->twp->ang.x = twp->ang.x + savepoint_data->ang.x;
		savepoint_data->tp[1]->twp->ang.y = twp->ang.y + savepoint_data->ang.y;
		savepoint_data->tp[1]->twp->ang.z = twp->ang.z + savepoint_data->ang.z;

		if (twp->mode == 1)
		{
			auto entity = CCL_IsHitPlayer(twp);

			if (entity)
			{
				twp->mode = 2;
				int pID = TASKWK_PLAYERID(entity);
				savepoint_data->write_timer = 300;
				savepoint_data->ang_spd.y = ((savepointGetSpeedM(twp, pID) * 10.0f) * 65536.0f * 0.0028f);
				updateContinueData(&entity->pos, &entity->ang);
				SetBroken(tp);
				dsPlay_oneshot(10, 0, 0, 0);
			}
		}
		
		EntryColliList(twp);
	}
	else
	{
		auto target = TARGET_DYNAMIC(savepointCollision);

		__asm
		{
			mov esi, [twp]
			mov edi, [tp]
			call target
		}
	}
}

static void __declspec(naked) savepointCollision_w()
{
	__asm
	{
		push esi // twp
		push edi // tp
		call savepointCollision_r
		pop edi // tp
		pop esi // twp
		retn
	}
}

// Patch mobile land detection to detect every player
bool CheckPlayerRideOnMobileLandObjectP_r(unsigned __int8 pno, task* ttp)
{
	if (IsMultiplayerEnabled() && pno == 0)
	{
		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (TARGET_DYNAMIC(CheckPlayerRideOnMobileLandObjectP)(i, ttp))
			{
				return true;
			}
		}

		return false;
	}
	else
	{
		return TARGET_DYNAMIC(CheckPlayerRideOnMobileLandObjectP)(pno, ttp);
	}
}

// Trick the game to draw shadows on other screens
BOOL IsGamePausedOrNot1stScreen()
{
	return IsGamePaused() || SplitScreen::numScreen != 0;
}

// Add shadow rendering in Tikal's display routine because they forgot it
void TikalDisplay_r(task* tp)
{
	TARGET_DYNAMIC(TikalDisplay)(tp);

	if (IsGamePausedOrNot1stScreen())
	{
		DrawCharacterShadow(tp->twp, &((playerwk*)tp->mwp->work.ptr)->shadow);
	}
}

void InitPatches()
{
	PGetRotation_t          = new Trampoline(0x44BB60, 0x44BB68, PGetRotation_r);
	GetPlayersInputData_t   = new Trampoline(0x40F170, 0x40F175, GetPlayersInputData_r);
	PInitialize_t           = new Trampoline(0x442750, 0x442755, PInitialize_r);
	//NpcMilesSet_t         = new Trampoline(0x47ED60, 0x47ED65, NpcMilesSet_r);
	Ring_t                  = new Trampoline(0x450370, 0x450375, Ring_r);
	EnemyCheckDamage_t      = new Trampoline(0x4CE030, 0x4CE036, EnemyCheckDamage_r);
	ProcessStatusTable_t    = new Trampoline(0x46BCE0, 0x46BCE5, ProcessStatusTable_r);
	CheckRangeOutWithR_t    = new Trampoline(0x46C010, 0x46C018, CheckRangeOutWithR_r);
	EnemyDist2FromPlayer_t  = new Trampoline(0x4CD610, 0x4CD61B, EnemyDist2FromPlayer_r);
	EnemyCalcPlayerAngle_t  = new Trampoline(0x4CD670, 0x4CD675, EnemyCalcPlayerAngle_r);
	savepointCollision_t    = new Trampoline(0x44F430, 0x44F435, savepointCollision_w);
	CheckPlayerRideOnMobileLandObjectP_t = new Trampoline(0x441C30, 0x441C35, CheckPlayerRideOnMobileLandObjectP_r);

	// EBuyon score patch:
	WriteCall((void*)0x7B3273, EBuyon_ScorePatch);
	WriteData<5>((void*)0x7B326D, 0x90);

	// Collision checks:
	WriteJump(CheckCollisionP, CheckCollisionP_r);
	WriteJump(CheckCollisionCylinderP, CheckCollisionCylinderP_r);

	// EnemySearchPlayer
	WriteData((char*)0x4CCB3F, (char)PLAYER_MAX);

	// Patch Skyboxes (display function managing mode)
	WriteData((void**)0x4F723E, (void*)0x4F71A0); // Emerald Coast
	WriteData((void**)0x4DDBFE, (void*)0x4DDB60); // Windy Valley
	WriteData((void**)0x61D57E, (void*)0x61D4E0); // Twinkle Park
	WriteData((void**)0x610A7E, (void*)0x6109E0); // Speed Highway
	WriteData((void**)0x5E1FCE, (void*)0x5E1F30); // Lost World
	WriteData((void**)0x4EA26E, (void*)0x4EA1D0); // Ice Cap

	// Character shadows:
	// Game draws shadow in logic sub but also in display sub *if* game is paused.
	// To keep compatibility with mods (like SA2 Sonic), I just force the display for the other screens.
	// Better alternative is to skip display in logic and always draw in display, but I chose max compatibility.
	WriteCall((void*)0x494B57, IsGamePausedOrNot1stScreen); // Sonic
	WriteCall((void*)0x461420, IsGamePausedOrNot1stScreen); // Tails
	WriteCall((void*)0x472674, IsGamePausedOrNot1stScreen); // Knuckles
	WriteCall((void*)0x4875F9, IsGamePausedOrNot1stScreen); // Amy
	WriteCall((void*)0x48BA5A, IsGamePausedOrNot1stScreen); // Big
	WriteCall((void*)0x480702, IsGamePausedOrNot1stScreen); // Gamma
	TikalDisplay_t = new Trampoline(0x7B33A0, 0x7B33A5, TikalDisplay_r);
}
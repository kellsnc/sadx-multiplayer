#include "pch.h"
#include "death.h"
#include "camera.h"
#include "splitscreen.h"

#include "objects/ObjectItemBox.h"
#include "objects/SnowBoard.h"

/*

General patches to allow compatibility for 4+ players

*/

Trampoline* PGetRotation_t         = nullptr;
Trampoline* GetPlayersInputData_t  = nullptr;
Trampoline* PInitialize_t          = nullptr;
Trampoline* NpcMilesSet_t          = nullptr;
Trampoline* Ring_t                 = nullptr;
Trampoline* PlayerVacumedRing_t    = nullptr;
Trampoline* EnemyCheckDamage_t     = nullptr;
Trampoline* EnemyDist2FromPlayer_t = nullptr;
Trampoline* EnemyCalcPlayerAngle_t = nullptr;
Trampoline* EnemyTurnToPlayer_t    = nullptr;
Trampoline* savepointCollision_t   = nullptr;
Trampoline* TikalDisplay_t         = nullptr;
Trampoline* ObjectSpringB_t        = nullptr;
Trampoline* SpinnaDisplayer_t      = nullptr;
Trampoline* CheckPlayerRideOnMobileLandObjectP_t = nullptr;
Trampoline* MakeLandCollLandEntryRangeIn_t = nullptr;

// Patch forward calculation to use multiplayer cameras
void __cdecl PGetRotation_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (SplitScreen::IsActive())
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			TARGET_DYNAMIC(PGetRotation)(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	TARGET_DYNAMIC(PGetRotation)(twp, mwp, pwp);
}

// Patch analog forward calculation to use multiplayer cameras
void __cdecl GetPlayersInputData_r()
{
	if (!SplitScreen::IsActive())
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

			auto cam_ang = GetCameraAngle(i);

			if (cam_ang)
			{
				ang = -cam_ang->y - (strk * -0.1591549762031479);
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

// Patch for other players to collect rings
void __cdecl Ring_r(task* tp)
{
	if (multiplayer::IsActive())
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
	if (!multiplayer::IsActive())
	{
		return TARGET_DYNAMIC(EnemyCheckDamage)(twp, ewp);
	}

	if (twp->flag & 4)
	{
		ewp->buyoscale = 0.35f;

		auto hit_twp = CCL_IsHitPlayer(twp);

		if (hit_twp)
		{
			auto pID = TASKWK_PLAYERID(hit_twp);

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

		hit_twp = CCL_IsHitBullet(twp);

		if (hit_twp)
		{
			ewp->flag |= 0x1000u;
			AddEnemyScoreM(hit_twp->btimer, 10);

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
	if (multiplayer::IsActive())
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
	return IsPlayerInSphere(vp, d);
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

// Despite taking player id, it always gets 0 so let's check closest player
float __cdecl EnemyDist2FromPlayer_r(taskwk* twp, int num)
{
	if (multiplayer::IsActive() && num == 0)
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
	if (multiplayer::IsActive() && pnum == 0)
	{
		return TARGET_DYNAMIC(EnemyCalcPlayerAngle)(twp, ewp, GetTheNearestPlayerNumber(&twp->pos));
	}
	else
	{
		return TARGET_DYNAMIC(EnemyCalcPlayerAngle)(twp, ewp, pnum);
	}
}

// Despite taking player id, it always gets 0 so let's check closest player
Angle __cdecl EnemyTurnToPlayer_r(taskwk* twp, enemywk* ewp, unsigned __int8 pnum)
{
	if (multiplayer::IsActive() && pnum == 0)
	{
		return TARGET_DYNAMIC(EnemyTurnToPlayer)(twp, ewp, GetTheNearestPlayerNumber(&twp->pos));
	}
	else
	{
		return TARGET_DYNAMIC(EnemyTurnToPlayer)(twp, ewp, pnum);
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
	if (multiplayer::IsActive())
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
	if (multiplayer::IsActive() && pno == 0)
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

// Patch for this to work properly for 4+ players
void ObjectSpringB_r(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 1 && twp->btimer > 10)
	{
		auto entity = CCL_IsHitPlayer(twp);

		if (entity)
		{
			twp->smode = TASKWK_PLAYERID(entity);
			twp->scl.z = 20.0f;
			twp->timer.f = 270.0f;
			twp->counter.f = 0.1f;
			twp->mode = 2;
		}
	}

	TARGET_DYNAMIC(ObjectSpringB)(tp);
}

void QueueMobileLandColl() // guessed inline function
{
	// Temporary because somehow this decomp doesn't work:
	MakeLandCollLandEntryALL();
	return;

	ri_landcoll_nmb = 0;

	njPushMatrixEx();

	for (int i = 0; i < numMobileEntry; ++i)
	{
		if (numLandCollList >= 1024 || ri_landcoll_nmb >= 128)
		{
			break;
		}

		auto& entry = MobileEntry[i];
		auto& tp = entry.pTask;
		auto& obj = entry.pObject;
		NJS_VECTOR p;

		// Task disabled collision
		if (tp && tp->twp && tp->twp->flag & 0x100)
		{
			continue;
		}

		if (entry.slAttribute & ColFlags_UseRotation)
		{
			njUnitMatrix(0);
			if (obj->ang[2]) njRotateZ(0, obj->ang[2]);
			if (obj->ang[1]) njRotateX(0, obj->ang[1]);
			if (obj->ang[0]) njRotateY(0, obj->ang[0]);
			njCalcPoint(0, &obj->basicdxmodel->center, &p);
			p.x += obj->pos[0];
			p.y += obj->pos[1];
			p.z += obj->pos[2];
		}
		else
		{
			p.x = obj->basicdxmodel->center.x + obj->pos[0];
			p.y = obj->basicdxmodel->center.y + obj->pos[1];
			p.z = obj->basicdxmodel->center.z + obj->pos[2];
		}

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			taskwk* srctwp = i == 0 ? playertp[0] == nullptr ? camera_twp : playertwp[0] : playertwp[i];

			if (!srctwp)
			{
				continue;
			}

			NJS_VECTOR cv
			{
				srctwp->pos.x - p.x,
				srctwp->pos.y - p.y,
				srctwp->pos.z - p.z
			};

			if (njScalor(&cv) - mleriRangeRad < obj->basicdxmodel->r)
			{
				ri_landcoll[ri_landcoll_nmb++] = entry;
				LandCollList[numLandCollList++] = entry; // add entry to active list

				break;
			}
		}
	}

	njPopMatrixEx();
}

void QueueLandCollLand() // guessed inline function
{
	ri_landentry_nmb = 0;

	if (boolLandCollision == TRUE && pObjLandTable)
	{
		for (int i = 0; i < pObjLandTable->ssCount; ++i)
		{
			if (numLandCollList >= 1024 || ri_landentry_nmb >= 128)
			{
				break;
			}

			auto& lnd = pObjLandTable->pLandEntry[i];

			if ((lnd.slAttribute & 0x400003) == 0)
			{
				continue;
			}

			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				taskwk* srctwp = i == 0 ? playertp[0] == nullptr ? camera_twp : playertwp[0] : playertwp[i];

				if (srctwp == nullptr)
				{
					continue;
				}

				NJS_VECTOR cv
				{
					lnd.xCenter - srctwp->pos.x,
					lnd.yCenter - srctwp->pos.y,
					lnd.zCenter - srctwp->pos.z
				};

				if (njScalor(&cv) - mleriRangeRad < lnd.xWidth)
				{
					ri_landentry_buf[ri_landentry_nmb++] = lnd;
					LandCollList[numLandCollList++] = { lnd.slAttribute, lnd.pObject, nullptr };
					break;
				}
			}
		}
	}
}

void __cdecl MakeLandCollLandEntryRangeInM()
{
	numLandCollList = 0;
	QueueMobileLandColl();
	QueueLandCollLand();
}

// Geometry collision lookup is hardcoded around P1 and P2, patching it for more
void __cdecl MakeLandCollLandEntryRangeIn_r()
{
	if (multiplayer::IsActive() && multiplayer::GetPlayerCount() > 2 )
	{
		MakeLandCollLandEntryRangeInM();
	}
	else
	{
		TARGET_DYNAMIC(MakeLandCollLandEntryRangeIn)();
	}
}

static BOOL PlayerVacumedRing_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		// Get closest players with magnetic field
		taskwk* pltwp_ = nullptr;
		playerwk* plpwp = nullptr;
		float dist = 10000000.0f;

		for (int i = 0; i < multiplayer::GetPlayerCount(); ++i)
		{
			auto pltwp = playertwp[i];
			plpwp = playerpwp[i];

			if (pltwp && plpwp && plpwp->item & Powerups_MagneticBarrier)
			{
				NJS_VECTOR v
				{
					twp->pos.x - pltwp->pos.x,
					twp->pos.y - pltwp->pos.y,
					twp->pos.z - pltwp->pos.z
				};

				auto curdist = njScalor(&v);

				if (curdist < dist)
				{
					dist = curdist;
					pltwp_ = pltwp;
				}
			}
		}

		// found one
		if (pltwp_ && (dist < 50.0f || twp->wtimer))
		{
			NJS_VECTOR dir = { 0.0f, 7.0f, 0.0f };
			njPushMatrix(_nj_unit_matrix_);
			if (pltwp_->ang.z) njRotateZ(0, pltwp_->ang.z);
			if (pltwp_->ang.x) njRotateX(0, pltwp_->ang.x);
			if (pltwp_->ang.y) njRotateY(0, pltwp_->ang.y);
			njCalcPoint(0, &dir, &dir);
			njPopMatrixEx();

			dir.x += pltwp_->pos.x;
			dir.y += pltwp_->pos.y;
			dir.z += pltwp_->pos.z;

			// clamp
			if (dist > 50.0f)
			{
				dist = 50.0f;
			}

			dist = min(5.0f, max(0.85f, dist * 0.026f));

			if (plpwp)
			{
				dist *= (njScalor(&plpwp->spd) * 0.5f + 1.0f);
			}

			CalcAdvanceAsPossible(&twp->pos, &dir, dist, &twp->pos);
			++twp->wtimer;

			twp->counter.f = twp->counter.f + 3.0f;
			EntryColliList(twp);
			return TRUE;
		}

		return FALSE;
	}
	else
	{
		auto target = TARGET_DYNAMIC(PlayerVacumedRing);
		BOOL result;
		__asm
		{
			mov esi, [twp]
			call target
			mov result, eax
		}
		return result;
	}
}

static void __declspec(naked) PlayerVacumedRing_w()
{
	__asm
	{
		push esi
		call PlayerVacumedRing_r
		pop esi
		retn
	}
}

void SpinnaDrawShield(taskwk* twp)
{
	for (int i = 0; i < 16; ++i)
	{
		Angle mod1 = ((0.5 - rand() * 0.000030517578) * 180.0 * 65536.0 * 0.002777777777777778);
		Angle mod2 = (rand() * 0.000030517578 * 360.0 * 65536.0 * 0.002777777777777778);
		float mod3 = (rand() * 0.000030517578 + 0.5) * 10.0;
		
		NJS_VECTOR posm;
		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		NJS_VECTOR pos1 = twp->pos;
		NJS_VECTOR pos2 = twp->pos;

		njAddVector(&pos1, &posm);

		pos1.y += 5.0f;
		pos2.y += 5.0f;
		DrawLineV(&pos2, &pos1);

		pos2 = pos1;
		mod1 = ((0.5 - rand() * 0.000030517578) * 120.0 * 65536.0 * 0.002777777777777778) + mod1;
		mod2 = ((0.5 - rand() * 0.000030517578) * 80.0 * 65536.0 * 0.002777777777777778) + mod2;
		mod3 = (rand() * 0.000030517578 + 1.0) * 4.0;
		
		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		njAddVector(&pos1, &posm);
		DrawLineV(&pos2, &pos1);

		pos2 = pos1;
		mod1 = ((0.5 - rand() * 0.000030517578) * 120.0 * 65536.0 * 0.002777777777777778) + mod1;
		mod2 = ((0.5 - rand() * 0.000030517578) * 80.0 * 65536.0 * 0.002777777777777778) + mod2;
		auto rnd = rand();
		mod3 = rnd * 0.000030517578 + 1.0 + rnd * 0.000030517578 + 1.0;
		
		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		njAddVector(&pos1, &posm);
		DrawLineV(&pos2, &pos1);
	}
}

void __cdecl SpinnaDisplayer_r(task* tp)
{
	if (SplitScreen::IsActive())
	{
		if (!MissedFrames)
		{
			auto twp = tp->twp;

			SpinnaDraw(twp, (enemywk*)tp->mwp);

			if (SplitScreen::GetCurrentScreenNum() > 0 && twp->smode == 0 && twp->wtimer != 0)
			{
				SpinnaDrawShield(twp);
			}
		}
	}
	else
	{
		TARGET_DYNAMIC(SpinnaDisplayer)(tp);
	}
}

void PadReadOnP_r(int8_t pnum)
{
	if (pnum == -1)
	{
		for (int i = 0; i < ucInputStatusForEachPlayer_Length; ++i)
		{
			ucInputStatusForEachPlayer[i] = TRUE;
		}
	}
	else
	{
		ucInputStatusForEachPlayer[pnum] = TRUE;
	}
}

void PadReadOffP_r(int8_t pnum)
{
	if (pnum < 0)
	{
		for (int i = 0; i < ucInputStatusForEachPlayer_Length; ++i)
		{
			ucInputStatusForEachPlayer[i] = FALSE;
		}
	}
	else
	{
		ucInputStatusForEachPlayer[pnum] = FALSE;
	}
}

BOOL dsCheckViewV_r(NJS_POINT3* ft, float radius)
{
	if (multiplayer::IsActive())
	{
		return TRUE;
	}
	else
	{
		return dsCheckViewV(ft, radius);
	}
}

void InitPatches()
{
	PGetRotation_t          = new Trampoline(0x44BB60, 0x44BB68, PGetRotation_r);
	GetPlayersInputData_t   = new Trampoline(0x40F170, 0x40F175, GetPlayersInputData_r);
	Ring_t                  = new Trampoline(0x450370, 0x450375, Ring_r);
	PlayerVacumedRing_t     = new Trampoline(0x44FA90, 0x44FA96, PlayerVacumedRing_w);
	savepointCollision_t    = new Trampoline(0x44F430, 0x44F435, savepointCollision_w);
	CheckPlayerRideOnMobileLandObjectP_t = new Trampoline(0x441C30, 0x441C35, CheckPlayerRideOnMobileLandObjectP_r);
	MakeLandCollLandEntryRangeIn_t = new Trampoline(0x43AEF0, 0x43AEF5, MakeLandCollLandEntryRangeIn_r);

	// Score patches
	EnemyCheckDamage_t = new Trampoline(0x4CE030, 0x4CE036, EnemyCheckDamage_r);
	WriteCall((void*)0x7B3273, EBuyon_ScorePatch); // EBuyon: add 100 points to proper player
	WriteData<5>((void*)0x7B326D, 0x90); // EBuyon: remove original 100 points for player 0

	// Collision checks:
	WriteJump(CheckCollisionP, CheckCollisionP_r);
	WriteJump(CheckCollisionCylinderP, CheckCollisionCylinderP_r);

	// Enemy player checks
	EnemyDist2FromPlayer_t = new Trampoline(0x4CD610, 0x4CD61B, EnemyDist2FromPlayer_r);
	EnemyCalcPlayerAngle_t = new Trampoline(0x4CD670, 0x4CD675, EnemyCalcPlayerAngle_r);
	EnemyTurnToPlayer_t    = new Trampoline(0x4CD6F0, 0x4CD6F5, EnemyTurnToPlayer_r);
	WriteData((char*)0x4CCB3F, (char)PLAYER_MAX); // EnemySearchPlayer

	// Enemies
	SpinnaDisplayer_t = new Trampoline(0x4AFD80, 0x4AFD85, SpinnaDisplayer_r);

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

	// Springs for 4+ players
	ObjectSpringB_t = new Trampoline(0x7A4E50, 0x7A4E55, ObjectSpringB_r);
	WriteData((uint8_t*)0x7A4DC4, (uint8_t)PLAYER_MAX); // ObjectSpring
	WriteData((uint8_t*)0x79F77C, (uint8_t)PLAYER_MAX); // spring_h_exec

	// Fix controller toggles
	WriteJump(PadReadOnP, PadReadOnP_r);
	WriteJump(PadReadOffP, PadReadOffP_r);

	// dsCheckViewV in exec functions
	WriteCall((void*)0x4E138F, dsCheckViewV_r); // wv hane, bigfloot, saku...
	WriteCall((void*)0x4DF5C8, dsCheckViewV_r); // ObjectWindySetInEff
	WriteCall((void*)0x5D3D54, dsCheckViewV_r); // ObjectCasinoCris
	WriteCall((void*)0x5E8327, dsCheckViewV_r); // ObjectRuinFire

	InitItemBoxPatches();
	InitSnowBoardPatches();
}
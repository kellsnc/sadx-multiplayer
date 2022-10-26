#include "pch.h"
#include "death.h"
#include "camera.h"
#include "splitscreen.h"
#include "objects/bosses.h"

#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"

#include "objects/ObjectItemBox.h"
#include "objects/ObjectRocket.h"
#include "objects/SnowBoard.h"
#include "objects/EnemyWindyE103.h"
#include "objects/EnemyMountainE104.h"
#include "objects/EnemyHotE105.h"
#include "objects/ef_spark.h"
#include "objects/ObjShelterKaitenKey.h"
#include "objects/mirror.h"
#include "objects/e_pondskater.h"
#include "objects/e_gachapon.h"
#include "objects/ladder.h"
#include "objects/ObjCylinderCmn.h"
#include "objects/o_sky_ocm.h"

#include "objects/sonic.h"
#include "objects//miles.h"
#include "objects/knuckles.h"
#include "objects/amy.h"
#include "objects/gamma.h"
#include "objects/big.h"
#include "objects/o_sky_cannon_s.h"
#include "objects/Sw_Door.h"

/*

General patches to allow compatibility for 4+ players

*/

static Trampoline* PGetRotation_t = nullptr;
static Trampoline* PGetAcceleration_t = nullptr;
static Trampoline* PGetAccelerationSnowBoard_t = nullptr;
static Trampoline* Ring_t = nullptr;
static Trampoline* Tobitiri_t = nullptr;
static Trampoline* EnemyDist2FromPlayer_t = nullptr;
static Trampoline* EnemyCalcPlayerAngle_t = nullptr;
static Trampoline* EnemyTurnToPlayer_t = nullptr;
static Trampoline* savepointCollision_t = nullptr;
static Trampoline* TikalDisplay_t = nullptr;
static Trampoline* ObjectSpringB_t = nullptr;
static Trampoline* SpinnaDisplayer_t = nullptr;
static Trampoline* ListGroundForDrawing_t = nullptr;
static FunctionHook<int, taskwk*, enemywk*> EnemyCheckDamage_t((intptr_t)OhNoImDead);
static FunctionHook<task*, NJS_POINT3*, NJS_POINT3*, float> SetCircleLimit_t(0x7AF3E0);
UsercallFuncVoid(SonicMotionCheckEdition, (taskwk* twp), (twp), 0x492170, rESI);
UsercallFunc(signed int, PlayerVacumedRing_t, (taskwk* a1), (a1), 0x44FA90, rEAX, rEDI);

void __cdecl PGetRotation_r(taskwk* twp, motionwk2* mwp, playerwk* pwp) // todo: rewrite
{
	if (SplitScreen::IsActive() && pwp->attr & 0x20000)
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

void __cdecl PGetAcceleration_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (SplitScreen::IsActive() && pwp->attr & 0x20000)
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			TARGET_DYNAMIC(PGetAcceleration)(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	TARGET_DYNAMIC(PGetAcceleration)(twp, mwp, pwp);
}

void __cdecl PGetAccelerationSnowBoard_r(taskwk* twp, motionwk2* mwp, playerwk* pwp, float Max_Speed)
{
	if (SplitScreen::IsActive())
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			TARGET_DYNAMIC(PGetAccelerationSnowBoard)(twp, mwp, pwp, Max_Speed);
			camera_twp->ang = backup;
			return;
		}
	}

	TARGET_DYNAMIC(PGetAccelerationSnowBoard)(twp, mwp, pwp, Max_Speed);
}

void __cdecl SonicMotionCheckEdition_r(taskwk* twp)
{
	if (SplitScreen::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(twp);
		auto& per = perG[pnum];

		if (per.press & Buttons_B)
		{
			SetPlayerInitialPosition(twp);
		}

		Float lx = (per.x1 << 8);
		Float ly = (per.y1 << 8);

		if (per.on & Buttons_X)
		{
			if (lx > 3072.0f || lx < -3072.0f || ly > 3072.0f || ly < -3072.0f)
			{
				if (ly == 0.0f)
				{
					twp->pos.y = twp->pos.y - 0.0 * 5.0;
				}
				else
				{
					twp->pos.y = twp->pos.y - ly / njSqrt(ly * ly) * 5.0f;
				}
			}
		}
		else
		{
			if (lx > 3072.0 || lx < -3072.0 || ly > 3072.0f || ly < -3072.0f)
			{
				auto camera_ang = GetCameraAngle(pnum);

				if (!camera_ang)
				{
					camera_ang = &camera_twp->ang;
				}

				Float ang = -camera_ang->y - -njArcTan2(ly, lx);
				twp->pos.x = njCos(ang) * 5.0f + twp->pos.x;
				twp->pos.z = njSin(ang) * 5.0f + twp->pos.z;
			}
		}
	}
	else
	{
		SonicMotionCheckEdition.Original(twp);
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

// Patch for other players to collect scattered rings
void __cdecl Tobitiri_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		taskwk* twp = tp->twp;

		if (twp->mode == 1 || twp->mode == 2)
		{
			auto player = CCL_IsHitPlayer(twp);

			if (player)
			{
				twp->mode = 4;
				twp->pos.y += 3.44f;
				twp->scl.x = -2.0f;
				twp->scl.z = -4.0f;
				twp->counter.l = 0;

				ResetParticle((EntityData1*)twp, (NJS_SPRITE*)0x3B42FC0);

				int pID = TASKWK_PLAYERID(player);

				if (!(playerpwp[pID]->item & 0x4000))
				{
					AddNumRingM(pID, 1);
					dsPlay_oneshot(7, 0, 0, 0);
					tp->disp = RingDoneDisplayer;
					return;
				}
			}
		}
	}

	TARGET_DYNAMIC(Tobitiri)(tp);
}

// Patch for other players to get kill score
BOOL __cdecl EnemyCheckDamage_r(taskwk* twp, enemywk* ewp)
{
	if (!multiplayer::IsActive())
	{
		return EnemyCheckDamage_t.Original(twp, ewp);
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

// Despite taking player id, it always gets 0 so let's check closest player
float __cdecl EnemyDist2FromPlayer_r(taskwk* twp, int num)
{
	if (multiplayer::IsActive() && num == 0)
	{
		return TARGET_DYNAMIC(EnemyDist2FromPlayer)(twp, GetClosestPlayerNum(&twp->pos));
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
		return TARGET_DYNAMIC(EnemyCalcPlayerAngle)(twp, ewp, GetClosestPlayerNum(&twp->pos));
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
		return TARGET_DYNAMIC(EnemyTurnToPlayer)(twp, ewp, GetClosestPlayerNum(&twp->pos));
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
				savepoint_data->ang_spd.y = static_cast<Angle>((savepointGetSpeedM(twp, pID) * 10.0f) * 65536.0f * 0.0028f);
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

static BOOL PlayerVacumedRing_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		// Get closest players with magnetic field
		taskwk* pltwp_ = nullptr;
		playerwk* plpwp = nullptr;
		float dist = 10000000.0f;

		for (unsigned int i = 0ui32; i < multiplayer::GetPlayerCount(); ++i)
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
		return PlayerVacumedRing_t.Original(twp);
	}
}

void SpinnaDrawShield(taskwk* twp)
{
	for (int i = 0; i < 16; ++i)
	{
		Angle mod1 = NJM_DEG_ANG(180.0 * (0.5 - njRandom()));
		Angle mod2 = NJM_DEG_ANG(360.0 * njRandom());
		float mod3 = static_cast<float>((njRandom() + 0.5) * 10.0);

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
		mod1 += NJM_DEG_ANG(120.0 * (0.5 - njRandom()));
		mod2 += NJM_DEG_ANG(80.0 * (0.5 - njRandom()));
		mod3 = static_cast<float>((njRandom() + 1.0) * 4.0);

		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		njAddVector(&pos1, &posm);
		DrawLineV(&pos2, &pos1);

		pos2 = pos1;
		mod1 += NJM_DEG_ANG(120.0 * (0.5 - njRandom()));
		mod2 += NJM_DEG_ANG(80.0 * (0.5 - njRandom()));
		mod3 = static_cast<float>((njRandom() + 1.0) * 2.0);

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

BOOL __cdecl dsCheckViewV_r(NJS_POINT3* ft, float radius)
{
	if (multiplayer::IsActive())
	{
		return true;
	}
	else
	{
		return dsCheckViewV(ft, radius);
	}
}

void __cdecl ListGroundForDrawing_r()
{
	if (SplitScreen::IsActive())
	{
		auto cam_pos = GetCameraPosition(SplitScreen::GetCurrentScreenNum());
		auto cam_ang = GetCameraAngle(SplitScreen::GetCurrentScreenNum());

		if (!cam_pos || !cam_ang)
		{
			return;
		}

		numDisplayEntry = 0;

		NJS_POINT3 center = { 0.0f, 0.0f, MaxDrawDistance * -0.5f };

		njPushMatrix(_nj_unit_matrix_);
		njRotateY_(cam_ang->y);
		njRotateX_(cam_ang->x);
		njCalcPoint(0, &center, &center);
		njPopMatrixEx();

		center.x += cam_pos->x;
		center.y += cam_pos->y;
		center.z += cam_pos->z;

		for (int i = 0; i < pObjLandTable->ssCount; ++i)
		{
			auto col = &pObjLandTable->pLandEntry[i];

			if (!col->blockbit || col->blockbit & MaskBlock)
			{
				if (col->slAttribute & ColFlags_Visible)
				{
					if (col->slAttribute & ColFlags_UseSkyDrawDist || GetDistance(&center, (NJS_POINT3*)&col->xCenter) < col->xWidth + MaxDrawDistance)
					{
						pDisplayEntry[numDisplayEntry++] = col;
					}
				}
			}
		}
	}
	else
	{
		TARGET_DYNAMIC(ListGroundForDrawing)();
	}
}

static void __cdecl InitTimer_r()
{
	TimeFrames = 0;
	TimeSeconds = 0;
	TimeMinutes = 0;
	GameTimer = 0;

	if (GetPlayerNumber() == Characters_Gamma && !multiplayer::IsBattleMode())
	{
		TimeMinutes = 3;
		TimeSeconds = 0;
	}
}

static void __cdecl InitTime_r()
{
	ulGlobalTimer = 0;
	InitTimer_r();
}

static int __cdecl GammaTickTimePatch()
{
	return multiplayer::IsBattleMode() ? 0 : CurrentCharacter;
}

bool CheckAnyPlayerRideOnMobileLandObjectP(unsigned __int8 pno, task* ttp)
{
	if (multiplayer::IsActive())
	{
		return IsPlayerOnDyncol(ttp);
	}
	else
	{
		return CheckPlayerRideOnMobileLandObjectP(pno, ttp);
	}
}

task* __cdecl SetCircleLimit_r(NJS_POINT3* pPos, NJS_POINT3* a2, float radius)
{
	if (multiplayer::IsActive()) {
		for (unsigned int i = 1; i < multiplayer::GetPlayerCount(); i++)
		{
			task* obj = CreateElementalTask((LoadObj_Data1), 0, CircleLimit);

			if (obj && playertwp[i])
			{
				auto data = obj->twp;
				data->pos = *a2;
				data->scl.x = radius;
				data->counter.l = (unsigned int)&playertwp[i]->pos;
			}
		}
	}

	return SetCircleLimit_t.Original(pPos, a2, radius);
}

void __cdecl HoldOnIcicleP_r(Uint8 pno, task* ttp)
{
	auto ptwp = playertwp[pno];

	if (!(ptwp->flag & Status_HoldObject))
	{
		SetFreeCameraMode_m(pno, FALSE);
		SetInputP(pno, PL_OP_HOLDONICICLE);
		playerpwp[pno]->htp = ttp;
	}
}

void initBossesPatches()
{
	InitE103Patches();
	InitE104Patches();
	InitE105Patches();
	initChaos0Patches();
	initEggHornetPatches();
	initEggWalkerPatches();
}

void InitPatches()
{
	Ring_t = new Trampoline(0x450370, 0x450375, Ring_r);
	Tobitiri_t = new Trampoline(0x44FD10, 0x44FD18, Tobitiri_r);;
	PlayerVacumedRing_t.Hook(PlayerVacumedRing_r);
	savepointCollision_t = new Trampoline(0x44F430, 0x44F435, savepointCollision_w);
	ListGroundForDrawing_t = new Trampoline(0x43A900, 0x43A905, ListGroundForDrawing_r);

	// Player
	PGetRotation_t = new Trampoline(0x44BB60, 0x44BB68, PGetRotation_r);
	PGetAcceleration_t = new Trampoline(0x44C270, 0x44C278, PGetAcceleration_r);
	PGetAccelerationSnowBoard_t = new Trampoline(0x448550, 0x448558, PGetAccelerationSnowBoard_r);
	SonicMotionCheckEdition.Hook(SonicMotionCheckEdition_r);

	// Misc
	WriteJump(HoldOnIcicleP, HoldOnIcicleP_r); // Disable free camera for the proper player on icicles
	WriteJump(LoadPlayerMotionData, _advertise_prolog); // Fix missing animations with testspawn
	WriteData((uint8_t*)0x500017, (uint8_t)PLAYER_MAX); // Patch launch ramp EC for 8 players

	// Score patches
	EnemyCheckDamage_t.Hook(EnemyCheckDamage_r);
	WriteCall((void*)0x7B3273, EBuyon_ScorePatch); // EBuyon: add 100 points to proper player
	WriteData<5>((void*)0x7B326D, 0x90); // EBuyon: remove original 100 points for player 0

	// Enemy player checks
	EnemyDist2FromPlayer_t = new Trampoline(0x4CD610, 0x4CD61B, EnemyDist2FromPlayer_r);
	EnemyCalcPlayerAngle_t = new Trampoline(0x4CD670, 0x4CD675, EnemyCalcPlayerAngle_r);
	EnemyTurnToPlayer_t = new Trampoline(0x4CD6F0, 0x4CD6F5, EnemyTurnToPlayer_r);
	WriteData((char*)0x4CCB3F, (char)PLAYER_MAX); // EnemySearchPlayer

	// Enemies
	SpinnaDisplayer_t = new Trampoline(0x4AFD80, 0x4AFD85, SpinnaDisplayer_r);

	// Bosses
	SetCircleLimit_t.Hook(SetCircleLimit_r);

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
	WriteData((uint8_t*)0x7A4DC4, (uint8_t)PLAYER_MAX); // ObjectSpring
	WriteData((uint8_t*)0x7A4FF7, (uint8_t)PLAYER_MAX); // ObjectSpringB
	WriteData((uint8_t*)0x79F77C, (uint8_t)PLAYER_MAX); // spring_h_exec

	// dsCheckViewV in exec functions
	WriteCall((void*)0x4E138F, dsCheckViewV_r); // wv hane, bigfloot, saku...
	WriteCall((void*)0x5D3D54, dsCheckViewV_r); // ObjectCasinoCris
	WriteCall((void*)0x5E8327, dsCheckViewV_r); // ObjectRuinFire
	WriteCall((void*)0x4AB41A, dsCheckViewV_r); // EnemySaru
	WriteCall((void*)0x7AA307, dsCheckViewV_r); // EnemyPondSkater

	// Normal timer for Gamma in multiplayer
	WriteJump((void*)0x425FF0, InitTimer_r);
	WriteJump((void*)0x427F10, InitTime_r);
	WriteCall((void*)0x426081, GammaTickTimePatch);

	// Patch CheckPlayerRideOnMobileLandObjectP occurences that don't need full rewrites
	WriteCall((void*)0x4CB36C, CheckAnyPlayerRideOnMobileLandObjectP); // Switch
	WriteCall((void*)0x52130E, CheckAnyPlayerRideOnMobileLandObjectP); // OStation
	WriteCall((void*)0x522F7E, CheckAnyPlayerRideOnMobileLandObjectP); // OSidelift
	WriteCall((void*)0x523185, CheckAnyPlayerRideOnMobileLandObjectP); // OSidelift
	WriteCall((void*)0x526AAE, CheckAnyPlayerRideOnMobileLandObjectP); // OBlift
	WriteCall((void*)0x52A5CE, CheckAnyPlayerRideOnMobileLandObjectP); // OStation
	WriteCall((void*)0x59DC87, CheckAnyPlayerRideOnMobileLandObjectP); // OUkijima
	WriteCall((void*)0x59DCA0, CheckAnyPlayerRideOnMobileLandObjectP); // OUkijima
	WriteCall((void*)0x5AC110, CheckAnyPlayerRideOnMobileLandObjectP); // Cargo
	WriteCall((void*)0x5AC153, CheckAnyPlayerRideOnMobileLandObjectP); // Cargo
	WriteCall((void*)0x5E6B26, CheckAnyPlayerRideOnMobileLandObjectP); // Aokiswitch
	WriteCall((void*)0x5FA25C, CheckAnyPlayerRideOnMobileLandObjectP); // Edge
	WriteCall((void*)0x5E6B26, CheckAnyPlayerRideOnMobileLandObjectP); // Connect
	WriteCall((void*)0x5E6B26, CheckAnyPlayerRideOnMobileLandObjectP); // Talap
	WriteCall((void*)0x601FB0, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63D865, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63D90D, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63D940, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63DFB6, CheckAnyPlayerRideOnMobileLandObjectP);
	WriteCall((void*)0x63DFED, CheckAnyPlayerRideOnMobileLandObjectP);

	InitItemBoxPatches();
	InitSnowBoardPatches();
	initBossesPatches();
	PatchEffectSpark();
	PatchAmyHandles();
	PatchTwinkleMirrors();
	PatchPondSkater();
	PatchGachapon();
	initLadderPatches();
	initSDIntroPatches();
	init_SDCylinderPatches();
	PatchSkyDeckOCM();
	init_SwDoorPatch();
	initSonicPatch();
	initMilesPatches();
	Init_KnuxPatches();
	Init_AmyPatches();
	Init_BigPatches();
	initGammaPatch();
	PatchRocket();
}

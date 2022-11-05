#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"

#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"

UsercallFuncVoid(CCL_CheckHoming_h, (taskwk* twp1, taskwk* twp2, float dist2), (twp1, twp2, dist2), 0x418BE0, rECX, rEDX, stack4);
UsercallFuncVoid(pLockingOnTargetEnemy2_h, (taskwk* twp, motionwk2* mwp, playerwk* pwp), (twp, mwp, pwp), 0x7984B0, rEDI, rEBX, stack4);
UsercallFuncVoid(SonicHomingOnTarget_h, (taskwk* twp, playerwk* pwp, motionwk2* mwp), (twp, pwp, mwp), 0x492300, rESI, rEDI, stack4);
UsercallFuncVoid(SonicHomeOnTarget_h, (taskwk* twp, playerwk* pwp, motionwk2* mwp), (twp, pwp, mwp), 0x494B80, rECX, rEAX, stack4);
UsercallFuncVoid(miles_oneshot, (taskwk* twp, Sint32 tone), (twp, tone), 0x45BDF0, rEAX, stack4);
UsercallFunc(Bool, MilesCheckFinishThrowObject_h, (taskwk* twp, playerwk* pwp), (twp, pwp), 0x45D0F0, rEAX, rESI, stack4);
UsercallFunc(Bool, KnucklesCheckFinishThrowObject_h, (taskwk* twp, motionwk2* mwp, playerwk* pwp), (twp, mwp, pwp), 0x4738A0, rEAX, rESI, stack4, stack4);
FunctionHook<taskwk*, taskwk*> CCL_IsHitPlayer_h(0x41CBC0);
FunctionHook<void> CCL_ClearAll_h(0x41C680);
FunctionHook<void> CCL_Analyze_h(0x420700);
FunctionHook<void, taskwk*, motionwk2*, playerwk*> LockingOnTargetEnemy_h(0x44C1C0);
FunctionHook<Bool, taskwk*> BigCheckTargetEnemy_h(0x46EE40);
FunctionHook<Sint32, Uint8> GetRivalPlayerNumber_h(0x441BF0);

FunctionHook<void> MakeLandCollLandEntryRangeIn_h(0x43AEF0);

static _OBJ_LANDENTRY ri_landentry_buf_ex[256]; // 256 instead of 128

static colaround around_ring_list_p2[257];
static colaround around_ring_list_p3[257];
static colaround around_enemy_list_p2[657];
static colaround around_enemy_list_p3[657];

static Uint16 arl_num2, arl_num3, ael_num2, ael_num3;

colaround* around_ring_list_p[]{
	around_ring_list_p0,
	around_ring_list_p1,
	around_ring_list_p2,
	around_ring_list_p3
};

colaround* around_enemy_list_p[]{
	around_enemy_list_p0,
	around_enemy_list_p1,
	around_enemy_list_p2,
	around_enemy_list_p3
};

Uint16* arl_num_p[]{
	&arl_num0,
	&arl_num1,
	&arl_num2,
	&arl_num3
};

Uint16* ael_num_p[]{
	&ael_num0,
	&ael_num1,
	&ael_num2,
	&ael_num3
};

void QueueMobileLandColl() // guessed inline function
{
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
		if (tp && tp->twp && !(tp->twp->flag & 0x100))
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

			if (njScalor2(&cv) < (obj->basicdxmodel->r + mleriRangeRad) * (obj->basicdxmodel->r + mleriRangeRad))
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
			if (numLandCollList >= 1024 || ri_landentry_nmb >= LengthOfArray(ri_landentry_buf_ex))
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
					ri_landentry_buf_ex[ri_landentry_nmb++] = lnd;
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
	if (multiplayer::IsActive() && multiplayer::GetPlayerCount() > 2)
	{
		MakeLandCollLandEntryRangeInM();
	}
	else
	{
		MakeLandCollLandEntryRangeIn_h.Original();
	}
}

taskwk* __cdecl CCL_IsHitPlayer_r(taskwk* twp)
{
	CCL_ClearSearch(); // <- needed when called by the same task twice, which actually happens vanilla
	return CCL_IsHitPlayer_h.Original(twp);
}

int __cdecl CheckCollisionP_r(NJS_POINT3* vp, Float d)
{
	return IsPlayerInSphere(vp, d);
}

int __cdecl CheckCollisionCylinderP_r(NJS_POINT3* vp, Float r, Float h)
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

void __cdecl CCL_CheckHoming_r(taskwk* twp1, taskwk* twp2, Float dist2)
{
	if (multiplayer::IsActive())
	{
		if (dist2 < 10000.0f && !twp1->cwp->id && (twp2->cwp->flag & 0x40))
		{
			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				auto ptwp = playertwp[i];

				if (!ptwp || twp1 != ptwp)
					continue;

				auto& arl_num = *arl_num_p[i];
				auto& ael_num = *ael_num_p[i];
				auto& around_ring_list = around_ring_list_p[i];
				auto& around_enemy_list = around_enemy_list_p[i];

				if (twp2->id == ID_I_RING)
				{
					if (arl_num < 256)
					{
						around_ring_list[arl_num] = { twp2, dist2 };
						around_ring_list[arl_num + 1].twp = nullptr;
						++arl_num;
					}
				}
				else
				{
					if (ael_num < 656)
					{
						around_enemy_list[ael_num] = { twp2, dist2 };
						around_enemy_list[ael_num + 1].twp = nullptr;
						++ael_num;
					}
				}
			}
		}
	}
	else
	{
		CCL_CheckHoming_h.Original(twp1, twp2, dist2);
	}
}

void ClearExtraHomingLists()
{
	arl_num2 = 0;
	arl_num3 = 0;
	ael_num2 = 0;
	ael_num3 = 0;
	around_ring_list_p2[0].twp = nullptr;
	around_ring_list_p3[0].twp = nullptr;
	around_enemy_list_p2[0].twp = nullptr;
	around_enemy_list_p3[0].twp = nullptr;
}

void __cdecl CCL_ClearAll_r()
{
	CCL_ClearAll_h.Original();
	ClearExtraHomingLists();
}

void __cdecl CCL_Analyze_r()
{
	ClearExtraHomingLists();
	CCL_Analyze_h.Original();
}

colaround* GetTargetEnemyList(Uint8 pno)
{
	return pno < PLAYER_MAX ? around_enemy_list_p[pno] : nullptr;
}

Bool __cdecl PCheckTargetEnemy_r(Uint8 pno)
{
	return pno < PLAYER_MAX&& around_enemy_list_p[pno]->twp != nullptr;
}

Uint16* __cdecl getAelNum(Uint8 pno)
{
	return pno < PLAYER_MAX ? ael_num_p[pno] : nullptr;
}

void __cdecl LockingOnTargetEnemy_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (!multiplayer::IsActive())
	{
		LockingOnTargetEnemy_h.Original(twp, mwp, pwp);
		return;
	}

	auto pnum = TASKWK_PLAYERID(twp);
	if (PCheckTargetEnemy_r(pnum))
	{
		auto ael = around_enemy_list_p[pnum];

		auto closest_tgt = ael->twp;
		auto closest_dist = ael->dist;

		do
		{
			if (closest_dist > ael->dist)
			{
				closest_tgt = ael->twp;
				closest_dist = ael->dist;
			}
			++ael;
		} while (ael->twp);

		mwp->ang_aim.y = njArcTan2(closest_tgt->cwp->info->center.z + closest_tgt->pos.z - twp->cwp->info->center.z,
			closest_tgt->cwp->info->center.x + closest_tgt->pos.x - twp->cwp->info->center.x);
		twp->ang.y = AdjustAngle(twp->ang.y, mwp->ang_aim.y, 0x1000);
	}
	else
	{
		PGetRotation(twp, mwp, pwp);
	}
}

void __cdecl pLockingOnTargetEnemy2_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (!multiplayer::IsActive())
	{
		pLockingOnTargetEnemy2_h.Original(twp, mwp, pwp);
		return;
	}

	auto pnum = TASKWK_PLAYERID(twp);
	auto ael = GetTargetEnemyList(pnum);
	if (ael)
	{
		auto closest_tgt = ael->twp;
		auto closest_dist = ael->dist;

		do
		{
			if (closest_dist > ael->dist)
			{
				closest_tgt = ael->twp;
				closest_dist = ael->dist;
			}
			++ael;
		} while (ael->twp);

		if (twp->cwp->info->kind % 4)
		{
			PGetRotation(twp, mwp, pwp);
		}
		else
		{
			if (closest_tgt) {
				mwp->ang_aim.y = njArcTan2(closest_tgt->cwp->info->center.z + closest_tgt->pos.z - twp->cwp->info->center.z,
					closest_tgt->cwp->info->center.x + closest_tgt->pos.x - twp->cwp->info->center.x);
			}
			twp->ang.y = AdjustAngle(twp->ang.y, mwp->ang_aim.y, 0x1000);
		}
	}
	else
	{
		PGetRotation(twp, mwp, pwp);
	}
}

Bool __cdecl BigCheckTargetEnemy_r(taskwk* twp)
{
	if (!multiplayer::IsActive())
	{
		return BigCheckTargetEnemy_h.Original(twp);
	}

	auto pnum = TASKWK_PLAYERID(twp);
	auto ael = GetTargetEnemyList(pnum);
	if (ael)
	{
		auto closest_dist = ael->dist;
		do
		{
			if (closest_dist > ael->dist)
			{
				closest_dist = ael->dist;
			}
			++ael;
		} while (ael->twp);

		return closest_dist < 2500.0f;
	}

	return 0;
}

Sint32 __cdecl GetRivalPlayerNumber_r(Uint8 pno)
{
	if (multiplayer::IsBattleMode())
	{
		auto twp1 = playertwp[pno];

		if (!twp1)
		{
			return -1;
		}

		float closest_dist = 100.0f;
		int closest_id = -1;

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (i == pno)
				continue;

			auto twp2 = playertwp[i];

			if (!twp2)
				continue;

			auto dist2 = GetDistance(&twp1->pos, &twp2->pos);

			if (dist2 < closest_dist)
			{
				closest_dist = dist2;
				closest_id = i;
			}
		}

		return closest_id;
	}
	else
	{
		return GetRivalPlayerNumber_h.Original(pno);
	}
}

void __cdecl SonicHomingOnTarget_r(taskwk* twp, playerwk* pwp, motionwk2* mwp)
{
	if (!multiplayer::IsActive())
	{
		SonicHomingOnTarget_h.Original(twp, pwp, mwp);
		return;
	}

	auto pnum = TASKWK_PLAYERID(twp);

	if (pnum >= PLAYER_MAX)
		return;

	auto ael = GetTargetEnemyList(pnum);
	auto rival = GetRivalPlayerNumber_r(pnum);

	if (pwp->free.sw[2] <= 0 && ((ael && ael->twp) || rival != -1))
	{
		taskwk* closest_tgt = nullptr;
		auto closest_dist = 10000.0;

		// Get rival
		if (rival != -1)
		{
			NJS_POINT3 pos = twp->pos;
			njSubVector(&pos, &playertwp[rival]->pos);
			auto dist = njScalor2(&pos);

			if (dist < 10000.0f && DiffAngle(twp->ang.y, njArcTan2(-pos.z, -pos.x)) < 0x5000)
			{
				closest_tgt = playertwp[rival];
				closest_dist = dist;
			}
		}

		// Get closest targetable entity
		if (ael->twp)
		{
			do
			{
				auto info = ael->twp->cwp->info;
				auto pos = info->center;
				if (!(info->attr & 0x20))
				{
					njAddVector(&pos, &ael->twp->pos);
				}
				njSubVector(&pos, &twp->pos);

				if ((pwp->equipment & Upgrades_SuperSonic) ||
					DiffAngle(twp->ang.y, njArcTan2(pos.z, pos.x)) <= 0x5000)
				{
					if (closest_dist > ael->dist)
					{
						closest_tgt = ael->twp;
						closest_dist = ael->dist;
					}
				}

				++ael;
			} while (ael->twp);
		}

		// Do stuff with match
		if (closest_tgt)
		{
			auto info = closest_tgt->cwp->info;
			auto pos = info->center;
			if (!(info->attr & 0x20))
			{
				njAddVector(&pos, &closest_tgt->pos);
			}
			njSubVector(&pos, &twp->pos);

			if ((pwp->equipment & (unsigned __int16)Upgrades_SuperSonic) != 0)
			{
				twp->ang.y = AdjustAngle(twp->ang.y, njArcTan2(pos.z, pos.x), 0x1000);
			}
			else
			{
				twp->ang.y = AdjustAngle(twp->ang.y, njArcTan2(pos.z, pos.x), 0x800);
			}

			if (pos.y > 0.0f && pwp->free.sw[2] < 0)
			{
				pos.y = 0.0f;
				pwp->free.sw[2] = 1;
			}

			auto len = sqrtf(pos.z * pos.z + pos.y * pos.y + pos.x * pos.x);
			if (len == 0.0f)
			{
				memset(&pos, 0, sizeof(pos));
			}
			else
			{
				len = 1.0f / len;
				pos.x *= len;
				pos.y *= len;
				pos.z *= len;
			}

			pos.x = njCos(twp->ang.y) * njSqrt(1.0f - pos.y * pos.y);
			pos.z = njSin(twp->ang.y) * njSqrt(1.0f - pos.y * pos.y);

			Float r = (pwp->equipment & Upgrades_SuperSonic) ? 10.0f : 5.0f;

			if (pwp->free.sw[3] > 180)
			{
				r *= (njRandom() * 0.1f + 0.7f);
			}

			pos.x *= r;
			pos.y *= r;
			pos.z *= r;

			mwp->spd = pos; // world
			PConvertVector_G2P(twp, &pos);
			pwp->spd = pos; // local

			if (pwp->free.sw[2] < 0)
			{
				pwp->free.sw[2] = 0;
			}
		}
		else
		{
			pwp->free.sw[2] = 1;
			PGetAcceleration(twp, mwp, pwp);
			PGetSpeed(twp, mwp, pwp);
		}
	}
	else
	{
		++pwp->free.sw[2];
		pwp->spd.x *= 0.9f;
		PGetAcceleration(twp, mwp, pwp);
		PGetSpeed(twp, mwp, pwp);
	}
}

void __cdecl SonicHomeOnTarget_r(taskwk* twp, playerwk* pwp, motionwk2* mwp)
{
	if (!multiplayer::IsActive())
	{
		SonicHomeOnTarget_h.Original(twp, pwp, mwp);
		return;
	}

	auto pnum = TASKWK_PLAYERID(twp);

	task* effect = CreateElementalTask(2, LEV_5, (pwp->equipment & Upgrades_SuperSonic) ? (TaskFuncPtr)0x55FB50 : (TaskFuncPtr)0x4A2A70);
	if (effect)
	{
		TASKWK_PLAYERID(effect->twp) = pnum;
	}

	pwp->free.sw[2] = -1; // if -1 or 0 the homing attack search in SonicHomingOnTarget will run
	pwp->free.sw[3] = 0; // additional timer
	dsPlay_oneshot(762, 0, 0, 0);

	if (pnum >= PLAYER_MAX)
		return;

	auto ael = GetTargetEnemyList(pnum);

	if ((ael && ael->twp) || GetRivalPlayerNumber_r(pnum) != -1)
	{
		SonicHomingOnTarget_r(twp, pwp, mwp);

		if (pwp->free.sw[2])
		{
			pwp->spd.x = 5.0f;
		}
	}
	else
	{
		pwp->spd.x = (pwp->equipment & Upgrades_SuperSonic) ? 10.0f : 5.0f;
		pwp->free.sw[2] = 1;
	}
}

Bool MilesCheckFinishThrowObject_r(taskwk* twp, playerwk* pwp)
{
	if (!multiplayer::IsActive())
	{
		return MilesCheckFinishThrowObject_h.Original(twp, pwp);
	}

	auto htp = pwp->htp;
	if (!htp || !htp->twp || !htp->mwp)
	{
		if (twp->flag & Status_HoldObject)
		{
			pwp->htp = nullptr;
			twp->flag &= ~Status_HoldObject;
			return 1;
		}
		return 0;
	}

	auto pnum = TASKWK_PLAYERID(twp);
	auto ael = GetTargetEnemyList(pnum);
	auto reqaction = pwp->mj.reqaction;

	if ((reqaction != 84 || pwp->mj.nframe != 20.0f)
		&& (reqaction != 86 || pwp->mj.nframe != 7.0f)
		|| !ael->twp)
	{
		if (reqaction == 85 && pwp->mj.nframe == 24.0f
			|| reqaction == 87 && pwp->mj.nframe == 6.0f)
		{
			htp->twp->flag &= ~0x1000;
			NJS_POINT3 vec = { 2.0f, 0.8f, 0.0f };
			PConvertVector_P2G(twp, &vec);
			pwp->htp->mwp->spd = vec;
			twp->flag &= ~Status_HoldObject;
			pwp->htp = 0;
			miles_oneshot(twp, 1241);
			return 1;
		}

		if (reqaction == 88 && pwp->mj.nframe == 12.0f)
		{
			htp->twp->flag &= ~0x1000;
			PPutHeldObject(twp, htp->twp, pwp);
			twp->flag &= ~Status_HoldObject;
			pwp->htp = 0;
			return 1;
		}

		return 0;
	}

	// Calculate thrown object velocity

	htp->twp->flag &= ~0x1000;

	auto closest_tgt = ael->twp;
	auto closest_dist = ael->dist;

	do
	{
		if (closest_dist > ael->dist)
		{
			closest_tgt = ael->twp;
			closest_dist = ael->dist;
		}
		++ael;
	} while (ael->twp);

	auto info = closest_tgt->cwp->info;

	NJS_VECTOR vec;
	vec.x = info->center.x + closest_tgt->pos.x - twp->pos.x;
	vec.y = info->center.y + closest_tgt->pos.y - twp->pos.y;
	vec.z = info->center.z + closest_tgt->pos.z - twp->pos.z;

	vec.x = njSqrt(vec.z * vec.z + vec.x * vec.x);
	vec.z = 0.0f;

	GetUnitVector(&vec);

	vec.x = vec.x * 3.0f;
	vec.y = vec.y * 3.0f;
	PConvertVector_P2G(twp, &vec);

	pwp->htp->mwp->spd = vec;
	twp->flag &= ~Status_HoldObject;
	pwp->htp = nullptr;
	miles_oneshot(twp, 1241);
	return 1;
}

Bool KnucklesCheckFinishThrowObject_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (!multiplayer::IsActive())
	{
		return KnucklesCheckFinishThrowObject_h.Original(twp, mwp, pwp);
	}

	auto htp = pwp->htp;
	if (!htp || !htp->twp || !htp->mwp)
	{
		if (twp->flag & Status_HoldObject)
		{
			pwp->htp = nullptr;
			twp->flag &= ~Status_HoldObject;
			return 1;
		}
		return 0;
	}

	auto pnum = TASKWK_PLAYERID(twp);
	auto ael = GetTargetEnemyList(pnum);
	auto reqaction = pwp->mj.reqaction;

	if ((reqaction != 96 || pwp->mj.nframe != 20.0f)
		&& (reqaction != 98 || pwp->mj.nframe != 7.0f)
		|| !ael->twp)
	{
		if (reqaction == 97 && pwp->mj.nframe == 24.0f
			|| reqaction == 99 && pwp->mj.nframe == 6.0f)
		{
			htp->twp->flag &= ~0x1000;
			NJS_POINT3 vec = { 2.0f, 0.8f, 0.0f };
			PConvertVector_P2G(twp, &vec);
			pwp->htp->mwp->spd = vec;
			twp->flag &= ~Status_HoldObject;
			pwp->htp = 0;
			dsPlay_oneshot(1258, 0, 0, 0);
			return 1;
		}

		if (reqaction == 100 && pwp->mj.nframe == 12.0f)
		{
			htp->twp->flag &= ~0x1000;
			PPutHeldObject(twp, htp->twp, pwp);
			twp->flag &= ~Status_HoldObject;
			pwp->htp = 0;
			return 1;
		}

		return 0;
	}

	// Calculate thrown object velocity

	htp->twp->flag &= ~0x1000;

	auto closest_tgt = ael->twp;
	auto closest_dist = ael->dist;

	do
	{
		if (closest_dist > ael->dist)
		{
			closest_tgt = ael->twp;
			closest_dist = ael->dist;
		}
		++ael;
	} while (ael->twp);

	auto info = closest_tgt->cwp->info;

	NJS_VECTOR vec;
	vec.x = info->center.x + closest_tgt->pos.x - twp->pos.x;
	vec.y = info->center.y + closest_tgt->pos.y - twp->pos.y;
	vec.z = info->center.z + closest_tgt->pos.z - twp->pos.z;

	vec.x = njSqrt(vec.z * vec.z + vec.x * vec.x);
	vec.z = 0.0f;

	GetUnitVector(&vec);

	vec.x = vec.x * 3.0f;
	vec.y = vec.y * 3.0f;
	PConvertVector_P2G(twp, &vec);

	pwp->htp->mwp->spd = vec;
	twp->flag &= ~Status_HoldObject;
	pwp->htp = nullptr;
	dsPlay_oneshot(1258, 0, 0, 0);
	return 1;
}

void InitCollisionPatches()
{
	// Dyncol lookup rewrite
	MakeLandCollLandEntryRangeIn_h.Hook(MakeLandCollLandEntryRangeIn_r);
	WriteData((_OBJ_LANDENTRY**)0x43AE3D, ri_landentry_buf_ex);
	WriteData((_OBJ_LANDENTRY**)0x43B123, ri_landentry_buf_ex);
	WriteData((_OBJ_LANDENTRY**)0x43B62E, ri_landentry_buf_ex);
	WriteData((size_t*)0x43B2AD, LengthOfArray(ri_landentry_buf_ex));
	WriteData((size_t*)0x43B688, LengthOfArray(ri_landentry_buf_ex));

	// Simple bound checking
	WriteJump(CheckCollisionP, CheckCollisionP_r);
	WriteJump(CheckCollisionCylinderP, CheckCollisionCylinderP_r);

	// Fix mistake with CCL_IsHitPlayer
	CCL_IsHitPlayer_h.Hook(CCL_IsHitPlayer_r);

	// Enemy list expansion
	CCL_CheckHoming_h.Hook(CCL_CheckHoming_r);
	CCL_ClearAll_h.Hook(CCL_ClearAll_r);
	CCL_Analyze_h.Hook(CCL_Analyze_r);
	LockingOnTargetEnemy_h.Hook(LockingOnTargetEnemy_r);
	pLockingOnTargetEnemy2_h.Hook(pLockingOnTargetEnemy2_r);
	BigCheckTargetEnemy_h.Hook(BigCheckTargetEnemy_r);
	GetRivalPlayerNumber_h.Hook(GetRivalPlayerNumber_r);
	SonicHomingOnTarget_h.Hook(SonicHomingOnTarget_r);
	SonicHomeOnTarget_h.Hook(SonicHomeOnTarget_r);
	MilesCheckFinishThrowObject_h.Hook(MilesCheckFinishThrowObject_r);
	KnucklesCheckFinishThrowObject_h.Hook(KnucklesCheckFinishThrowObject_r);
	WriteJump((void*)0x43C110, PCheckTargetEnemy_r);
}
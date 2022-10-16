#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"

static Trampoline* CCL_IsHitPlayer_t = nullptr;
static Trampoline* MakeLandCollLandEntryRangeIn_t = nullptr;

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
	if (multiplayer::IsActive() && multiplayer::GetPlayerCount() > 2)
	{
		MakeLandCollLandEntryRangeInM();
	}
	else
	{
		TARGET_DYNAMIC(MakeLandCollLandEntryRangeIn)();
	}
}

void __cdecl CCL_IsHitPlayer_r(taskwk* twp)
{
	CCL_ClearSearch(); // <- needed when called by the same task twice, which actually happens vanilla
	TARGET_DYNAMIC(CCL_IsHitPlayer)(twp);
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
//}

void InitCollisionPatches()
{
	CCL_IsHitPlayer_t = new Trampoline(0x41CBC0, 0x41CBC5, CCL_IsHitPlayer_r);
	MakeLandCollLandEntryRangeIn_t = new Trampoline(0x43AEF0, 0x43AEF5, MakeLandCollLandEntryRangeIn_r);
	WriteJump(CheckCollisionP, CheckCollisionP_r);
	WriteJump(CheckCollisionCylinderP, CheckCollisionCylinderP_r);
}

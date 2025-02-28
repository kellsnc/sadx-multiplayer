#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "camera.h"
#include "camerafunc.h"

FastFunctionHook<void, task*> PathworkCamera_h(0x4BBB90);
FastFunctionHook<void, task*> PathworkSeeingPath_h(0x4BB1F0);
FastFunctionHook<void, task*> PathworkSeeingPath_Sky_h(0x5F16C0);
FastFunctionHook<void, task*> PathworkLaddering_h(0x4BB830);
FastFunctionHook<void, task*> PathworkGoWithHangingDownFromBars_h(0x4BB520);

// Fix PathworkCamera, a path task only used for Red Mountain

PATHCAMERA1WORK pathcamera1playersworks[PLAYER_MAX] = {};

/*
 * Guessed inlined function
 * Gets a box boundary that contains the entire path
 */
static void CalcPathBounds(pathtag* path, NJS_POINT3* pMin, NJS_POINT3* pMax, Float margin)
{
	pMin->x = pMax->x = path->tblhead[0].xpos;
	pMin->y = pMax->y = path->tblhead[0].ypos;
	pMin->z = pMax->z = path->tblhead[0].zpos;

	for (int i = 0; i < path->points; ++i)
	{
		auto p = &path->tblhead[i];

		if (p->xpos > pMax->x)
			pMax->x = p->xpos;
		if (p->ypos > pMax->y)
			pMax->y = p->ypos;
		if (p->zpos > pMax->z)
			pMax->z = p->zpos;

		if (p->xpos < pMin->x)
			pMin->x = p->xpos;
		if (p->ypos < pMin->y)
			pMin->y = p->ypos;
		if (p->zpos < pMin->z)
			pMin->z = p->zpos;
	}

	pMax->x += margin;
	pMax->y += margin;
	pMax->z += margin;
	pMin->x -= margin;
	pMin->y -= margin;
	pMin->z -= margin;
}

/*
 * Guessed inlined function
 * Check if position is in box boundary
 */
static bool CheckPathBounds(NJS_POINT3* pPos, NJS_POINT3* pMin, NJS_POINT3* pMax, Float radius)
{
	return (pMax->x + radius) >= pPos->x && (pMax->y + radius) >= pPos->y && (pMax->z + radius) >= pPos->z
		&& (pMin->x - radius) <= pPos->x && (pMin->y - radius) <= pPos->y && (pMin->z - radius) <= pPos->z;
}

static void PathworkCamera_m(task* tp)
{
	auto twp = tp->twp;
	auto path = reinterpret_cast<pathtag*>(twp->value.ptr);
	auto pathwk = pathcamera1works[twp->smode];

	if (GetStageNumber() != twp->timer.w[0])
	{
		FreeTask(tp);
		return;
	}

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];
		int plmask = (1 << i);

		if (!ptwp)
		{
			continue;
		}

		// If the player is not on the path
		if ((twp->btimer & plmask) == 0)
		{
			if (IsCompulsionCamera_m(i) || IsEventCamera_m(i))
			{
				continue;
			}

			// Inlined function that checks if in bounding box
			if (!CheckPathBounds(&ptwp->pos, &twp->pos, &twp->scl, 50.0f))
			{
				continue;
			}

			Float dist = 0.0f;

			for (int pt = 0; pt < path->points - pathwk->nBottomPathGap; ++pt)
			{
				NJS_POINT3 v;
				v.x = path->tblhead[pt].xpos;
				v.y = path->tblhead[pt].ypos;
				v.z = path->tblhead[pt].zpos;

				njSubVector(&v, &ptwp->pos);

				Float current_dist = njScalor(&v);
				if (current_dist < dist || pt == pathwk->nTopPathGap)
				{
					dist = current_dist;
				}
			}

			if (dist <= 50.0f)
			{
				twp->btimer |= plmask;

				auto param = GetCamAnyParam(i);

				if (param)
				{
					pathcamera1playersworks[i] = *pathwk;
					pathcamera1playersworks[i].pPathTag = path;

					if (i == 0)
					{
						camera_twp->counter.ptr = &pathcamera1playersworks[i];
					}

					param->camAnyTmpSint32[0] = reinterpret_cast<int>(&pathcamera1playersworks[i]);
					CameraSetEventCameraFunc_m(i, PathCamera1, CAMADJ_NONE, CDM_NONE);
				}
			}
		}
		// If the player is on the path
		else
		{
			if (IsCompulsionCamera_m(i) || !IsEventCamera_m(i))
			{
				continue; // break;
			}

			if (twp->scl.x + 90.0f < ptwp->pos.x
				|| twp->scl.y + 90.0f < ptwp->pos.y
				|| twp->scl.z + 90.0f < ptwp->pos.z
				|| twp->pos.x - 90.0f > ptwp->pos.x
				|| twp->pos.y - 90.0f > ptwp->pos.y
				|| twp->pos.z - 90.0f > ptwp->pos.z)
			{
				twp->btimer &= ~plmask;
				CameraReleaseEventCamera_m(i);
			}

			Float dist = 0.0f;

			for (int pt = 0; pt < path->points - pathwk->nBottomPathGap; ++pt)
			{
				NJS_POINT3 v;
				v.x = path->tblhead[pt].xpos;
				v.y = path->tblhead[pt].ypos;
				v.z = path->tblhead[pt].zpos;

				njSubVector(&v, &ptwp->pos);

				Float current_dist = njScalor(&v);
				if (current_dist < dist || pt == pathwk->nTopPathGap)
				{
					dist = current_dist;
				}
			}

			if (dist > 90.0f)
			{
				twp->btimer &= ~plmask;
				CameraReleaseEventCamera_m(i);
			}
		}
	}
}

static void __cdecl PathworkCamera_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		PathworkCamera_m(tp);
	}
	else
	{
		PathworkCamera_h.Original(tp);
	}
}

static void __cdecl PathworkSeeingPath_m(task* tp)
{
	auto twp = tp->twp;
	auto path = (pathtag*)twp->value.ptr;

	// Originally a switch but who cares
	if (twp->mode == 1)
	{
		if (GetStageNumber() != twp->timer.w[0])
		{
			FreeTask(tp);
			return;
		}

		for (int pnum = 0; pnum < PLAYER_MAX; ++pnum)
		{
			auto ptwp = playertwp[pnum];

			if (!ptwp)
				continue;

			auto mask = (1 << pnum);

			if (twp->btimer & mask)
			{
				if (!(ptwp->flag & Status_OnPath))
				{
					twp->counter.b[pnum] = 0;
					twp->btimer &= ~mask;
				}
			}
			else
			{
				if (++twp->counter.b[pnum] < 60)
				{
					continue;
				}

				twp->counter.b[pnum] = 60;

				if (CheckPathBounds(&ptwp->pos, &twp->pos, &twp->scl, 0.0f))
				{
					for (int i = 0; i < path->points - 1; ++i)
					{
						auto pt = &path->tblhead[i];
						NJS_POINT3 pos = { pt->xpos, pt->ypos, pt->zpos };
						njSubVector(&pos, &ptwp->pos);
						if (njScalor(&pos) < 40.0f)
						{
							RunWithSeeingPathP(pnum, path);
							twp->btimer |= mask;
							break;
						}
					}
				}
			}
		}

	}
	else if (twp->mode == 0)
	{
		twp->btimer = 0;
		twp->counter.b[0] = 60;
		twp->counter.b[1] = 60;
		twp->counter.b[2] = 60;
		twp->counter.b[3] = 60;
		CalcPathBounds(path, &twp->pos, &twp->scl, 40.0f);
		twp->mode = 1;
	}
}

static void __cdecl PathworkSeeingPath_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		PathworkSeeingPath_m(tp);
	}
	else
	{
		PathworkSeeingPath_h.Original(tp);
	}
}

static void __cdecl PathworkSeeingPath_Sky_m(task* tp)
{
	auto twp = tp->twp;
	auto path = (pathtag*)twp->value.ptr;

	// Originally a switch but who cares
	if (twp->mode == 1)
	{
		if (GetStageNumber() != twp->timer.w[0])
		{
			FreeTask(tp);
			return;
		}

		for (int pnum = 0; pnum < PLAYER_MAX; ++pnum)
		{
			auto ptwp = playertwp[pnum];

			if (!ptwp)
				continue;

			auto mask = (1 << pnum);

			if (twp->btimer & mask)
			{
				if (!(ptwp->flag & Status_OnPath))
				{
					twp->counter.b[pnum] = 0;
					twp->btimer &= ~mask;
				}
			}
			else
			{
				if (++twp->counter.b[pnum] < 60)
				{
					continue;
				}

				twp->counter.b[pnum] = 60;

				if (CheckPathBounds(&ptwp->pos, &twp->pos, &twp->scl, 0.0f))
				{
					for (int i = 0; i < path->points - 1; ++i)
					{
						auto pt = &path->tblhead[i];
						NJS_POINT3 pos = { pt->xpos, pt->ypos, pt->zpos };
						njSubVector(&pos, &ptwp->pos);
						if (njScalor(&pos) < 10.0f)
						{
							RunWithSeeingPathP(pnum, path);
							twp->btimer |= mask;
							break;
						}
					}
				}
			}
		}

	}
	else if (twp->mode == 0)
	{
		twp->btimer = 0;
		twp->counter.b[0] = 60;
		twp->counter.b[1] = 60;
		twp->counter.b[2] = 60;
		twp->counter.b[3] = 60;
		CalcPathBounds(path, &twp->pos, &twp->scl, 10.0f);
		twp->mode = 1;
	}
}

static void __cdecl PathworkSeeingPath_Sky_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		PathworkSeeingPath_Sky_m(tp);
	}
	else
	{
		PathworkSeeingPath_Sky_h.Original(tp);
	}
}

static void __cdecl PathworkLaddering_m(task* tp)
{
	auto twp = tp->twp;
	auto path = (pathtag*)twp->value.ptr;

	// Originally a switch but who cares
	if (twp->mode == 1)
	{
		if (GetStageNumber() != twp->timer.w[0])
		{
			FreeTask(tp);
			return;
		}

		for (int pnum = 0; pnum < PLAYER_MAX; ++pnum)
		{
			auto ptwp = playertwp[pnum];

			if (!ptwp)
				continue;

			auto mask = (1 << pnum);

			if (twp->btimer & mask)
			{
				if (!(ptwp->flag & Status_OnPath))
				{
					twp->counter.b[pnum] = 0;
					twp->btimer &= ~mask;
				}
			}
			else
			{
				if (++twp->counter.b[pnum] < 60)
				{
					continue;
				}

				twp->counter.b[pnum] = 60;

				if (CheckPathBounds(&ptwp->pos, &twp->pos, &twp->scl, 0.0f))
				{
					for (int i = 0; i < path->points; ++i)
					{
						auto pt = &path->tblhead[i];
						NJS_POINT3 pos = { pt->xpos, pt->ypos, pt->zpos };
						njSubVector(&pos, &ptwp->pos);
						if (njScalor(&pos) < (TASKWK_CHARID(ptwp) == Characters_Big ? 10.0f : 5.0f))
						{
							LadderingPathP(pnum, path, i, &twp->ang);
							twp->btimer |= mask;
							break;
						}
					}
				}
			}
		}

	}
	else if (twp->mode == 0)
	{
		twp->btimer = 0;
		twp->counter.b[0] = 60;
		twp->counter.b[1] = 60;
		twp->counter.b[2] = 60;
		twp->counter.b[3] = 60;
		CalcPathBounds(path, &twp->pos, &twp->scl, 10.0f /*5.0f*/);

		//if (GetPlayerNumber() == Characters_Big)
		//{
		//	twp->scl.x += 5.0f;
		//	twp->scl.z += 5.0f;
		//	twp->pos.x -= 5.0f;
		//	twp->pos.x += 5.0f;
		//}

		twp->mode = 1;
	}
}

static void __cdecl PathworkLaddering_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		PathworkLaddering_m(tp);
	}
	else
	{
		PathworkLaddering_h.Original(tp);
	}
}

static void __cdecl PathworkGoWithHangingDownFromBars_m(task* tp)
{
	auto twp = tp->twp;
	auto path = (pathtag*)twp->value.ptr;

	// Originally a switch but who cares
	if (twp->mode == 1)
	{
		if (GetStageNumber() != twp->timer.w[0])
		{
			FreeTask(tp);
			return;
		}

		for (int pnum = 0; pnum < PLAYER_MAX; ++pnum)
		{
			auto ptwp = playertwp[pnum];

			if (!ptwp)
				continue;

			auto mask = (1 << pnum);

			if (twp->btimer & mask)
			{
				if (!(ptwp->flag & Status_OnPath))
				{
					twp->counter.b[pnum] = 0;
					twp->btimer &= ~mask;
				}
			}
			else
			{
				if (++twp->counter.b[pnum] < 30)
				{
					continue;
				}

				twp->counter.b[pnum] = 30;

				if (CheckPathBounds(&ptwp->pos, &twp->pos, &twp->scl, 0.0f))
				{
					for (int i = 0; i < path->points; ++i)
					{
						auto pt = &path->tblhead[i];
						NJS_POINT3 pos = { pt->xpos, pt->ypos, pt->zpos };
						njSubVector(&pos, &ptwp->pos);
						if (njScalor(&pos) < 10.0f)
						{
							HangDownFromPathP(pnum, path, i);
							twp->btimer |= mask;
							break;
						}
					}
				}
			}
		}

	}
	else if (twp->mode == 0)
	{
		twp->btimer = 0;
		twp->counter.b[0] = 30;
		twp->counter.b[1] = 30;
		twp->counter.b[2] = 30;
		twp->counter.b[3] = 30;
		CalcPathBounds(path, &twp->pos, &twp->scl, 10.0f);
		twp->mode = 1;
	}
}

static void __cdecl PathworkGoWithHangingDownFromBars_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		PathworkGoWithHangingDownFromBars_m(tp);
	}
	else
	{
		PathworkGoWithHangingDownFromBars_h.Original(tp);
	}
}

void patch_pathtask_init()
{
	PathworkCamera_h.Hook(PathworkCamera_r);
	PathworkSeeingPath_h.Hook(PathworkSeeingPath_r);
	PathworkSeeingPath_Sky_h.Hook(PathworkSeeingPath_Sky_r);
	PathworkLaddering_h.Hook(PathworkLaddering_r);
	PathworkGoWithHangingDownFromBars_h.Hook(PathworkGoWithHangingDownFromBars_r);
}

RegisterPatch patch_pathtask(patch_pathtask_init);
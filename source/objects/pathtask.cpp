#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"
#include "camera.h"
#include "camerafunc.h"

// Fix PathworkCamera, a path task only used for Red Mountain

PATHCAMERA1WORK pathcamera1playersworks[PLAYER_MAX] = {};

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
			if (twp->scl.x + 50.0f < ptwp->pos.x
				|| twp->scl.y + 50.0f < ptwp->pos.y
				|| twp->scl.z + 50.0f < ptwp->pos.z
				|| twp->pos.x - 50.0f > ptwp->pos.x
				|| twp->pos.y - 50.0f > ptwp->pos.y
				|| twp->pos.z - 50.0f > ptwp->pos.z)
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

static void __cdecl PathworkCamera_r(task* tp);
Trampoline PathworkCamera_t(0x4BBB90, 0x4BBB97, PathworkCamera_r);
static void __cdecl PathworkCamera_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		PathworkCamera_m(tp);
	}
	else
	{
		TARGET_STATIC(PathworkCamera)(tp);
	}
}


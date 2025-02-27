#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "camera.h"

FastFunctionHook<void, task*> ObjectBeachYurehasiExec_h(0x501BC0);

static void ObjectBeachYurehasiExec_m(task* tp)
{
	auto twp = tp->twp;

	// Allow collision/rendering of children
	auto ctp = tp->ctp;
	while (ctp)
	{
		auto ctwp = ctp->twp;

		ctwp->flag &= ~0x100;
		ctwp->smode = 0;

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto ptwp = playertwp[i];

			if (ptwp && GetDistance(&ctwp->pos, &ptwp->pos) < sqrtf(*(float*)0x7EC9F8))
			{
				ctwp->flag |= 0x100;
			}

			auto cam_pos = GetCameraPosition(i);

			if (cam_pos && GetDistance(&ctwp->pos, cam_pos) < sqrtf(*(float*)0x7EC9F4))
			{
				ctwp->smode = 1;
			}
		}

		ctp = ctp->next;
	}

	// Update bride parts position
	Float posx = twp->pos.x + 75.0f;
	Float posy = twp->pos.y + 9.5f;
	Float posz = twp->pos.z;

	Float waveheight = orGetWaveHeight(posx, posz);
	Float newheight;
	hasi_a_posy[0] = waveheight;

	Bool passed = 1;
	for (int i = 0; i < 25; ++i)
	{
		posx += 40.0f;
		newheight = orGetWaveHeight(posx, posz);
		if (newheight != -1.5f)
		{
			passed = 0;
		}
		hasi_b_posy[i] = (waveheight + newheight) * 0.5f + posy;
		hasi_b_angz[i] = njArcTan2(newheight - waveheight, 40.0f);
		waveheight = newheight;
	}

	hasi_a_posy[1] = newheight;

	// Sound stuff
	if (twp->timer.l == 1 && passed == 1)
	{
		dsStop_id((int)twp);
		twp->timer.l = 0;
	}
	else if (twp->timer.l == 0 && passed == 0)
	{
		dsPlay_iloop(41, (int)twp, 1, 0x7D0);
		twp->timer.l = 1;
	}

	// Run the bridge parts
	LoopTaskC(tp);
}

static void __cdecl ObjectBeachYurehasiExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectBeachYurehasiExec_m(tp);
	}
	else
	{
		ObjectBeachYurehasiExec_h.Original(tp);
	}
}

void patch_beach_yurehasi_init()
{
	ObjectBeachYurehasiExec_h.Hook(ObjectBeachYurehasiExec_r);
}

RegisterPatch patch_beach_yurehasi(patch_beach_yurehasi_init);
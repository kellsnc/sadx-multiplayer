#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "utils.h"

static void __cdecl rotateobj_Draw_r(task* tp);
FastFunctionHookPtr<decltype(&rotateobj_Draw_r)> rotateobj_Draw_h(0x4E1010);

static void __cdecl rotateobj_Draw_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		if (GetDistance2(&tp->twp->pos, &camera_twp->pos) < *(float*)0x4E136A && dsCheckViewV(&tp->twp->pos, *(float*)0x4E138A))
		{
			rotateobj_Draw_h.Original(tp);
		}
	}
	else
	{
		rotateobj_Draw_h.Original(tp);
	}
}

void patch_windy_rotateobj_init()
{
	rotateobj_Draw_h.Hook(rotateobj_Draw_r);
}

RegisterPatch patch_windy_rotateobj(patch_windy_rotateobj_init);
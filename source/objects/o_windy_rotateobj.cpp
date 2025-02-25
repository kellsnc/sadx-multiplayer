#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "utils.h"

static void __cdecl rotateobj_Draw_r(task* tp);
FastFunctionHookPtr<decltype(&rotateobj_Draw_r)> rotateobj_Draw_t(0x4E1010, rotateobj_Draw_r);
static void __cdecl rotateobj_Draw_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		if (GetDistance2(&tp->twp->pos, &camera_twp->pos) < *(float*)0x4E136A && dsCheckViewV(&tp->twp->pos, *(float*)0x4E138A))
		{
			rotateobj_Draw_t.Original(tp);
		}
	}
	else
	{
		rotateobj_Draw_t.Original(tp);
	}
}
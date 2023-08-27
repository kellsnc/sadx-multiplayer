#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "utils.h"

static void __cdecl rotateobj_Draw_r(task* tp);
Trampoline rotateobj_Draw_t(0x4E1010, 0x4E1015, rotateobj_Draw_r);
static void __cdecl rotateobj_Draw_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		if (GetDistance2(&tp->twp->pos, &camera_twp->pos) < *(float*)0x4E136A && dsCheckViewV(&tp->twp->pos, *(float*)0x4E138A))
		{
			TARGET_STATIC(rotateobj_Draw)(tp);
		}
	}
	else
	{
		TARGET_STATIC(rotateobj_Draw)(tp);
	}
}
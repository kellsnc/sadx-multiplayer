#include "pch.h"
#include "SADXModLoader.h"
#include "FunctionHook.h"
#include "sadx_utils.h"

TaskFunc(ObjectTuribasiDisplayer, 0x7A6240);

void __cdecl ObjectWindyTuribasiDisplayerE102(task* tp)
{
	ObjectTuribasiDisplayer(tp);
	auto twp = tp->twp;
	if (twp->smode == 0)
	{
		NJS_VECTOR pos = { 100.0f, -30.0f, 0.0f };
		njPushMatrix(nj_unit_matrix_);
		njTranslate(0, twp->pos.x, twp->pos.y, twp->pos.z);
		ROTATEY(0, twp->ang.y);
		njCalcPoint(0, &pos, &pos);
		njPopMatrix(1u);
		Angle3 ang;
		pos.y = GetShadowPos(pos.x, pos.y + 10.0f, pos.z, &ang);
		njSetTexture(&E_SAI_TEXLIST);
		njPushMatrix(0);
		njTranslate(0, pos.x, pos.y, pos.z);
		ROTATEZ(0, ang.z);
		ROTATEX(0, ang.x);
		ROTATEY(0, twp->ang.y + 0x8000);
		ds_DrawObjectClip((NJS_OBJECT*)0x38CBC74, 1.0f);
		njPopMatrix(1);
	}
}

void __cdecl ObjectWindyTuriBrokenE102(task* tp);
TaskHook ObjectWindyTuriBrokenE102_h(0x4E0FC0, ObjectWindyTuriBrokenE102);
void __cdecl ObjectWindyTuriBrokenE102(task* tp)
{
	ObjectWindyTuriBrokenE102_h.Original(tp);
	tp->disp = ObjectWindyTuribasiDisplayerE102;
}

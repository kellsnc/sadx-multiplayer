#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "utils.h"
#include "camera.h"

enum : __int8
{
	MODE_INITIAL,
	MODE_NORMAL,
	MODE_END
};

static void __cdecl torin_r(task* tp);
FastFunctionHookPtr<decltype(&torin_r)> torin_t(0x4DF5A0);

static void torin_m(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == MODE_INITIAL)
	{
		twp->mode = 1i8;

		twp->pos.x = 0.0f;
		twp->pos.z = 0.0f;

		Float random = njRandom();
		twp->scl.y = njAbs(random);
		twp->counter.f = (random * 360.0f);

		tp->disp = (TaskFuncPtr)0x4DF500;
	}
	else if (twp->mode == MODE_NORMAL)
	{
		twp->counter.f += twp->scl.y * 1.5f + 8.0f;
		Angle ang = NJM_DEG_ANG(-twp->counter.f);
		twp->pos.x = njCos(ang) * 100.0f;
		twp->pos.z = njSin(ang) * 100.0f;

		twp->pos.y = fabs(njRandom()) * 3.4f + twp->scl.y * 1.3f + twp->pos.y;

		ang = -NJM_DEG_ANG(twp->scl.y + twp->scl.y);
		twp->ang.x -= ang;
		twp->ang.z -= ang;

		twp->ang.y += NJM_DEG_ANG(twp->scl.y * 5.0f);
	}

	tp->disp(tp);
}

static void __cdecl torin_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		torin_m(tp);
	}
	else
	{
		torin_t.Original(tp);
	}
}

void patch_windy_torin_init()
{
	torin_t.Hook(torin_r);
}

RegisterPatch patch_windy_torin(patch_windy_torin_init);
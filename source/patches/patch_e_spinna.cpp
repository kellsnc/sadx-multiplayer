#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "splitscreen.h"

FastFunctionHook<void, task*> SpinnaDisplayer_h(0x4AFD80);

void SpinnaDrawShield(taskwk* twp)
{
	for (int i = 0; i < 16; ++i)
	{
		Angle mod1 = NJM_DEG_ANG(180.0 * (0.5 - njRandom()));
		Angle mod2 = NJM_DEG_ANG(360.0 * njRandom());
		float mod3 = static_cast<float>((njRandom() + 0.5) * 10.0);

		NJS_VECTOR posm;
		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		NJS_VECTOR pos1 = twp->pos;
		NJS_VECTOR pos2 = twp->pos;

		njAddVector(&pos1, &posm);

		pos1.y += 5.0f;
		pos2.y += 5.0f;
		DrawLineV(&pos2, &pos1);

		pos2 = pos1;
		mod1 += NJM_DEG_ANG(120.0 * (0.5 - njRandom()));
		mod2 += NJM_DEG_ANG(80.0 * (0.5 - njRandom()));
		mod3 = static_cast<float>((njRandom() + 1.0) * 4.0);

		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		njAddVector(&pos1, &posm);
		DrawLineV(&pos2, &pos1);

		pos2 = pos1;
		mod1 += NJM_DEG_ANG(120.0 * (0.5 - njRandom()));
		mod2 += NJM_DEG_ANG(80.0 * (0.5 - njRandom()));
		mod3 = static_cast<float>((njRandom() + 1.0) * 2.0);

		posm.x = njCos(mod1) * njCos(mod2) * mod3;
		posm.y = njSin(mod1) * mod3;
		posm.z = njCos(mod1) * njSin(mod2) * mod3;

		njAddVector(&pos1, &posm);
		DrawLineV(&pos2, &pos1);
	}
}

void __cdecl SpinnaDisplayer_r(task* tp)
{
	if (splitscreen::IsActive())
	{
		if (!MissedFrames)
		{
			auto twp = tp->twp;

			SpinnaDraw(twp, (enemywk*)tp->mwp);

			if (splitscreen::GetCurrentScreenNum() > 0 && twp->smode == 0 && twp->wtimer != 0)
			{
				SpinnaDrawShield(twp);
			}
		}
	}
	else
	{
		SpinnaDisplayer_h.Original(tp);
	}
}

void patch_spinna_init()
{
	SpinnaDisplayer_h.Hook(SpinnaDisplayer_r);
}

RegisterPatch patch_spinna(patch_spinna_init);
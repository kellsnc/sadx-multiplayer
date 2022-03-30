#include "pch.h"
#include "multiplayer.h"
#include "fishing.h"

// Floating platforms in Hot Shelter

enum : __int8
{
	MODE_WAIT,
	MODE_RIDED,
	MODE_FREEZE
};

static void ObjShelterUkijimaExec_m(task* tp)
{
	auto twp = tp->twp;

	if (twp->smode == 0 && CheckRangeOut(tp))
	{
		return;
	}

	auto mwp = tp->mwp;

	switch (twp->mode)
	{
	case MODE_WAIT:
		if (IsPlayerOnDyncol(tp))
		{
			twp->mode = MODE_RIDED;

			if (twp->timer.f >= -1.0f)
			{
				twp->timer.f -= 0.5f;
			}

			NJS_POINT3 v;
			if (GetPlayerPosition(GetClosestPlayerNum(&twp->pos), 0, &v, 0) != FALSE)
			{
				double x = (double)(v.x - twp->pos.x);
				double z = (double)(v.z - twp->pos.z);
				mwp->spd.x = (z == 0.0f ? 0.0f : (float)NJM_RAD_ANG(atan2(z, 140.0))) * 0.25f;
				mwp->spd.z = (x == 0.0f ? 0.0f : (float)NJM_RAD_ANG(atan2(-x, 140.0))) * 0.25f;
			}
		}
		break;
	case MODE_RIDED:
		if (!IsPlayerOnDyncol(tp))
		{
			twp->mode = MODE_WAIT;
		}
		break;
	case MODE_FREEZE:
		if (mwp->rad != twp->value.f)
		{
			twp->mode = MODE_WAIT;
		}
		break;
	}

	float gap = twp->value.f - twp->pos.y;
	if (fabs(twp->timer.f) >= 0.001f || fabs(gap) >= 0.001f)
	{
		twp->pos.y += twp->timer.f;
		twp->timer.f = (twp->timer.f + gap * 0.005f) * 0.94f;
	}
	else
	{
		twp->pos.y = twp->value.f;
		twp->timer.f = 0.0f;
	}

	auto dyncol = (NJS_OBJECT*)twp->counter.ptr;
	dyncol->pos[1] = twp->pos.y;

	if (fabs(mwp->spd.x) >= 0.25f)
	{
		float angxf = (float)twp->ang.x;
		mwp->spd.x = (mwp->spd.x + -angxf * 0.01f) * 0.95f;
		Angle new_angx = (Angle)(angxf + mwp->spd.x);
		twp->ang.x = new_angx;
		dyncol->ang[0] = new_angx;
	}
	else
	{
		twp->ang.x = 0;
		mwp->spd.x = 0.0f;
		dyncol->ang[0] = 0;
	}

	if (fabs(mwp->spd.z) >= 0.25f)
	{
		float angzf = (float)twp->ang.z;
		mwp->spd.z = (mwp->spd.z + -angzf * 0.01f) * 0.95f;
		Angle new_angz = (Angle)(angzf + mwp->spd.z);
		twp->ang.z = new_angz;
		dyncol->ang[2] = new_angz;
	}
	else
	{
		mwp->spd.z = 0.0f;
		twp->ang.z = 0;
		dyncol->ang[2] = 0;
	}

	CheckDyncolRange(twp, &twp->pos, 25.0f);
	tp->disp(tp);
}

static void __cdecl ObjShelterUkijimaExec_r(task* tp); //"Exec"
Trampoline ObjShelterUkijimaExec_t(0x59DC20, 0x59DC25, ObjShelterUkijimaExec_r);
static void __cdecl ObjShelterUkijimaExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjShelterUkijimaExec_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjShelterUkijimaExec)(tp);
	}
}
#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"

// Objects in Gamma's Hot Shelter that teleports player to cargo area

enum MD_CARGO // made up
{
	MDCARGO_CHECKBOUNDS,
	MDCARGO_CHECKGROUND,
	MDCARGO_WAIT,
	MDCARGO_STARTFADE,
	MDCARGO_TELEPORT,
	MDCARGO_OUT
};

static void CreateObjShelterFade(char pnum)
{
	auto tp = CreateElementalTask(2u, 3, ObjShelterFade);
	tp->twp->btimer = pnum;
}

static void Move(task* tp, float x)
{
	auto twp = tp->twp;
	auto fwp = tp->fwp;

	twp->pos.x += x;

	auto object = reinterpret_cast<NJS_OBJECT*>(twp->counter.ptr);
	object->pos[0] = twp->pos.x;
	object->pos[1] = twp->pos.y;

	if (twp->btimer == 0)
	{
		fwp->pos_spd.x = x;
	}
	else
	{
		if (CheckPlayerRideOnMobileLandObjectP(twp->btimer, tp))
		{
			playertwp[twp->btimer]->pos.x += x;
		}
	}
}

static void ObjShelterCargostartExec_m(task* tp)
{
	auto twp = tp->twp;

	switch (twp->mode)
	{
	case MDCARGO_CHECKBOUNDS:
		twp->btimer = IsPlayerInSphere(&twp->pos, 95.0f) - 1;

		if (twp->btimer >= 0ui16)
		{
			PadReadOffP(twp->btimer);
			twp->mode = MDCARGO_CHECKGROUND;
			twp->flag |= 1; // enables dyncol
		}
		else
		{
			twp->flag &= ~1; // disables dyncol
		}

		break;
	case MDCARGO_CHECKGROUND:
		if (CheckPlayerRideOnMobileLandObjectP(twp->btimer, tp))
		{
			// Changes made here so that the train departs faster
			// That way it's ready in time for the next player

			twp->wtimer = 30ui16; // 0ui16
			twp->mode = MDCARGO_STARTFADE; // MDCARGO_WAIT
			twp->timer.f = 0.0f;
		}

		break;
	case MDCARGO_WAIT:
		++twp->wtimer;

		if (twp->wtimer >= 50ui16)
		{
			dsPlay_timer(325, (int)tp, 1, 0, 2);
		}

		if (twp->wtimer > 210ui16)
		{
			twp->wtimer = 0ui16;
			twp->mode = MDCARGO_STARTFADE;
		}

		break;
	case MDCARGO_STARTFADE:
		++twp->wtimer;
		dsPlay_timer(325, (int)tp, 1, 0, 2);
		if (twp->wtimer > 60ui16)
		{
			twp->wtimer = 0ui16;
			twp->mode = MDCARGO_TELEPORT;
			CreateObjShelterFade(twp->btimer);
		}

		twp->timer.f = min(20.0f, twp->timer.f + 0.1f);
		twp->pos.y = min(twp->value.f, twp->pos.y + 0.05f);

		Move(tp, -twp->timer.f);

		break;
	case MDCARGO_TELEPORT:
		Move(tp, -twp->timer.f);

		if (++twp->wtimer > 170ui16)
		{
			PadReadOnP(twp->btimer);
			PositionPlayer(twp->btimer, -48.0f, 10006.0f, -188.0f);
			twp->btimer = 0;
			twp->mode = MDCARGO_OUT;
		}
		break;
	case MDCARGO_OUT: // deletes the object in singleplayer, let's just reset in multiplayer
		if (tp->ocp && tp->ocp->pObjEditEntry)
		{
			Move(tp, twp->timer.f);

			if (twp->pos.x >= tp->ocp->pObjEditEntry->xpos)
			{
				twp->mode = MDCARGO_CHECKBOUNDS;
			}
		}
		else
		{
			FreeTask(tp);
		}

		break;
	}

	tp->disp(tp);
}

static void __cdecl ObjShelterCargostartExec_r(task* tp); // "Exec"
Trampoline ObjShelterCargostartExec_t(0x5A4B30, 0x5A4B38, ObjShelterCargostartExec_r);
static void __cdecl ObjShelterCargostartExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjShelterCargostartExec_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjShelterCargostartExec)(tp);
	}
}
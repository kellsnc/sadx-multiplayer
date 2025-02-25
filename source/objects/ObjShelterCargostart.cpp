#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"
#include "teleport.h"

// Objects in Gamma's Hot Shelter that teleports player to cargo area

#define PNUMS(twp) twp->ang.z
#define ADD_PNUM(twp, pnum) PNUMS(twp) |= (1 << pnum)
#define REMOVE_PNUM(twp, pnum) PNUMS(twp) &= ~(1 << pnum)
#define CHECK_PNUM(twp, pnum) PNUMS(twp) & (1 << pnum)

enum MD_CARGO // made up
{
	MDCARGO_CHECKBOUNDS,
	MDCARGO_CHECKGROUND,
	MDCARGO_WAIT,
	MDCARGO_STARTFADE,
	MDCARGO_TELEPORT,
	MDCARGO_OUT
};

static void CheckPlayerGround(task* tp)
{
	auto twp = tp->twp;
	
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		taskwk* ptwp = playertwp[i];

		if (ptwp)
		{
			// Check if on board
			if (CheckPlayerRideOnMobileLandObjectP(i, tp))
			{
				PadReadOffP(i);
				ADD_PNUM(twp, i);
			}
			
			// Remove if somehow got out of the train
			if (CHECK_PNUM(twp, i))
			{
				if (!CheckPlayerRideOnMobileLandObjectP(i, tp) || GetDistance(&twp->pos, &ptwp->pos) > 95.0f)
				{
					PadReadOnP(i);
					REMOVE_PNUM(twp, i);
				}
			}
		}
		else
		{
			// Remove if player doesn't exist anymore
			if (CHECK_PNUM(twp, i))
			{
				PadReadOnP(i);
				REMOVE_PNUM(twp, i);
			}
		}
	}
}

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

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (CHECK_PNUM(twp, i))
		{
			fwp[i].pos_spd.x = x;
		}
		else
		{
			fwp[i].pos_spd.x = 0.0f;
		}
	}
}

static void ObjShelterCargostartExec_m(task* tp)
{
	auto twp = tp->twp;

	CheckPlayerGround(tp);

	switch (twp->mode)
	{
	case MDCARGO_CHECKBOUNDS:
	case MDCARGO_CHECKGROUND:
		MirenObjCheckCollisionP(twp, 95.0f);

		if (PNUMS(twp)) // if at least one player is on it
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

			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				if (CHECK_PNUM(twp, i))
				{
					CreateObjShelterFade(i);
				}
			}
		}

		twp->timer.f = min(20.0f, twp->timer.f + 0.1f);
		twp->pos.y = min(twp->value.f, twp->pos.y + 0.05f);

		Move(tp, -twp->timer.f);

		break;
	case MDCARGO_TELEPORT:
		Move(tp, -twp->timer.f);

		if (++twp->wtimer > 170ui16)
		{
			for (int i = 0; i < PLAYER_MAX; ++i)
			{
				if (CHECK_PNUM(twp, i))
				{
					PadReadOnP(i);
					TeleportPlayer(i, -48.0f, 10006.0f, -188.0f);
				}
			}
			
			PNUMS(twp) = 0;
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
FastFunctionHookPtr<decltype(&ObjShelterCargostartExec_r)> ObjShelterCargostartExec_t(0x5A4B30, ObjShelterCargostartExec_r);
static void __cdecl ObjShelterCargostartExec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjShelterCargostartExec_m(tp);
	}
	else
	{
		ObjShelterCargostartExec_t.Original(tp);
	}
}
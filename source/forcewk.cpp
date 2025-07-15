#include "pch.h"
#include "FastFunctionHook.hpp"

// The force worker (what moves player on moving geometry collisions) is hardcoded for 2 players
// We patch if for 4 players

static constexpr size_t memsize = sizeof(forcewk) * PLAYER_MAX;

FastUsercallHookPtr<void(*)(playerwk* pwp, NJS_POINT3* vec, taskwk* twp, csts* cp), noret, rEAX, rECX, rESI, stack4> PSSGCollisionForceWorkEffect_h(0x43CA40);

static void __cdecl PSSGCollisionForceWorkEffect_r(playerwk* pwp, NJS_POINT3* vec, taskwk* twp, csts* cp)
{
	auto pnum = TASKWK_PLAYERID(twp);

	if (pnum < 0 || pnum > 4)
	{
		return;
	}

	if (!multiplayer::IsActive() || EV_MainThread_ptr)
	{
		return PSSGCollisionForceWorkEffect_h.Original(pwp, vec, twp, cp);
	}

	auto fwp = pwp->ttp->fwp;

	if (!fwp)
	{
		return;
	}

	fwp = &fwp[pnum];

	if (njScalor(&pwp->spd) == 0.0f)
	{
		cp->pos.x = twp->pos.x + vec->x;
		cp->pos.y = twp->pos.y + vec->y;
		cp->pos.z = twp->pos.z + vec->z;
	}

	if (fwp->call_back) 
	{
		auto x = twp->pos;
		twp->pos = cp->pos;
		fwp->call_back(playertp[pnum], twp, fwp);
		twp->pos = x;
	}

	cp->pos.x = fwp->pos_spd.x + cp->pos.x;
	cp->pos.y = fwp->pos_spd.y + cp->pos.y;
	cp->pos.z = fwp->pos_spd.z + cp->pos.z;

	twp->ang.x = (unsigned __int16)(twp->ang.x + fwp->ang_spd.x);
	twp->ang.y = (unsigned __int16)(twp->ang.y + fwp->ang_spd.y);
	twp->ang.z = (unsigned __int16)(twp->ang.z + fwp->ang_spd.z);
}

static void* GetForceWork_r()
{
	auto mem = AllocateMemory((int)memsize);

	if (mem)
	{
		memset(mem, 0, memsize);
	}

	return mem;
}

void InitForceWorkPatches()
{
	PSSGCollisionForceWorkEffect_h.Hook(PSSGCollisionForceWorkEffect_r);
	WriteCall((void*)0x40B8AB, GetForceWork_r);
	WriteData<1>((void*)0x57C33C, PLAYER_MAX); // also allocate more memory for viper platform since fwp is set manually 
}
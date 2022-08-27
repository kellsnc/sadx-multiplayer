#include "pch.h"
#include "bosses.h"


static const int timeLimit = 300;

static FunctionHook<void, taskwk*, chaoswk*> turnToPlayer_t(0x7AE660);
static FunctionHook<void, task*> chaos0_t(0x548640);

//Patches Chaos effects to make them display on other player screens, it is done by manually setting a disp function that is lacking in vanilla.

void __cdecl ExecEffectChaos0AttackB(task* obj)
{
	SetAndDisp(obj, drawEffectChaos0EffectB);
}

void __cdecl ExecEffectChaos0LightParticleB(task* obj)
{
	SetAndDisp(obj, drawEffectChaos0LightParticle);
}

void __cdecl ExecEffectChaos0AttackA(task* obj)
{
	SetAndDisp(obj, dispEffectChaos0AttackA);
}

//Some Chaos effects don't have the flag data1 so we add it first
task* LoadChaos0AttackEffB(LoadObj flags, int index, void(__cdecl* loadSub)(task*))
{
	return CreateElementalTask(LoadObj_Data1, index, ExecEffectChaos0AttackB);
}

task* LoadChaos0LightParticleB(LoadObj flags, int index, void(__cdecl* loadSub)(task*))
{
	return CreateElementalTask(LoadObj_Data1, index, ExecEffectChaos0LightParticleB);
}

void turnToPlayer_r(taskwk* twp, chaoswk* bwp)
{
	if (!multiplayer::IsActive())
	{
		return turnToPlayer_t.Original(twp, bwp);
	}

	float dist = 0.0f;

	if (playertwp[randomPnum])
	{
		dist = atan2((float)(playertwp[randomPnum]->pos.z - twp->pos.z), (float)(playertwp[randomPnum]->pos.x - twp->pos.x));
		twp->ang.y = AdjustAngle(
			twp->ang.y,
			(int)(dist * 65536.0f * 0.1591549762031479f) - bwp->attack_yang,
			chaosparam->turn_spd);
	}
}


void Chaos0_Exec_r(task* tp)
{
	if (tp->twp && !tp->twp->mode)
		ResetBossRNG();

	chaos0_t.Original(tp);
	Boss_SetNextPlayerToAttack(timeLimit);
}

void initChaos0Patches()
{
	WriteCall((void*)0x7AD3F3, LoadChaos0AttackEffB);	
	WriteCall((void*)0x7AD388, LoadChaos0LightParticleB);	
	WriteCall((void*)0x7AD75D, LoadChaos0LightParticleB);
	WriteData((TaskFuncPtr*)0x7AD221, ExecEffectChaos0AttackA);
	turnToPlayer_t.Hook(turnToPlayer_r);
	chaos0_t.Hook(Chaos0_Exec_r);
}
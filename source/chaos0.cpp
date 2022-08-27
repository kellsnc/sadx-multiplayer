#include "pch.h"

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

void initChaos0Patches()
{
	WriteCall((void*)0x7AD3F3, LoadChaos0AttackEffB);	
	WriteCall((void*)0x7AD388, LoadChaos0LightParticleB);	
	WriteCall((void*)0x7AD75D, LoadChaos0LightParticleB);
	WriteData((TaskFuncPtr*)0x7AD221, ExecEffectChaos0AttackA);
}
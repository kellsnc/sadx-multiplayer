#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"

// Egg Walker

FastFunctionHook<void, task*> Egm2_h(0x576650);

void Egm2_r(task* tp)
{
	Egm2_h.Original(tp);
}

void patch_egm3_init()
{
	//Egm2_h.Hook(Egm2_r);
}

#ifdef MULTI_TEST
RegisterPatch patch_egm3(patch_egm3_init);
#endif
#include "pch.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"

//static auto chkDamage = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5AC700, rEAX);
//static auto smoke = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5AC8C0, rEAX);
//static auto addVectorAng = GenerateUsercallWrapper<void (*)(Angle3* ang, MISSILE_WK* wk, float force)>(noret, 0x5AC860, rEDI, rESI, stack4); // custom name

static void __cdecl EnemyHotTgtMissile_exec_r(task* tp);
FastFunctionHookPtr<decltype(&EnemyHotTgtMissile_exec_r)> EnemyHotTgtMissile_exec_t(0x5ACB70);

// Todo: rewrite properly
static void __cdecl EnemyHotTgtMissile_exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto pltwp = playertwp[0];
		playertwp[0] = playertwp[GetClosestPlayerNum(&tp->twp->pos)];
		EnemyHotTgtMissile_exec_t.Original(tp);
		playertwp[0] = pltwp;
	}
	else
	{
		EnemyHotTgtMissile_exec_t.Original(tp);
	}
}

void patch_hot_e105missile_init()
{
	EnemyHotTgtMissile_exec_t.Hook(EnemyHotTgtMissile_exec_r);
}

RegisterPatch patch_hot_e105missile(patch_hot_e105missile_init);
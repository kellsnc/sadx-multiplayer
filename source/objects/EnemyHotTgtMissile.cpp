#include "pch.h"
#include <UsercallFunctionHandler.h>
#include "multiplayer.h"

//static auto chkDamage = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5AC700, rEAX);
//static auto smoke = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5AC8C0, rEAX);
//static auto addVectorAng = GenerateUsercallWrapper<void (*)(Angle3* ang, MISSILE_WK* wk, float force)>(noret, 0x5AC860, rEDI, rESI, stack4); // custom name

// Unknown main struct for large function so I cannot rewrite properly yet
static void __cdecl EnemyHotTgtMissile_exec_r(task* tp);
Trampoline EnemyHotTgtMissile_exec_t(0x5ACB70, 0x5ACB75, EnemyHotTgtMissile_exec_r);
static void __cdecl EnemyHotTgtMissile_exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto pltwp = playertwp[0];
		playertwp[0] = playertwp[GetClosestPlayerNum(&tp->twp->pos)];
		TARGET_STATIC(EnemyHotTgtMissile_exec)(tp);
		playertwp[0] = pltwp;
	}
	else
	{
		TARGET_STATIC(EnemyHotTgtMissile_exec)(tp);
	}
}
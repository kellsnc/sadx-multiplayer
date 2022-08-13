#include "pch.h"
#include "multiplayer.h"

static void MoveSnake_w();
Trampoline MoveSnake_t(0x5E4360, 0x5E4367, MoveSnake_w);

VariableHook<int, 0x3C7ED74> semafo_m;

DataPointer(char, flag_1, 0x3C7ED84);
DataPointer(float, suimen_ypos, 0x20397A0);
DataPointer(float, up_up_speed, 0x3C7ED70);

static void MoveSnake_o(pathtbl* ptag, float onpathpos, task* tp)
{
	auto target = MoveSnake_t.Target();
	__asm
	{
		push [tp]
		push [onpathpos]
		mov eax, [ptag]
		call target
		add esp, 8
	}
}

static void MoveSnake_m(pathtbl* ptag, float onpathpos, task* tp)
{
	auto twp = tp->twp;
	auto parent_twp = tp->ptp->twp;

	auto current_pt = onpathpos < parent_twp->scl.z ? &ptag[(int)onpathpos] : ptag;

	NJS_POINT3 pos;
	pos.x = current_pt->xpos - twp->pos.x;
	pos.y = current_pt->ypos - twp->pos.y;
	pos.z = current_pt->zpos - twp->pos.z;

	auto next_pt = &ptag[(int)onpathpos + 1];

	Angle add_ang_x = SubAngle(twp->ang.x, next_pt->slangx);
	Angle add_ang_z = SubAngle(twp->ang.z, next_pt->slangz);

	twp->ang.z = next_pt->slangz;
	twp->ang.x = next_pt->slangx;
	twp->scl.z += 1.0f;

	double x, z;
	if (onpathpos + 2.0f < parent_twp->scl.z)
	{
		x = next_pt[2].xpos - next_pt->xpos;
		z = next_pt[2].zpos - next_pt->zpos;
	}
	else if (onpathpos + 2.0f != parent_twp->scl.z)
	{
		twp->scl.z = 0.0f;
		x = current_pt[1].xpos - next_pt->xpos;
		z = current_pt[1].zpos - next_pt->zpos;
	}
	else
	{
		x = current_pt->xpos - next_pt[1].xpos;
		z = current_pt->zpos - next_pt[1].zpos;
	}

	Angle dest_ang = NJM_RAD_ANG(atan2(x, z));
	Angle add_ang_y = SubAngle(twp->ang.y, dest_ang);
	twp->ang.y = dest_ang;

	twp->pos.x = pos.x + twp->pos.x;
	twp->pos.y = pos.y + twp->pos.y;
	twp->pos.z = pos.z + twp->pos.z;

	auto head = (task*)parent_twp->counter.ptr;
	
	if (twp->btimer == 2 || twp->btimer == 3 || !twp->btimer || twp->btimer == 5)
	{
		if (twp->cwp->flag & 1)
		{
			head->twp->flag |= 0x100;
			flag_1 = 1;
		}
		else if (!flag_1)
		{
			head->twp->flag &= ~0x100;
		}

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			if (semafo_m[i])
			{
				continue;
			}

			if (CheckPlayerRideOnMobileLandObjectP(i, head))
			{
				auto ptwp = playertwp[i];

				NJS_POINT3 p = twp->pos;
				njAddVector(&p, &twp->cwp->info->center);
				if (GetDistance(&p, &ptwp->pos) < twp->cwp->info->a)
				{
					head->fwp[i].pos_spd = pos;
					head->fwp[i].ang_spd.x = add_ang_x;
					head->fwp[i].ang_spd.y = add_ang_y;
					head->fwp[i].ang_spd.z = add_ang_z;
					semafo_m[i] = 1;
				}
			}
		}
	}
}

static void __cdecl MoveSnake_r(pathtbl* ptag, float onpathpos, task* tp)
{
	if (multiplayer::IsActive())
	{
		MoveSnake_m(ptag, onpathpos, tp);
	}
	else
	{
		MoveSnake_o(ptag, onpathpos, tp);
	}
}

static void __declspec(naked) MoveSnake_w()
{
	__asm
	{
		push[esp + 08h]
		push[esp + 08h]
		push eax
		call MoveSnake_r
		pop eax
		add esp, 8
		retn
	}
}

static void __cdecl ObjectRuinSnake_r(task* tp);
Trampoline ObjectRuinSnake_t(0x5E45C0, 0x5E45C7, ObjectRuinSnake_r);
static void __cdecl ObjectRuinSnake_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode == 2)
	{
		auto twp = tp->twp;

		if (suimen_ypos != 0.0f)
		{
			auto v3 = suimen_ypos - (twp->scl.x + twp->pos.y);
			if (v3 <= 10.0f)
			{
				if (v3 >= -10.0f)
				{
					up_up_speed = 0.0f;
				}
				else
				{
					up_up_speed = -0.8f;
					twp->pos.y -= up_up_speed;
				}
			}
			else
			{
				up_up_speed = 0.8f;
				twp->pos.y += up_up_speed;
			}
		}

		if (IsPlayerInSphere(5207.0f, -2557.0f, 1197.0f, 800.0f))
		{
			LoopTaskC(tp);

			for (auto& i : semafo_m)
			{
				i = 0;
			}
		}

		flag_1 = 0;
	}
	else
	{
		TARGET_STATIC(ObjectRuinSnake)(tp);
	}
}
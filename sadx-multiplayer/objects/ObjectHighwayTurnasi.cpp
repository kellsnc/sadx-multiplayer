#include "pch.h"
#include "multiplayer.h"

// ObjectHighwayTurnasi
// Rewrite collision detection for multiplayer and set the detected player id in twp->smode
// Rewrite logic to use the detected player id instead of player 0

enum MD_COLASI // made up
{
	MD_COLASI_INIT,
	MD_COLASI_CHECK,
	MD_COLASI_STOP
};

enum MD_TURNASI // made up
{
	MD_TURNASI_INIT,
	MD_TURNASI_WAIT,
	MD_TURNASI_LAUNCH,
	MD_TURNASI_RESET,
	MD_TURNASI_DEADOUT
};

static void AsiCollisionCollision_w();
static void ObjectHighwayTurnasiHit_w();
static void ObjectHighwayTurnasiNormal_w();

Trampoline AsiCollisionCollision_t(0x618CE0, 0x618CE5, AsiCollisionCollision_w);
Trampoline ObjectHighwayTurnasiHit_t(0x618F50, 0x618F56, ObjectHighwayTurnasiHit_w);
Trampoline ObjectHighwayTurnasiNormal_t(0x618B10, 0x618B15, ObjectHighwayTurnasiNormal_w);

#pragma region "Collision" subroutine from AsiCollision
static void AsiCollisionCollision_o(task* tp)
{
	auto target = AsiCollisionCollision_t.Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

static void AsiCollisionCollision_m(task* tp)
{
	auto twp = tp->twp; // this task work (collision child task)
	auto turnasi_twp = tp->ptp->twp; // parent task work (main task)

	turnasi_twp->btimer = 0;

	if (turnasi_twp->scl.y == 5.0f)
	{
		twp->mode = MD_COLASI_STOP;
		twp->scl.y = 10.0f;
		turnasi_twp->scl.y = 0.0f;
	}
	else
	{
		NJS_VECTOR v { 0.0f, 0.0f, 0.0f };
		njPushMatrix(_nj_unit_matrix_);
		njTranslate(0, turnasi_twp->pos.x, turnasi_twp->pos.y, turnasi_twp->pos.z);
		njRotateY_(turnasi_twp->ang.y);
		njRotateX_(turnasi_twp->ang.x + twp->ang.x);
		njTranslate(0, object_turnasi_oya_koa.pos[0], object_turnasi_oya_koa.pos[1], object_turnasi_oya_koa.pos[2]);
		njTranslate(0, object_turnasi_oya_kob.pos[0], object_turnasi_oya_kob.pos[1], object_turnasi_oya_kob.pos[2]);
		njCalcPoint(0, &v, &v);
		njPopMatrixEx();

		twp->pos = v;
		twp->ang.y = turnasi_twp->ang.y;

		if (turnasi_twp->scl.z != 20.0f)
		{
			if (turnasi_twp->scl.z != 10.0f)
			{
				if (twp->cwp->flag & 1)
				{
					twp->cwp->flag &= ~1;
					auto ptwp = CCL_IsHitPlayer(twp);

					if (!ptwp)
					{
						ptwp = playertwp[GetTheNearestPlayerNumber(&twp->pos)];
					}

					twp->btimer = TASKWK_PLAYERID(ptwp);

					auto pwp = playerpwp[twp->btimer];

					turnasi_twp->value.f = njScalor(&pwp->spd);
					turnasi_twp->btimer = 10;
					twp->scl.z = 10.0f; 
					twp->scl.x = max(-15.0f, min(15.0f, ptwp->pos.x - twp->pos.x));
					twp->value.f = max(-15.0f, min(15.0f, ptwp->pos.z - twp->pos.z));

				}
			}

			EntryColliList(twp);
		}
	}
}

static void __cdecl AsiCollisionCollision_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		AsiCollisionCollision_m(tp);
	}
	else
	{
		AsiCollisionCollision_o(tp);
	}
}

static void __declspec(naked) AsiCollisionCollision_w()
{
	__asm
	{
		push eax
		call AsiCollisionCollision_r
		pop eax
		retn
	}
}
#pragma endregion

#pragma region "Hit" subroutine from ObjectHighwayTurnasi
static void ObjectHighwayTurnasiHit_o(taskwk* twp)
{
	auto target = ObjectHighwayTurnasiHit_t.Target();
	__asm
	{
		mov esi, [twp]
		call target
	}
}

static void ObjectHighwayTurnasiHit_m(taskwk* twp)
{
	auto ctp = reinterpret_cast<task*>(twp->timer.ptr);

	if (!ctp)
	{
		return;
	}

	auto pnum = ctp->twp->btimer;
	auto ptwp = playertwp[pnum];
	auto ppwp = playerpwp[pnum];

	if (!ptwp || !ppwp)
	{
		return;
	}

	twp->scl.z = 20.0f;
	ptwp->pos.x = ctp->twp->scl.x + ctp->twp->pos.x;
	ptwp->pos.y = ctp->twp->pos.y - 7.5f;
	ptwp->pos.z = ctp->twp->pos.z;

	float t = static_cast<float>(++twp->wtimer);
	float vel;

	if (twp->value.f < 4.0f && twp->scl.x != 5.0f)
	{
		if (twp->value.f <= 3.0f)
		{
			vel = 8.0f - t / twp->value.f * 0.63;
		}
		else
		{
			vel = 8.0f - t / twp->value.f * 0.13;
		}

		if (vel >= 0.0f)
		{
			if (vel > 4.3f)
			{
				vel = 4.3f;
			}

			ctp->twp->ang.x = NJM_DEG_ANG(vel);
			twp->ang.x += ctp->twp->ang.x;
		}
		else
		{
			vel = fabs(vel);

			if (vel > 4.3f)
			{
				vel = 4.3f;
			}

			ctp->twp->ang.x = NJM_DEG_ANG(-vel);
			twp->ang.x += ctp->twp->ang.x;
		}

		if (twp->ang.x >= 0)
		{
			if (twp->ang.x <= 0x8000)
			{
				return;
			}

			twp->mode = MD_TURNASI_RESET;

			NJS_VECTOR v { twp->value.f + 3.0f, 0.0f, 0.0f };
			Angle3 ang{ 0, twp->ang.y, 0 };
			SetVelocityAndRotationAndNoconTimeP(pnum, &v, &ang, 10);
		}
		else
		{
			twp->mode = MD_TURNASI_WAIT;

			NJS_VECTOR v{ twp->value.f + 1.0f, 0.0f, 0.0f };
			Angle3 ang{ 0, twp->ang.y, 0 };
			SetVelocityAndRotationAndNoconTimeP(pnum, &v, &ang, 10);
		}

		dsPlay_oneshot(101, 0, 0, 0);
	}
	else
	{
		twp->scl.x = 5.0;
		vel = (8.0f - t / twp->value.f * 0.2f) * 0.5f;

		if (vel >= 0.0f)
		{
			ctp->twp->ang.x = NJM_DEG_ANG(vel);
			twp->ang.x += ctp->twp->ang.x;
		}
		else
		{
			ctp->twp->ang.x = NJM_DEG_ANG(-fabs(vel));
			twp->ang.x += ctp->twp->ang.x;
		}

		if (twp->ang.x >= 0)
		{
			if (NJM_ANG_DEG(twp->ang.x) < 180.0f)
			{
				return;
			}

			twp->mode = MD_TURNASI_RESET;

			NJS_VECTOR v{ twp->value.f + 1.0f, 0.0f, 0.0f };
			Angle3 ang{ 0, twp->ang.y, 0 };
			
			SetVelocityAndRotationAndNoconTimeP(pnum, &v, &ang, 10);
			dsPlay_oneshot(101, 0, 0, 0);
		}
		else
		{
			twp->mode = MD_TURNASI_WAIT;

			NJS_VECTOR v{ 0.0f, 0.0f, twp->value.f + 2.0f };
			njPushMatrix(_nj_unit_matrix_);
			njRotateY_(twp->ang.y);
			njCalcPoint(0, &v, &v);
			njPopMatrixEx();
			SetVelocityP(pnum, v.x, v.y, v.z);
		}
	}

	SetInputP(pnum, 24);
	twp->value.l = 0;
	twp->wtimer = 0;
	twp->scl.y = 5.0f;
	twp->scl.z = 0.0f;
}

static void __cdecl ObjectHighwayTurnasiHit_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		ObjectHighwayTurnasiHit_m(twp);
	}
	else
	{
		ObjectHighwayTurnasiHit_o(twp);
	}
}

static void __declspec(naked) ObjectHighwayTurnasiHit_w()
{
	__asm
	{
		push esi
		call ObjectHighwayTurnasiHit_r
		pop esi
		retn
	}
}
#pragma endregion

#pragma region "Normal" subroutine from ObjectHighwayTurnasi
static void ObjectHighwayTurnasiNormal_o(task* tp)
{
	auto target = ObjectHighwayTurnasiNormal_t.Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

static void __cdecl ObjectHighwayTurnasiNormal_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto ctp = reinterpret_cast<task*>(twp->timer.ptr);

		EntryColliList(twp);

		// The child task sets this to 10. Why 10? I don't know!
		if (twp->btimer == 10)
		{
			if (twp->value.f >= 1.5f)
			{
				dsPlay_oneshot(100, 0, 0, 0);
				twp->mode = MD_TURNASI_LAUNCH;

				auto pnum = ctp->twp->btimer;

				if (twp->value.f >= 4.0f)
				{
					SetInputP(pnum, 13);
				}
				else
				{
					SetInputP(pnum, 15);
				}
			}
			else
			{
				twp->btimer = 0;
				twp->ang.x = 0;
				twp->scl.x = 0.0;
				twp->scl.y = 0.0;
				twp->scl.z = 0.0;

				if (ctp)
				{
					ctp->twp->scl.z = 0.0f;
				}
			}
		}
	}
	else
	{
		ObjectHighwayTurnasiNormal_o(tp);
	}
}

static void __declspec(naked) ObjectHighwayTurnasiNormal_w()
{
	__asm
	{
		push eax
		call ObjectHighwayTurnasiNormal_r
		pop eax
		retn
	}
}
#pragma endregion
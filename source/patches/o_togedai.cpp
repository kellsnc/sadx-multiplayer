#include "pch.h"
#include "multiplayer.h"
#include "camera.h"

// Snake door in Lost World

DataPointer(Bool, kouka_flag, 0x3C7EE0C);
DataPointer(int, last_counter, 0x3C7EE08);

enum : char
{
	MODE_0,
	MODE_1,
	MODE_2,
	MODE_3,
	MODE_4,
	MODE_5,
	MODE_20 = 20i8
};

static void __cdecl ObjectTogedai_Normal_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectTogedai_Normal_r)> ObjectTogedai_Normal_t(0x5EA7A0);

static void effect(taskwk* twp)
{
	auto pnum = GetClosestPlayerNum(&twp->pos);
	auto cam_pos = GetCameraPosition(pnum);

	auto ang = NJM_RAD_ANG(atan2f(cam_pos->x - twp->pos.x, cam_pos->z - twp->pos.z));

	NJS_POINT3 p;
	p.x = njSin(ang) * 20.0f;
	p.y = -12.0f;
	p.z = njCos(ang) * 20.0f;

	njPushMatrix(_nj_unit_matrix_);
	njRotateZ_(twp->ang.z);
	njRotateX_(twp->ang.x);
	njRotateY_(twp->ang.y);
	njCalcVector(0, &p, &p);
	njPopMatrixEx();
	njAddVector(&p, &twp->pos);

	NJS_VECTOR velo{};

	CreateSmoke(&p, &velo, 3.0f);
}

static void move(taskwk* twp, float y)
{
	if (twp->mode != 5)
	{
		NJS_VECTOR p = { 0.0f, y, 0.0f };
		njPushMatrix(_nj_unit_matrix_);
		njRotateZ_(twp->ang.z);
		njRotateX_(twp->ang.x);
		njRotateY_(twp->ang.y);
		njCalcVector(0, &p, &p);
		njPopMatrixEx();
		njAddVector(&twp->pos, &p);
	}
}

static void ObjectTogedai_Normal_m(task* tp)
{
	if (CheckRangeOut(tp))
	{
		return;
	}

	auto twp = tp->twp;

	switch (twp->mode)
	{
	case MODE_0:
		if (IsPlayerInSphere(&twp->pos, 60.0f))
		{
			dsPlay_oneshot(204, 0, 0, 0);
			kouka_flag = TRUE;
			twp->mode = MODE_20;
		}
		else
		{
			if (kouka_flag == TRUE)
			{
				twp->mode = MODE_20;
			}
		}
		break;
	case MODE_1:
		move(twp, twp->scl.y);
		twp->scl.z += twp->scl.y;
		kouka_flag = 0;

		if (twp->scl.z < twp->scl.x)
		{
			effect(twp);
			twp->mode = MODE_2;
			twp->btimer = 30i8;
		}
		break;
	case MODE_2:
		if ((twp->flag & 0x100) == 0)
		{
			if (--twp->btimer == 0)
				twp->mode = MODE_3;
		}
		else
		{
			twp->mode = MODE_5;
		}
		break;
	case MODE_3:
		move(twp, twp->scl.y * -0.1f);
		twp->scl.z += twp->scl.y * -0.1f;

		if (twp->scl.z > 0)
		{
			twp->pos.x = twp->counter.f;
			twp->pos.y = twp->timer.f;
			twp->pos.z = twp->value.f;
			twp->btimer = 10i8;
			twp->mode = MODE_4;
		}

		break;
	case MODE_4:
		if (--twp->btimer == 0)
			twp->mode = MODE_0;
		break;
	case MODE_20:
		if (GameTimer != last_counter)
		{
			kouka_flag = 2;
			twp->mode = MODE_1;
		}
		if (kouka_flag == 2)
		{
			twp->mode = MODE_1;
		}
		break;
	}

	last_counter = GameTimer;
	EntryColliList(twp);
	ObjectSetupInput(twp, nullptr);
	tp->disp(tp);
}

static void __cdecl ObjectTogedai_Normal_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjectTogedai_Normal_m(tp);
	}
	else
	{
		ObjectTogedai_Normal_t.Original(tp);
	}
}

void patch_togedai_init()
{
	ObjectTogedai_Normal_t.Hook(ObjectTogedai_Normal_r);
}

RegisterPatch patch_togedai(patch_togedai_init);
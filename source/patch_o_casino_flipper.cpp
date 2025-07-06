#include "pch.h"

#define FLAG_FLIPPER_X			0x200				/* Given to Lx/Rx Flippers but has no purpose. */
#define FLAG_FLIPPER_SPDDOWN	0x2000				/* Slower bounce for the outer edges.*/
#define FLAG_FLIPPER_L			0x4000				/* Left Flipper */
#define FLAG_FLIPPER_R			0x8000				/* Right Flipper */

static Float PINPOWER = 8.3f;
FastFunctionHook<void, task*> ObjectFlipperLExec_h(0x5DC890);
FastFunctionHook<void, task*> ObjectFlipperRExec_h(0x5DC9E0);

static void CalcCenter(task* tp)
{
	taskwk* twp = tp->twp;
	NJS_VECTOR center = { 14.0f, 0.0f, 0.0f };
	twp->cwp->info->angy = twp->timer.l + twp->value.l;
	njPushMatrixEx();
	njUnitMatrix(0);
	ROTATEZ(0, twp->ang.z);
	ROTATEX(0, twp->ang.x);
	ROTATEY(0, twp->cwp->info->angy);
	njCalcPoint(0, &center, &center);
	twp->cwp->info->center = center;
	njPopMatrix(1);
}

static void BallSpeedDownM(task* tp, const uint8_t pnum)
{
	taskwk* twp = tp->twp;
	colliwk* cwp = twp->cwp;
	if (cwp->flag & CWK_FLAG_HIT && !cwp->hit_cwp->id)
	{
		NJS_VECTOR scalor = {
			twp->pos.x - playertwp[pnum]->pos.x,
			twp->pos.y - playertwp[pnum]->pos.y,
			twp->pos.z - playertwp[pnum]->pos.z
		};

		if (njScalor(&scalor) >= 1.0f)
		{
			NJS_VECTOR spd = playermwp[pnum]->spd;
			Float velo = njScalor(&spd);
			if (velo >= 0.05f)
			{
				njUnitVector(&scalor);
				njUnitVector(&spd);
				if (twp->flag & FLAG_FLIPPER_SPDDOWN && velo > 0.6f)
				{
					velo *= 0.9f;
					SetVelocityP(pnum, spd.x * velo, spd.y * velo, spd.z * velo);
				}
			}
		}
	}
}

static Float CalcPlayerSide(Angle angy, taskwk* twp, const uint8_t pnum)
{
	NJS_VECTOR vec;
	GetPlayerPosition(pnum, 1, &vec, 0);
	vec.x -= twp->pos.x;
	vec.y -= twp->pos.y;
	vec.z -= twp->pos.z;
	njUnitVector(&vec);
	NJS_VECTOR vd = { 0.966f, 0.0f, 0.259f };
	if (twp->flag & FLAG_FLIPPER_R)
		vd.x = -vd.x;
	njPushMatrixEx();
	njUnitMatrix(0);
	ROTATEZ(0, twp->ang.z);
	ROTATEX(0, twp->ang.x);
	ROTATEY(0, angy);
	njCalcPoint(0, &vd, &vd);
	njPopMatrix(1);
	njOuterProduct(&vd, &vec, &vd);
	return vd.y;
}

static Angle CalcAngleFlipperHitPlayer(taskwk* twp, const uint8_t pnum)
{
	NJS_VECTOR pos;
	GetPlayerPosition(pnum, 1, &pos, 0);
	pos.x -= twp->pos.x;
	pos.y -= twp->pos.y;
	pos.z -= twp->pos.z;
	njUnitVector(&pos);
	NJS_VECTOR vd = { 1.0f, 0.0f, 0.0f };
	if (twp->flag & FLAG_FLIPPER_R)
		vd.x = -vd.x;
	njPushMatrixEx();
	njUnitMatrix(0);
	ROTATEZ(0, twp->ang.z);
	ROTATEX(0, twp->ang.x);
	ROTATEY(0, twp->value.l + twp->timer.l + twp->ang.y);
	njCalcPoint(0, &vd, &vd);
	njPopMatrix(1);
	return njArcCos(njInnerProduct(&vd, &pos));
}

static void ActionFlipper(taskwk* twp, Angle angy, const uint8_t pnum)
{
	taskwk* pl_twp = playertwp[pnum];
	NJS_VECTOR velo = { 0.0f, 0.0f, 1.0f };
	if (twp->flag & FLAG_FLIPPER_L)
		velo.z = -velo.z;
	njPushMatrixEx();
	njUnitMatrix(0);
	Angle hitangle = CalcAngleFlipperHitPlayer(twp, pnum);
	Angle angle = (Sint16)SubAngle(0, angy) / 2;
	if (angle <= hitangle) {
		if (angle < -hitangle) {
			angle = -hitangle;
		}
	}
	else {
		angle = hitangle;
	}

	ROTATEZ(0, twp->ang.z);
	ROTATEX(0, twp->ang.x);
	ROTATEY(0, twp->value.l + angle + twp->timer.l + twp->ang.y);
	NJS_VECTOR vd;
	njCalcPoint(0, &velo, &vd);
	njPopMatrix(1);
	if (CalcPlayerSide(twp->value.l + twp->timer.l + twp->ang.y, twp, pnum) < 0.0f)
	{
		vd.x = -vd.x;
		vd.y = -vd.y;
		vd.z = -vd.z;
	}
	NJS_VECTOR vd2 = vd;
	NJS_LINE line = { 0.0f, 0.0f, 0.0f, vd.x, vd.y, vd.z };
	NJS_VECTOR v = { pl_twp->pos.x - twp->pos.x, pl_twp->pos.y - twp->pos.y, pl_twp->pos.z - twp->pos.z };
	Float sin = njSin(angy);
	if (twp->flag & FLAG_FLIPPER_R)
	{
		sin = -sin;
	}
	velo = playermwp[pnum]->spd;
	Float innerfac = njAbs(sin) * (twp->scl.x + 1.0f) * sin * njScalor(&v) * PINPOWER;
	NJS_VECTOR scalor;
	njDistanceP2PL(&velo, &line, &scalor);
	vd2.x = velo.x - scalor.x;
	vd2.y = velo.y - scalor.y;
	vd2.z = velo.z - scalor.z;

	NJS_POINT3 inner = { innerfac * vd.x, innerfac * vd.y, innerfac * vd.z };

	if (njInnerProduct(&velo, &inner) >= 0.0f) {
		if (njScalor(&inner) > njScalor(&vd2))
			vd2 = inner;
	}
	else if (njInnerProduct(&vd, &inner) < 0.0f)
	{
		vd2 = inner;
	}
	else
	{
		Float fac = 0.0f;
		if (innerfac)
			fac = -0.5f;
		vd2.y *= fac;
		vd2.z *= fac;
		vd2.x = inner.x + vd2.x * fac;
		vd2.y += inner.y;
		vd2.z += inner.z;
	}
	if (njScalor(&scalor) > 1.0f)
	{
		scalor.x *= 0.81f;
		scalor.y *= 0.81f;
		scalor.z *= 0.81f;
	}
	velo.x = scalor.x + vd2.x;
	velo.y = scalor.y + vd2.y;
	velo.z = scalor.z + vd2.z;
	SetVelocityP(pnum, velo.x, velo.y, velo.z);
}

static void NormalL(task* tp)
{
	if (multiplayer::IsActive() == false)
	{
		return ObjectFlipperLExec_h.Original(tp);
	}

	taskwk* twp = tp->twp;
	Sint32 timer = twp->timer.l;
	tp->disp(tp);

	Angle angle;
	if (timer <= 0x2C00)
	{
		if (timer <= 0x2000)
			angle = timer <= 0x1000 ? 0xC00 : 0x800;
		else
			angle = 0x400;
	}
	else
	{
		angle = 0x100;
	}

	Angle angscl = (Angle)((Float)angle * (twp->scl.x + 1.0f));

	auto pCount = multiplayer::GetPlayerCount();
	bool quit = false;
	for (uint8_t i = 0; i < pCount; i++)
	{
		if ((perG[i].l - 0x80) << 8 > 0x80)
		{
			quit = true;
			angle = angscl + timer;
		}
			

		//A Button or trigger pressed
		if ((perG[i].l - 0x80) << 8 > 0x80 || per[i]->on & Buttons_A)
		{
			quit = true;
			angle = angscl + timer;
		}
		else
			angle = timer - angscl;

		if (quit)
			break;
	}

	if (angle <= 0x3000)
	{
		twp->flag &= ~FLAG_FLIPPER_SPDDOWN;
		if (angle < 0)
			angle = 0;
	}
	else
	{
		twp->flag |= FLAG_FLIPPER_SPDDOWN;
		angle = 0x3000;
	}

	//
	if (angle == angscl && !timer)
		dsPlay_oneshot(SE_CA_FLIPPER, 0, 0, 0);

	int pnum = 0;
	//Bounce the player
	if (twp->cwp->flag & CWK_FLAG_HIT)
	{
		auto player = CCL_IsHitPlayer(twp);
		pnum = player->counter.b[0];
		ActionFlipper(twp, angle - timer, pnum);
	}

	twp->timer.l = angle;
	BallSpeedDownM(tp, pnum);
	CalcCenter(tp);
	EntryColliList(twp);
	ObjectSetupInput(twp, 0);
}

static void NormalR(task* tp)
{
	if (multiplayer::IsActive() == false)
	{
		return ObjectFlipperRExec_h.Original(tp);
	}
	taskwk* twp = tp->twp;
	Sint32 timer = twp->timer.l;
	tp->disp(tp);

	Angle angle;
	if (timer >= (Angle)-0x2C00)
	{
		if (timer >= (Angle)-0x2000)
			angle = timer >= (Angle)-0x1000 ? 0xC00 : 0x800;
		else
			angle = 0x400;
	}
	else
	{
		angle = 0x100;
	}

	Angle angscl = (Angle)((Float)angle * (twp->scl.x + 1.0f));
	auto pCount = multiplayer::GetPlayerCount();
	bool quit = false;
	for (uint8_t i = 0; i < pCount; i++)
	{

		if ((perG[i].r - 0x80) << 8 > 0x80)
		{
			angle = timer - angscl;
			quit = true;
		}
		

		//A Button or trigger pressed
		if ((perG[i].r - 0x80) << 8 > 0x80 || per[i]->on & Buttons_A)
		{
			angle = timer - angscl;
			quit = true;
		}
		else
		{
			angle = angscl + timer;
		}

		if (quit)
			break;
	}

	if (angle >= (Angle)-0x3000)
	{
		twp->flag &= ~FLAG_FLIPPER_SPDDOWN;
		if (angle > 0)
			angle = 0;
	}
	else
	{
		twp->flag |= FLAG_FLIPPER_SPDDOWN;
		angle = -0x3000;
	}


	if (angle == -angscl && !timer)
		dsPlay_oneshot(SE_CA_FLIPPER, 0, 0, 0);

	int pnum = 0;
	//Bounce the player
	if (twp->cwp->flag & CWK_FLAG_HIT)
	{
		auto player = CCL_IsHitPlayer(twp);
		pnum = player->counter.b[0];
		ActionFlipper(twp, angle - timer, pnum);
	}

	twp->timer.l = angle;
	BallSpeedDownM(tp, pnum);
	CalcCenter(tp);
	EntryColliList(twp);
	ObjectSetupInput(twp, 0);
}

void patch_flipper_init()
{
	ObjectFlipperLExec_h.Hook(NormalL);
	ObjectFlipperRExec_h.Hook(NormalR);
}

RegisterPatch patch_flipper(patch_flipper_init);
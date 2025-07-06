#include "pch.h"
#include "pinball.h"
#include <camerafunc.h>

//Tunnel that attract the players in pinball

#define JDAI_FLAG_BONUS_GIVEN 0x8000
#define GET_PNUM twp->id
DataPointer(pathtag, pathtag_slot1a, 0x1D800C4);
DataPointer(pathtag, pathtag_slot2a, 0x1D80A84);

DataPointer(pathtag, pathtag_slot2b, 0x1D8131C);
DataPointer(pathtag, pathtag_slot1b, 0x1D809C0);


FastFunctionHook<void, task*> Jdai_h(0x5DA3B0);
FastFunctionHook<void, task*> cJdai_h(0x5DA2C0);

static void cCameraReturnM(task* tp)
{
	taskwk* twp = tp->twp;
	auto pnum = GET_PNUM;
	if (!--tp->twp->btimer)
	{
		PinballCameraMode[pnum] = 0x80;
		if (twp->smode)
			PinballFlag |= PINBALL_FLAG_SLOT_STOPER_L;
		else
			PinballFlag |= PINBALL_FLAG_SLOT_STOPER_R;
		tp->ptp->twp->flag &= ~0x100;
		FreeTask(tp);
	}
}

void cJdai_r(task* tp)
{

	if (multiplayer::IsActive() == false)
	{
		return cJdai_h.Original(tp);
	}

	taskwk* ptwp = tp->ptp->twp;
	taskwk* twp = tp->twp;
	auto pnum = GET_PNUM;
	if (twp->cwp->flag & CWK_FLAG_HIT)
	{
		auto player = CCL_IsHitPlayer(twp);
		if (player)
		{
			GET_PNUM = player->counter.b[0];
			pnum = GET_PNUM;

			//Player got into one of the towers, award 20 rings if it hasn't already been given.
			if (!(twp->flag & JDAI_FLAG_BONUS_GIVEN))
			{
				//Sonic slot bonus is active
				if (PinballFlag & PINBALL_FLAG_SLOT_SONIC)
				{
					//Set the tower as entered.
					if (twp->smode)
						PinballFlag |= PINBALL_FLAG_SLOT_TOWER_R;
					else
						PinballFlag |= PINBALL_FLAG_SLOT_TOWER_L;

					/*  If both have been entered, give the player 150 Rings, move the camera to the nearest cannon and tell it to fire confetti.
						It doesn't actually fire until you get back to the main board because it needs the flag PL_FLAG_ONLAND.*/
					if ((PinballFlag & (PINBALL_FLAG_SLOT_TOWER_L | PINBALL_FLAG_SLOT_TOWER_R)) == (PINBALL_FLAG_SLOT_TOWER_L | PINBALL_FLAG_SLOT_TOWER_R))
					{
						PinballFlag &= ~(PINBALL_FLAG_SLOT_TOWER_L | PINBALL_FLAG_SLOT_TOWER_R);
						SetGetRingM((Sint8)150, pnum);
						PinballCameraMode[pnum] = (ptwp->smode == 0) - 121;
						taskwk* ctp = CreateChildTask(IM_TASKWK, cCameraReturnM, tp->ptp)->twp;
						ctp->btimer = 60;
						ctp->smode = ptwp->smode;
						ctp->id = pnum;
						PinballFlag |= (PINBALL_FLAG_SLOT_CANNON_L | PINBALL_FLAG_SLOT_CANNON_R); //Fire the confetti cannons
						ptwp->flag |= 0x100;
					}
				}
				SetGetRingM(20, pnum);
				twp->flag |= JDAI_FLAG_BONUS_GIVEN;

			}
		}
	}
	else
	{
		twp->flag &= ~JDAI_FLAG_BONUS_GIVEN;
	}
	EntryColliList(twp);
	ObjectSetupInput(twp, 0);
}

static bool CheckPlayerLimit(taskwk* twp)
{
	if (twp->cwp->flag & CWK_FLAG_HIT && twp->mode != 0)
	{
		auto player = CCL_IsHitPlayer(twp);
		if (player)
		{
			auto pnumCheck = player->counter.b[0];

			if (pnumCheck != GET_PNUM)
				return false;
			else
				return true;
		}
	}

	return false;
}

void Jdai_r(task* tp)
{

	if (multiplayer::IsActive() == false)
	{
		return Jdai_h.Original(tp);
	}

	taskwk* twp = tp->twp;
	auto pnum = GET_PNUM;
	pathinfo pinfo;
	NJS_POINT3 r;
	NJS_POINT3 s;
	NJS_POINT3* posPlayer = &playertwp[pnum]->pos;


	switch (twp->mode)
	{
	case 0: //Idle
		if (twp->cwp->flag & CWK_FLAG_HIT)
		{
			auto player = CCL_IsHitPlayer(twp);
			if (player)
			{
				GET_PNUM = player->counter.b[0];
				pnum = GET_PNUM;
				posPlayer = &player->pos;
				twp->scl.z = GetBallSpeedM(pnum);
				SetInputP(pnum, PL_OP_PLACEWITHSPIN);
				if (twp->smode)
				{
					twp->value.ptr = &pathtag_slot1a;
					twp->scl.x = 76.0f;  //totallen
				}
				else
				{
					twp->value.ptr = &pathtag_slot2a;
					twp->scl.x = 78.0f;  //totallen
				}
				SCPathPntNearToOnpos((pathtag*)twp->value.ptr, posPlayer, &s, &twp->scl.y);
				++twp->mode;
			}
		}
		break;
	case 1: //Player entering the tube
	{
		pinfo.onpathpos = twp->scl.y;
		GetStatusOnPath((pathtag*)twp->value.ptr, &pinfo);
		posPlayer->x = pinfo.xpos;
		posPlayer->y = pinfo.ypos;
		posPlayer->z = pinfo.zpos;
		twp->scl.z -= 0.05f;
		twp->scl.y += twp->scl.z;
		if (twp->scl.y < twp->scl.x)
		{
			if (twp->scl.y <= 0.0f)
			{
				twp->mode = 0;
				SetInputP(pnum, PL_OP_PINBALL);
				s = { -twp->scl.z, 0.0f, 0.0f };
				Angle3 ang = { 0, twp->smode != 0 ? 0xEEEF : 0x4B9, 0 };
				SetVelocityAndRotationAndNoconTimeP(pnum, &s, &ang, 0);
			}
		}
		else
		{
			PinballCameraMode[pnum] = (twp->smode == 0) - 0x7E;
			++twp->mode;
		}
		break;
	}
	case 2: //Holding the player in the tube
		twp->ang.x -= 0x100;
		if (twp->ang.x < -0x4000)
		{
			twp->ang.x = -0x4000;
			if (twp->smode)
			{
				twp->value.ptr = &pathtag_slot1b;
				twp->scl.x = 1285.0f; //totallen
				SCPathPntNearToOnpos(&pathtag_slot1b, posPlayer, &s, &twp->scl.y);
			}
			else
			{
				twp->value.ptr = &pathtag_slot2b;
				twp->scl.x = 1243.0f;  //totallen
				SCPathPntNearToOnpos(&pathtag_slot2b, posPlayer, &s, &twp->scl.y);
			}
			PinballCameraMode[pnum] &= ~0x80;
			++twp->mode;
		}
		break;
	case 3: //Player moving through the path in the tube.
		pinfo.onpathpos = twp->scl.y;
		GetStatusOnPath((pathtag*)twp->value.ptr, &pinfo);
		posPlayer->x = pinfo.xpos;
		posPlayer->y = pinfo.ypos - 4.0f;
		posPlayer->z = pinfo.zpos;
		pinfo.onpathpos = twp->scl.y - 10.0f;
		GetStatusOnPath((pathtag*)twp->value.ptr, &pinfo);

		NJS_POINT3 pos, tgt;
		tgt.x = pinfo.xpos;
		tgt.y = pinfo.ypos;
		tgt.z = pinfo.zpos;

		pinfo.onpathpos = twp->scl.y - 20.0f;
		GetStatusOnPath((pathtag*)twp->value.ptr, &pinfo);
		pos.x = pinfo.xpos;
		pos.y = pinfo.ypos;
		pos.z = pinfo.zpos;

		SetRuinWaka1Data(&pos, &tgt, pnum);
		twp->scl.y += 5.0f;
		if (twp->scl.y < twp->scl.x)
		{
			if (twp->ang.x)
			{
				twp->ang.x += 0x400;
				if (twp->ang.x > 0)
				{
					twp->ang.x = 0;
				}
			}
		}
		else
		{
			SetInputP(pnum, PL_OP_PINBALL);
			r = { 1.0f, 0.0f, 0.0f };
			s = { 0.0f, 0.0f, 0.0f };
			SetVelocityAndRotationAndNoconTimeP(pnum, &r, (Angle3*)&s, 0);
			twp->counter.l = 0x8000;
			twp->timer.l = 0x41A00000;
			++twp->mode;
		}
		break;
	case 4: //Side camera after player exits the tube.
		twp->timer.l = 0x42800000;
		if (twp->smode)
			twp->counter.l = 0x4000;
		else
			twp->counter.l = 0xC000;
		if (!(twp->flag & 0x100))
		{

			NJS_POINT3 pos, tgt;
			pos.x = njSin(twp->counter.l) * twp->timer.f + posPlayer->x;
			pos.y = posPlayer->y;
			pos.z = njCos(twp->counter.l) * twp->timer.f + posPlayer->z;
			tgt = *posPlayer;

			SetRuinWaka1Data(&pos, &tgt, pnum);

			if (posPlayer->y < 35.0f)
			{
				twp->mode = 0;
				PinballCameraMode[pnum] = 0x80;
				if (twp->smode)
					PinballFlag |= PINBALL_FLAG_SLOT_STOPER_L;
				else
					PinballFlag |= PINBALL_FLAG_SLOT_STOPER_R;
			}
		}
		LoopTaskC(tp);
		break;
	default:
		break;
	}

	EntryColliList(twp);
	ObjectSetupInput(twp, 0);
	tp->disp(tp);
}


void patch_jdai_init()
{
	Jdai_h.Hook(Jdai_r);
	cJdai_h.Hook(cJdai_r);
}

RegisterPatch patch_jdai(patch_jdai_init);

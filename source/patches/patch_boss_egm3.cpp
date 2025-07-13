#include "pch.h"

typedef struct EGM3_SPRITE
{
	Sint32 nbPatterns;
	NJS_SPRITE* spr;
	Sint32 attr;
}EGM3_SPRITE;

typedef struct egm3wk
{
	NJS_POINT3 scl;
	NJS_POINT3 spd;
	NJS_POINT3 acc;
	NJS_POINT3 dst;
	Angle3 dstang;
	Angle3 ang;
	Angle3 ang_spd;
	Sint32 area;
	Float t;
	Sint32 kmode;
	Sint32 ksmode;
	Sint32 comu;
	Sint32 flag;
	NJS_MOTION* mtn;
	NJS_MOTION* shp;
	EGM3_SPRITE* spr;
	Float frame;
	Float fspd;
}egm3wk;

enum {
	egm3mtn_backopen,
	egm3mtn_backopenS,
	egm3mtn_frontopen,
	egm3mtn_hadou,
	egm3mtn_hadouS,
	egm3mtn_cockpit,
	egm3mtn_max
};

FunctionPointer(int, UkisimaExist, (float x, float z), 0x5846F0);
DataPointer(task*, egm3_taskptr, 0x3C6CF60);
FastcallFunctionPointer(int, StopMotion, (egm3wk* wk), 0x57EDB0);
DataArray(NJS_MOTION*, egm3_motion_list, 0x167DC80, 0x6);
DataPointer(int, egm3_energy_timer, 0x3C6E0D8);
FunctionPointer(int, Egm3TurnToward, (unsigned __int16 src, unsigned __int16 dst, int spd), 0x57EAB0);
DataPointer(int, egm3_camflag_forboss, 0x3C6E0DC);
DataArray(Float, laser1_tobidasi_x, 0x165EE28, 0x2);
DataArray(NJS_POINT3, boom_hasi1, 0x165EFDC, 0x4);
DataArray(Float[64], egm3_matrix_buffer, 0x3C6D028, 16);

TaskHook egm3_laser1_h(0x583360);

static void egm3_laser1(task* tp)
{

	if (multiplayer::IsActive() == false)
	{
		return egm3_laser1_h.Original(tp);
	}
	taskwk* twp = tp->twp;
	egm3wk* wk = (egm3wk*)tp->awp;
	Float newDstZ = 0.0f;

	switch (twp->mode)
	{
	case 0:
	{
		tp->dest = (TaskFuncPtr)nullsub;
		tp->disp = (TaskFuncPtr)0x57DE50;
		wk->acc = twp->pos;
		auto pnum = GetTheNearestPlayerNumber(&twp->pos);

		if (!twp->btimer || twp->btimer == 1)
		{

			wk->dst.x = ((playermwp[pnum]->spd.x * 20.0f) + playertwp[pnum]->pos.x);
			wk->dst.y = 15.0f;
			wk->dst.z = ((playermwp[pnum]->spd.z * 20.0f) + playertwp[pnum]->pos.z);

			//if platforms exist
			if (UkisimaExist(wk->dst.x, wk->dst.z))
			{
				wk->dst.x = (playermwp[pnum]->spd.x * 10.0f) + playertwp[pnum]->pos.x;
				wk->dst.z = (playermwp[pnum]->spd.z * 10.0f) + playertwp[pnum]->pos.z;
			}
			else
			{
				wk->dst.x = playertwp[pnum]->pos.x;
				wk->dst.z = playertwp[pnum]->pos.z;
			}
		}
		else
		{
			wk->dst.x = playertwp[pnum]->pos.x;
			wk->dst.y = 15.0f;
			wk->dst.z = playertwp[pnum]->pos.z;
		}

		if ((twp->pos.z - playertwp[pnum]->pos.z) > 300.0f)
		{
			wk->dst.z = (twp->pos.z - 300.0f);
		}
		else if ((twp->pos.z - playertwp[pnum]->pos.z) < -300.0f)
		{
			wk->dst.z = (twp->pos.z + 300.0f);
		}

		//if platforms don't exist, give up firing laser
		if (!UkisimaExist(wk->dst.x, wk->dst.z))
		{
			twp->mode = 100;
		}
		else
		{
			wk->spd.x = (wk->dst.x - twp->pos.x) * 0.06666667f;
			wk->spd.y = (wk->dst.y - twp->pos.y) * 0.06666667f;
			wk->spd.z = (wk->dst.z - twp->pos.z) * 0.06666667f;

			Float xOffset = (wk->dst.x - twp->pos.x);
			Float zOffset = (wk->dst.z - twp->pos.z);
			Float yOffset = (wk->dst.y - twp->pos.y);

			twp->ang.y = njArcTan2(xOffset, zOffset);
			Float res = njSqrt((zOffset * zOffset) + (xOffset * xOffset));
			twp->ang.x = njArcTan2(-yOffset, res);
			wk->t = 0.0f;

			twp->wtimer = 0;
			if (!twp->btimer || twp->btimer == 2)
			{
				setlaser1pon(&twp->pos, &twp->ang);
			}
			dsPlay_oneshot_Dolby(489, 0, 0, 0, 120, twp);
			++twp->mode;
		}
	}
	}

	egm3_laser1_h.Original(tp);
	tp->disp(tp);
}

TaskHook Egm3Energy_h(0x582BC0);
//start to deploy tail and become vulnerable, throw energy ball if you are too slow to hit him
void Egm3Energy(task* tp)
{
	taskwk* twp = tp->twp;
	egm3wk* wk = (egm3wk*)tp->awp;
	auto pnum = twp->counter.b[0];

	switch (twp->smode)
	{
	case 0:
	{
		twp->counter.b[0] = GetTheNearestPlayerNumber(&twp->pos);
		pnum = twp->counter.b[0];
		wk->mtn = egm3_motion_list[egm3mtn_backopen];
		wk->shp = egm3_motion_list[egm3mtn_backopenS];
		wk->frame = 0.0f;
		wk->fspd = 1.0f;
		wk->area = playertwp[pnum]->pos.x >= 0.0f;
		++egm3_energy_timer;
		twp->smode = 1;
	}
	case 1:
		if (wk->area)
		{
			wk->dst.x = 110.0f;
		}
		else
		{
			wk->dst.x = -110.0f;
		}
		wk->dst.y = -200.0f;
		wk->dst.z = playertwp[pnum]->pos.z;
		twp->smode = 2;
	case 2:
		twp->pos = wk->dst;
		twp->smode = 3;
		break;
	}

	if (twp->smode > 2)
	{
		Egm3Energy_h.Original(tp);
	}
}

TaskHook Egm3Laser1_h(0x57F430);

void Egm3Laser1(task* tp)
{


	taskwk* twp = tp->twp;
	egm3wk* wk = (egm3wk*)tp->awp;
	auto pnum = twp->counter.b[0];
	taskwk* p = playertwp[pnum];

	switch (twp->smode)
	{
	case 0:
		twp->counter.b[0] = GetTheNearestPlayerNumber(&twp->pos);
		p = playertwp[twp->counter.b[0]];
		wk->shp = 0;
		wk->mtn = 0;
		twp->wtimer = 0;
		twp->smode = 1;
	case 1:

		if (!egm3_camflag_forboss)
		{
			return;
		}
		egm3_camflag_forboss = FALSE;

		//get which side laser should go
		wk->area = (p->pos.x >= 0.0f);
		wk->dst.x = laser1_tobidasi_x[wk->area];
		wk->dst.y = -150.0f;
		if (p->pos.z <= 0.0f)
		{
			wk->dst.z = -550.0f;
		}
		else
		{
			wk->dst.z = 550.0f;
		}
		twp->smode = 2;
	case 2:
		twp->pos = wk->dst;
		twp->smode = 3;
		return;
	case 3:
		if (twp->pos.z <= 0.0f)
		{
			wk->spd.z = 16.0f;
		}
		else
		{
			wk->spd.z = -16.0f;
		}

		twp->smode = 4;
	case 4:
		twp->pos.z += wk->spd.z;
		twp->ang.x = Egm3TurnToward(twp->ang.x, 0, 546);
		twp->ang.y = Egm3TurnToward(twp->ang.y, (njArcTan2(0.0f, wk->spd.z)), 546);

		if (wk->spd.z <= 0.0f)
		{
			if (twp->pos.z > p->pos.z && p->pos.z + 200.0f >= twp->pos.z)
			{
				twp->smode = 5;
			}
		}
		else if (twp->pos.z < p->pos.z && p->pos.z - 200.0f <= twp->pos.z)
		{
			twp->smode = 5;
		}

		if (wk->spd.z > 0.0f && twp->pos.z >= 550.0f || wk->spd.z < 0.0f && twp->pos.z <= -550.0f)
		{
			twp->smode = 11;
		}
		return;
	}

	if (twp->smode > 4)
	{
		Egm3Laser1_h.Original(tp);
	}
}

TaskHook Egm3BoomerangPre_h(0x57F940);
//set up viper in one of the corner of the room to throw the wheels
void Egm3BoomerangPre(task* tp)
{
	taskwk* twp = tp->twp;
	egm3wk* wk = (egm3wk*)tp->awp;

	switch (twp->smode)
	{
	case 3:
	{
		Sint32 flag = 0;
		auto pnum = GetTheNearestPlayerNumber(&twp->pos);
		if (playertwp[pnum]->pos.x >= 0.0f)
		{
			flag = 2;
		}

		wk->area = flag | (playertwp[pnum]->pos.z <= 0.0f);

		wk->dst = boom_hasi1[wk->area];
		twp->smode = 4;
	}
	}

	Egm3BoomerangPre_h.Original(tp);
}

TaskHook EggmanBoomerang_h(0x580E70);
static taskwk* eggmanWheelPtr = nullptr;
//Show Eggman (human) in the cockpit during the wheel section
void EggmanBoomerang(task* tp)
{
	eggmanWheelPtr = tp->twp;
	EggmanBoomerang_h.Original(tp);	
}

TaskHook Egm3Sippo_h(0x583FB0);
static taskwk* sippoPtr = nullptr;
void __cdecl Egm3Sippo(task* tp)
{
	sippoPtr = tp->twp;
	Egm3Sippo_h.Original(tp);
}


static void SetVelocityPSippo_r(uint8_t pnum, Float x, Float y, Float z)
{
	if (sippoPtr)
	{
		auto p = CCL_IsHitPlayer(sippoPtr);
		if (p)
		{
			return SetVelocityP(p->counter.b[0], x, y, z);
		}
	}

	SetVelocityP(pnum, x, y, z);
}


TaskHook EggmanEnergy_h(0x580A90);
static taskwk* eggmanPtr = nullptr;
void __cdecl EggmanEnergy(task* tp)
{
	eggmanPtr = tp->twp;
	EggmanEnergy_h.Original(tp);
}


static void SetVelocityPEggman_r(uint8_t pnum, Float x, Float y, Float z)
{
	if (eggmanPtr)
	{
		auto p = CCL_IsHitPlayer(eggmanPtr);
		if (p)
		{
			return SetVelocityP(p->counter.b[0], x, y, z);
		}
	}

	SetVelocityP(pnum, x, y, z);
}

static void SetVelocityPEggmanWheel_r(uint8_t pnum, Float x, Float y, Float z)
{
	if (eggmanWheelPtr)
	{
		auto p = CCL_IsHitPlayer(eggmanWheelPtr);
		if (p)
		{
			return SetVelocityP(p->counter.b[0], x, y, z);
		}
	}

	SetVelocityP(pnum, x, y, z);
}



void patch_egm3_init()
{
	egm3_laser1_h.Hook(egm3_laser1);
	Egm3Laser1_h.Hook(Egm3Laser1); //not the same thing than egm3_laser1. (Egm3Laser1 != egm3_laser1)
	EggmanBoomerang_h.Hook(EggmanBoomerang);


	Egm3Sippo_h.Hook(Egm3Sippo);
	EggmanEnergy_h.Hook(EggmanEnergy);
	Egm3Energy_h.Hook(Egm3Energy);
	Egm3BoomerangPre_h.Hook(Egm3BoomerangPre);

	//sippo (tail)
	WriteCall((void*)0x584430, SetVelocityPSippo_r); //I'm lazy

	//eggman energy
	WriteCall((void*)0x580C7E, SetVelocityPEggman_r);
	WriteCall((void*)0x580D2C, SetVelocityPEggman_r);
	WriteCall((void*)0x580BED, SetVelocityPEggman_r);

	//eggman wheel version
	WriteCall((void*)0x580F27, SetVelocityPEggmanWheel_r);
}

#ifdef MULTI_TEST
RegisterPatch patch_egm3(patch_egm3_init);
#endif
#include "pch.h"
#include "patch_sm_tornade.h"

TaskHook SonicMilesTornade_h(0x627EC0);
TaskHook TornadeDisplay_h(0x627430);
TaskHook TornadoMove_h(0x627A30);
FastUsercallHookPtr<void(*)(task* tp), noret, rEAX> AnimeCountUp_h(0x627580);

VariableHook <tornadewk, 0x3C822A8> tornade_m;
//VariableHook <int, 0x3C822F4> TornadeMode_m;
VariableHook <Float, 0x3C8231C> AnmTimer_m;
VariableHook<Float, 0x3C82310> ShtMOVE_SPEED_m;
VariableHook<Float, 0x3C8230C> ShtMAX_POSX_m;
VariableHook<Float, 0x3C82314>  ShtMAX_POSY_m;
VariableHook<CamBackWk, 0x3C82390> camBackwk_m;

DataPointer(int, TornadeMode, 0x3C822F4);

DataArray(plmodel[3], PL_Model, 0x27D6E38, 0x4);
DataPointer(NJS_POINT3*, PlayerPath, 0x3C822A0);
DataPointer(int, FlyingPoint, 0x3C82300);
DataPointer(int, FlyingMode, 0x3C822F8);
StdcallFunctionPointer(double, njRoundOff, (float a1), 0x789320);




void TornadoMove(task* tp)
{
	
	if (multiplayer::IsActive() == false)
	{
		return TornadoMove_h.Original(tp);
	}

	taskwk* twp = tp->twp;
	auto pnum = twp->counter.b[0];

	twp->scl.x = PlayerPath[FlyingPoint].x - tornade_m[pnum].pathx;
	twp->scl.y = PlayerPath[FlyingPoint].y - tornade_m[pnum].pathy;
	twp->scl.z = PlayerPath[FlyingPoint].z - tornade_m[pnum].pathz;
	NJS_POINT3* curPlayerPath = &PlayerPath[FlyingPoint];
	NJS_POINT3 ps, newPos;
	tornade_m[pnum].pathx = curPlayerPath->x;
	tornade_m[pnum].pathy = curPlayerPath->y;
	tornade_m[pnum].pathz = curPlayerPath->z;
	DirectionMatrix(FlyingPoint, twp, 0);
	ps.x = 0.0f;
	ps.y = 0.0f;
	ps.z = 0.0f;
	njPushMatrix(0);
	njUnitMatrix(0);
	njTranslate(0, tornade_m[pnum].pathx, tornade_m[pnum].pathy, tornade_m[pnum].pathz);
	njRotateXYZ(0, twp->ang.x, twp->ang.y, twp->ang.z);
	njTranslate(0, tornade_m[pnum].ofsx, tornade_m[pnum].ofsy, tornade_m[pnum].ofsz);
	njCalcPoint(0, &ps, &newPos);
	njPopMatrix(1u);
	twp->pos = newPos;

	if (tornade_m[pnum].rotx > 0)
	{
		tornade_m[pnum].rotx -= 0x20;
		tornade_m[pnum].rotx = NJM_MAX(0, tornade_m[pnum].rotx);
	}
	else if (tornade_m[pnum].rotx < 0)
	{
		tornade_m[pnum].rotx += 0x20;
		tornade_m[pnum].rotx = NJM_MIN(0, tornade_m[pnum].rotx);
	}

	if (tornade_m[pnum].rotz > 0)
	{
		tornade_m[pnum].rotz -= 0x40;
		tornade_m[pnum].rotz = NJM_MAX(0, tornade_m[pnum].rotz);
	}
	else if (tornade_m[pnum].rotz < 0)
	{
		tornade_m[pnum].rotz += 0x40;
		tornade_m[pnum].rotz = NJM_MIN(0, tornade_m[pnum].rotz);
	}

	if (tornade_m[pnum].roty > 0)
	{
		tornade_m[pnum].roty -= 0x20;
		tornade_m[pnum].roty = NJM_MAX(0, tornade_m[pnum].roty);

	}
	else if (tornade_m[pnum].roty < 0)
	{
		tornade_m[pnum].roty += 0x20;
		tornade_m[pnum].roty = NJM_MIN(0, tornade_m[pnum].roty);
	}

	//X Pos
	tornade_m[pnum].ofsx = tornade_m[pnum].ofsx - (Float)(perG[pnum].x1 << 8) * ShtMOVE_SPEED_m[pnum] * 0.000030517578f;
	if (FlyingMode)
	{
		tornade_m[pnum].ofsx = NJM_MIN(ShtMAX_POSX_m[pnum], tornade_m[pnum].ofsx);
		tornade_m[pnum].ofsx = NJM_MAX(-ShtMAX_POSX_m[pnum], tornade_m[pnum].ofsx);
	}

	//Y Pos
	tornade_m[pnum].ofsy = tornade_m[pnum].ofsy - (Float)(perG[pnum].y1 << 8) * ShtMOVE_SPEED_m[pnum] * 0.000030517578f;
	if (FlyingMode)
	{
		tornade_m[pnum].ofsy = NJM_MIN(ShtMAX_POSY_m[pnum], tornade_m[pnum].ofsy);
		tornade_m[pnum].ofsy = NJM_MAX(-ShtMAX_POSY_m[pnum], tornade_m[pnum].ofsy);
	}

	//Angle of plane while moving
	Float magn = perG[pnum].x1 << 8 <= 0;
	if ((perG[pnum].x1 & 0x800000))
	{
		tornade_m[pnum].rotz -= 0xC0;
		tornade_m[pnum].rotz = NJM_MAX(-0x1800, tornade_m[pnum].rotz);

		magn = perG[0].x1 << 8 <= 0;
	}
	if (!magn)
	{
		tornade_m[pnum].rotz += 0xC0;
		tornade_m[pnum].rotz = NJM_MIN(0x1800, tornade_m[pnum].rotz);
	}

	if (1)
	{
		Float camAngZ = tornade_m[pnum].ofsx * tornade_m[pnum].ofsx / (ShtMAX_POSX_m[pnum] * ShtMAX_POSX_m[pnum]) * 0.5f * tornade_m[pnum].rotz;
		camBackwk_m[0].ang.z = njRoundOff(camAngZ);
	}

	if (playertwp[pnum]->smode)
	{
		//CreateChildTask(IM_TASKWK, EffectKumo, tp);
		playertwp[pnum]->smode = 0;
	}

	tornade_m[pnum].speed = njScalor(&twp->scl);
}

void TornadeDisplayHack(task* tp)
{
	tp->disp(tp);
}

void TornadeDisplay_r(task* tp)
{

	if (multiplayer::IsActive() == false)
	{
		return TornadeDisplay_h.Original(tp);
	}

	taskwk* twp = tp->twp;
	Sint32 index;
	auto pnum = twp->counter.b[0];

	if (!loop_count)
	{
		switch (twp->mode)
		{
		case MD_TORNADE_DAMG:
			index = 1;
			break;
		case MD_TORNADE_DEAD:
			index = 2;
			break;
		case MD_TORNADE_DEAD2:
			index = 3;
			break;
		default:
			index = 0;
			break;
		}
		___njFogDisable();
		njPushMatrix(0);
		njTranslate(0, tornade_m[pnum].pathx, tornade_m[pnum].pathy, tornade_m[pnum].pathz);
		njRotateXYZ(0, twp->ang.x, twp->ang.y, twp->ang.z);
		njPushMatrix(0);
		njTranslate(0, tornade_m[pnum].ofsx, tornade_m[pnum].ofsy, tornade_m[pnum].ofsz);
		njRotateXYZ(0, tornade_m[pnum].rotx, tornade_m[pnum].roty, tornade_m[pnum].rotz);
		SaveConstantAttr();
		OnConstantAttr(0, NJD_FLAG_USE_ALPHA);
		SetMaterial(tornade_m[pnum].Alpha, 0.0f, 0.0f, 0.0f);
		NJS_TEXLIST* tex = PL_Model[TornadeMode][index].tex;
		njSetTexture(tex);
		//___dsSetPalette(2);
		___dsSetPalette(0); //lantern engine support
		late_ActionMesh(PL_Model[TornadeMode][index].act, AnmTimer_m[pnum], LATE_LIG);
		___dsSetPalette(0);
		ResetMaterial();
		LoadConstantAttr();
		njPopMatrix(1);
		njPopMatrix(1);
	}
}

void AnimeCountUp(task* tp)
{
	if (multiplayer::IsActive() == false)
	{
		return AnimeCountUp_h.Original(tp);
	}


	auto twp = tp->twp;
	auto pnum = twp->counter.b[0];

	Sint32 index = 0;
	switch (tp->twp->mode)
	{
	case MD_TORNADE_DAMG:
		index = 1;
		break;
	case MD_TORNADE_DEAD:
		index = 2;
		break;
	case MD_TORNADE_DEAD2:
		index = 3;
		break;
	}


	NJS_MOTION* mtn = PL_Model[TornadeMode][index].act->motion;

	AnmTimer_m[pnum]++;

	if (mtn->nbFrame < 0)
	{
		mtn->nbFrame = (Float)0x82000000; //what
	}

	if (AnmTimer_m[pnum] >= (Float)mtn->nbFrame)
	{
		AnmTimer_m[pnum] = 0.0f;
	}
}

void __cdecl SonicMilesTornade(task* tp)
{
	auto twp = tp->twp;
	auto pnum = twp->counter.b[0];

	switch (twp->mode)
	{
	case 0:
		memset(&tornade_m[pnum], 0, sizeof(tornadewk));
		AnmTimer_m[pnum] = 0.0f;
		break;
	case 5:
		if (!twp->wtimer)
		{
			KillHimP(pnum);
		}
		break;
	}

	SonicMilesTornade_h.Original(tp);
}

void patch_shooting_init()
{
	WriteCall((void*)0x628084, TornadeDisplayHack);
	WriteCall((void*)0x6280C8, TornadeDisplayHack);
	WriteCall((void*)0x62812F, TornadeDisplayHack);

	WriteData<5>((int*)0x628150, 0x90); //remove kill player we will manually set it

	SonicMilesTornade_h.Hook(SonicMilesTornade);
	TornadeDisplay_h.Hook(TornadeDisplay_r);
	AnimeCountUp_h.Hook(AnimeCountUp);
	TornadoMove_h.Hook(TornadoMove);
}

RegisterPatch patch_shooting(patch_shooting_init);
#include "pch.h"
#include "patch_sm_tornade.h"
#include <splitscreen.h>

TaskHook SonicMilesTornade_h(0x627EC0);
TaskHook TornadeDisplay_h(0x627430);
TaskHook TornadoMove_h(0x627A30);
TaskHook RapidShotCheck_h(0x627830);

FastUsercallHookPtr<void(*)(task* tp), noret, rEAX> AnimeCountUp_h(0x627580);
FastUsercallHookPtr<void(*)(taskwk* twp), noret, rESI> Tornado_chk_mode_h(0x627930);
FastUsercallHookPtr<void(*)(task* tp), noret, rEDI> HomingShotCheck_h(0x627770);

FastFunctionHook<void> SkyChaseInit_h(0x6281A0);

VariableHook <tornadewk, 0x3C822A8> tornade_m;
//VariableHook <int, 0x3C822F4> TornadeMode_m;
VariableHook <Float, 0x3C8231C> AnmTimer_m;
VariableHook<Float, 0x3C82310> ShtMOVE_SPEED_m;
VariableHook<Float, 0x3C8230C> ShtMAX_POSX_m;
VariableHook<Float, 0x3C82314>  ShtMAX_POSY_m;
VariableHook<CamBackWk, 0x3C82390> camBackwk_m;
VariableHook<Float, 0x3C82304> TornadeHP_m;
VariableHook<task*, 0x3C82348> lockon_list_m;

DataArray(task*, lockon_list, 0x3C82348, 0x6);

static const double SkyRotationMultiplier = -0.5;


DataPointer(int, TornadeMode, 0x3C822F4);

DataArray(plmodel[3], PL_Model, 0x27D6E38, 0x4);
DataPointer(NJS_POINT3*, PlayerPath, 0x3C822A0);
DataPointer(int, FlyingPoint, 0x3C82300);
DataPointer(int, FlyingMode, 0x3C822F8);
StdcallFunctionPointer(double, njRoundOff, (float a1), 0x789320);
DataPointer(int, ShtInvincible, 0x3C82318);
TaskFunc(Tama1, 0x62C820);
DataPointer(int, SeFlag, 0x3C82320);

#define TamaWK(tp) (tp->awp)

static Bool TornadoDead[PLAYER_MAX];

Bool CheckLockMode(int pnum)
{
	if (((AttackButtons | JumpButtons) & perG[pnum].on) != 0 && tornade_m[pnum].LockTame++ > 30)
	{
		return TRUE;
	}

	return FALSE;
}

void RapidShotCheck(task* tp)
{
	if (multiplayer::IsActive() == false)
	{
		return RapidShotCheck_h.Original(tp);
	}

	auto twp = tp->twp;
	auto pnum = twp->counter.b[0];

	if ((perG[pnum].press & (JumpButtons | AttackButtons)))
	{
		tornade_m[pnum].RapidFire = 5;
	}

	if (!tornade_m[pnum].RapidCnt && tornade_m[pnum].RapidFire > 0)
	{
		task* tamaTP = CreateChildTask(IM_TASKWK, Tama1, tp);
		if (tamaTP)
		{
			taskwk* tamaTWP = tamaTP->twp;

			tamaTWP->counter.b[1] = 0xFF;
			--tornade_m[pnum].RapidFire;
			if (!SeFlag)
			{
				dsPlay_oneshot(SE_SHT_VULCAN, 0, 0, 0);
			}
			SeFlag = ((Sint8)SeFlag - 1) & 1;
		}
	}
	tornade_m[pnum].RapidCnt = ((Sint8)(tornade_m[pnum].RapidCnt) + 1) & 3; //what
}

void HomingShotCheck(task* tp)
{
	if (multiplayer::IsActive() == false)
	{
		return HomingShotCheck_h.Original(tp);
	}

	auto twp = tp->twp;
	auto pnum = twp->counter.b[0];
	//If the button for locking on has been released
	if (!(JumpButtons & perG[pnum].on)
		&& !(AttackButtons & perG[pnum].on)
		&& (perG[pnum].release & (JumpButtons | AttackButtons)))
	{
		if (tornade_m[pnum].LockTame++ > 30)
		{
			//Check the lockon list and fire at the targets
			Sint32 count = 0;
			for (Sint32 i = 0; i < lockon_list.size(); ++i)
			{
				if (lockon_list[i])
				{
					task* TamaTP = CreateChildTask(IM_TASKWK, Tama1, tp);
					if (TamaTP)
					{
						TamaWK(TamaTP) = (anywk*)lockon_list[i];
						TamaTP->twp->counter.b[1] = i;
						++count;
					}
				}
			}
			if (count)
			{
				dsPlay_oneshot_Dolby(SE_SHT_MISSILE, 0, 0, 0, 120, tp->twp);
			}
			VibShot(pnum, 0);
		}
		tornade_m[pnum].LockTame = 0;
	}
}

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

		magn = perG[pnum].x1 << 8 <= 0;
	}
	if (!magn)
	{
		tornade_m[pnum].rotz += 0xC0;
		tornade_m[pnum].rotz = NJM_MIN(0x1800, tornade_m[pnum].rotz);
	}

	if (!pnum)
	{
		Float camAngZ = tornade_m[pnum].ofsx * tornade_m[pnum].ofsx / (ShtMAX_POSX_m[pnum] * ShtMAX_POSX_m[pnum]) * SkyRotationMultiplier * tornade_m[pnum].rotz;
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

	if (TornadoDead[pnum])
	{
		return;
	}

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

static void Tornado_chk_mode(taskwk* twp)
{
	if (multiplayer::IsActive() == false)
	{
		return Tornado_chk_mode_h.Original(twp);
	}

	auto pnum = twp->counter.b[0];
	if (!twp->wtimer)
	{
		//Damage
		if (twp->flag & PL_FLAG_DAMAGE)
		{
			twp->flag &= ~PL_FLAG_DAMAGE;

			if (TornadoDead[pnum] == FALSE)
			{
				twp->wtimer = 30;
				CreateBomb(&twp->pos, 2.0f);
				if (!ShtInvincible)
				{
					TornadeHP_m[pnum] -= 0.5f;
					VibConvergence(pnum, 7, 59, 6);
					if (TornadeHP_m[pnum] > 0.0f)
					{
						twp->mode = MD_TORNADE_DAMG;
						dsPlay_oneshot(SE_SHT_KAGU, 0, 0, 0);
					}
					else //Death
					{
						twp->scl.y = 0.0f;
						twp->mode = MD_TORNADE_DEAD;
						AnmTimer_m[pnum] = 0.0f;
					}
				}
			}
		}
		else
		{
			twp->flag &= ~PL_FLAG_INVINCIBLE;
		}
	}

	Tornado_chk_mode_h.Original(twp);
}

void __cdecl SonicMilesTornade(task* tp)
{
	auto twp = tp->twp;
	auto pnum = twp->counter.b[0];

	switch (twp->mode)
	{
	case 0:
	{
		memset(&tornade_m[pnum], 0, sizeof(tornadewk));
		memset(&TornadoDead[pnum], FALSE, sizeof(Bool));
		AnmTimer_m[pnum] = 0.0f;

		auto ratio = splitscreen::GetScreenRatio(pnum);
		float scaleX = HorizontalStretch * ratio->w;
		Float scaleY = VerticalStretch * ratio->h;

		ShtMOVE_SPEED_m[pnum] = 1.0f;
		ShtMAX_POSX_m[pnum] = 1.0f;
		ShtMAX_POSY_m[pnum] = 1.0f;
	}
		break;
	case MD_TORNADE_DEAD:
		tp->disp(tp);
		//TornadoDeadFall(tp);
		AnimeCountUp(tp);
		if (AnmTimer_m[pnum] == 0.0f)
		{
			TornadoDead[pnum] = TRUE;
			twp->mode = 60; //dead
		}
		break;
	case 5:
		TornadoDead[pnum] = TRUE;
		twp->mode = 60; //dead
		break;
	case 60:
		
		if (!pnum)
		{
			int count = 0;
			int pMax = multiplayer::GetPlayerCount();
			for (int i = 0; i < pMax; i++)
			{
				if (TornadoDead[i])
				{
					count++;
				}
			}

			if (count >= pMax)
			{
				for (int i = 0; i < pMax; i++)
				{
					KillHimP(i);
				}

				twp->mode++;
			}
		}
		break;
	}


	if (twp->mode != MD_TORNADE_DEAD)
		SonicMilesTornade_h.Original(tp);
}

void __cdecl SkyChaseInit_r()
{
	SkyChaseInit_h.Original();
	for (uint8_t i = 1; i < PLAYER_MAX; i++)
	{
		TornadeHP_m[i] = 5.0f;
	}
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
	Tornado_chk_mode_h.Hook(Tornado_chk_mode);
	SkyChaseInit_h.Hook(SkyChaseInit_r);
	HomingShotCheck_h.Hook(HomingShotCheck);
	RapidShotCheck_h.Hook(RapidShotCheck);
}

RegisterPatch patch_shooting(patch_shooting_init);
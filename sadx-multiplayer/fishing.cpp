#include "pch.h"
#include "multiplayer.h"
#include "hud_fishing.h"
#include "fishing.h"

/*

Fishing system patch
The game uses several global variables and player 1 pointers throughout the entire system
I packed all global variables into a struct and rewrote most of the system for multiplayer.

*/

enum : __int32
{
	MODE_LURE_INIT = 0x0,
	MODE_LURE_SETUP = 0x1,
	MODE_LURE_WAIT = 0x2,
	MODE_LURE_NORMAL = 0x3,
	MODE_LURE_FISHING_INIT = 0x4,
	MODE_LURE_FISHING = 0x5,
	MODE_LURE_HIT = 0x6,
	MODE_LURE_RETURN = 0x7,
	MODE_LURE_RETURN_GET = 0x8,
	MODE_LURE_END = 0x9,
};

struct lurewk // custom; todo: find symbol struct
{
	void* unk0;
	void* unk1;
	String* string;
	NJS_POINT3* pos;
};

MAKEVARMULTI(BIGETC, bigetc, 0x3C524E8);

FunctionPointer(BOOL, checkturipoint2, (), 0x48D030);
FunctionPointer(int, sub_46EE90, (), 0x46EE90); // inline, get fished item
DataPointer(CCL_INFO, lure_colli_tbl, 0x91BA7C);
DataArray(NJS_POINT3, icecap_fpoint_tbl, 0x91BA44, 4);

static auto sub_46D0D0 = GenerateUsercallWrapper<BOOL (*)(task* tp)>(rEAX, 0x46D0D0, rEAX); // inline, checks if lure touches ground
static auto sub_46D970 = GenerateUsercallWrapper<BOOL (*)(Big_ypos* big_y_ptr, task* tp)>(rEAX, 0x46D970, rEAX, stack4); // inline, checks if lure collided with water
static auto calcFishingLureY = GenerateUsercallWrapper<void (*)(taskwk* twp, Big_ypos* big_y_ptr, char flag)>(noret, 0x46DD20, rEAX, rESI, stack4);
static auto moveFishingRotY = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x46E380, rEAX);
static auto chkKabeAngle2 = GenerateUsercallWrapper<BOOL (*)(Angle3* angle3_p)>(noret, 0x46C6A0, rESI);

Trampoline* dispFishingLure_t       = nullptr;
Trampoline* dispFishingLureSwitch_t = nullptr;
Trampoline* fishingLureCtrl_t       = nullptr;
Trampoline* SetFishingLureTask_t    = nullptr;
Trampoline* fishingCursorCtrl_t     = nullptr;
Trampoline* SetFishingCursorTask_t  = nullptr;

BIGETC* GetBigEtc(int pnum)
{
	return pnum >= 0 && pnum <= 3 ? bigetc_m[pnum] : nullptr;
}

#pragma region utilities
float GetWaterLevel_m(BIGETC* etc)
{
	return etc->water_level;
}

float GetReelLength_m(BIGETC* etc)
{
	return etc->reel_length;
}

void CalcHookPos_m(BIGETC* etc, NJS_POINT3* ret)
{
	if (etc->Big_Lure_Ptr)
	{
		auto twp = etc->Big_Lure_Ptr->twp;
		NJS_POINT3 v = { 0.6f, -0.45f, 0.0f };

		njPushMatrix(_nj_unit_matrix_);
		njRotateZ_(twp->ang.z);
		njRotateX_(twp->ang.x);
		njRotateY_(-HIWORD(twp->ang.y));
		njCalcVector(0, &v, ret);
		njPopMatrixEx();

		ret->x += twp->pos.x;
		ret->y += twp->pos.y;
		ret->z += twp->pos.z;
	}
	else
	{
		ret->z = 0.0f;
		ret->y = 0.0f;
		ret->x = 0.0f;
	}
}

bool ChkFishingThrowNow_m(int pnum)
{
	auto mode = playertwp[pnum]->mode;
	return (mode < 32 || mode > 37) && (mode < 40 || mode > 45);
}
#pragma endregion

#pragma region dispFishingLure
static void __cdecl dispFishingLure_m(task* tp)
{
	auto twp = tp->twp;
	auto pnum = twp->btimer;
	auto ptwp = playertwp[pnum];

	if (!MissedFrames && ptwp && TASKWK_CHARID(ptwp) == Characters_Big && (ulGlobalMode != MD_GAME_FADEOUT_CHANGE2 || !GetMiClearStatus()))
	{
		auto etc = GetBigEtc(pnum);

		njSetTexture(&FISHING_TEXLIST);
		njControl3D_Backup();
		njControl3D_Remove(NJD_CONTROL_3D_CONSTANT_MATERIAL);
		njControl3D_Add(NJD_CONTROL_3D_OFFSET_MATERIAL);
		njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
		njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
		ResetMaterial();

		if (!(etc->Big_Fish_Flag & LUREFLAG_MISS))
		{
			njPushMatrixEx();
			njTranslateEx(&twp->pos);
			njRotateY_(-twp->ang.y);
			njRotateX_(twp->ang.x);
			njRotateZ_(twp->ang.z);
			if (twp->mode < 3.0f)
				njTranslate(0, 0.6f, -0.3f, 0.0f);
			njScale(0, 0.3f, 0.3f, 0.3f);
			dsDrawObject(lure_kind[getLureKind()]);
			njPopMatrixEx();
		}

		if (twp->mode >= 3)
		{
			String_Dsp((String*)tp->awp->work.ul[2]);
			DrawFishingMeter(pnum, etc->reel_length_d, etc->reel_tension, twp->mode == 3 || twp->mode > 6 ? 0 : etc->reel_angle);
		}

		njControl3D_Restore();
	}
}

static void __cdecl dispFishingLure_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		dispFishingLure_m(tp);
	}
	else
	{
		TARGET_DYNAMIC(dispFishingLure)(tp);
	}
}

static void __cdecl dispFishingLureSwitch_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		dispFishingLure_m(tp);
	}
	else
	{
		TARGET_DYNAMIC(dispFishingLureSwitch)(tp);
	}
}
#pragma endregion

#pragma region DustTextureCtrl
static void __cdecl DustTextureCtrl_m(task* tp)
{
	auto twp = tp->twp;

	switch (twp->mode)
	{
	case 0i8:
		tp->disp = (TaskFuncPtr)0x46ECA0;
		twp->counter.l = 0;
		twp->wtimer = 5i16;
		twp->mode = 1i8;
		break;
	case 1i8:
	{
		if (twp->counter.l != 7)
		{
			if (!--twp->wtimer)
			{
				twp->wtimer = 5i16;
				++twp->counter.l;
			}
			break;
		}

		twp->mode = 2i8;
		twp->wtimer = 10i16;

		auto etc = GetBigEtc(twp->btimer);
		if (etc)
		{
			etc->Big_Fish_Flag &= ~LUREFLAG_PTCL;
		}

		break;
	}
	case 2i8:
		if (twp->value.f <= 0.0f)
		{
			FreeTask(tp);
			return;
		}
		if (!--twp->wtimer)
		{
			twp->wtimer = 10;
			twp->value.f -= 0.1f;
		}
		break;
	}

	tp->disp(tp);
}

static void setDustTextureCtrl_m(NJS_POINT3* pos, Angle ang, int pnum)
{
	auto tp = CreateElementalTask(2u, 2, DustTextureCtrl_m);
	if (tp)
	{
		tp->twp->pos = *pos;
		tp->twp->ang.y = ang;
		tp->twp->btimer = pnum;
		tp->twp->value.f = 1.0f;
	}
}
#pragma endregion

#pragma region fishingLureCtrl
static void calcTension_m(taskwk* twp, motionwk* mwp, BIGETC* etc, NJS_POINT3* vec_p, int pnum)
{
	auto fish_tp = etc->Big_Fish_Ptr;

	if (fish_tp)
	{
		auto pper = per[pnum];
		auto fish_mwp = fish_tp->mwp;
		float weight = fish_mwp && fish_mwp->weight == 0.0f ? 0.1f : fish_mwp->weight * 0.0001f;
		
		if (etc->Big_Fish_Flag & LUREFLAG_HIT)
		{
			if (pper->on & (AttackButtons | JumpButtons))
			{
				etc->reel_tension_add += 0.0005f;
			}
			else
			{
				if (etc->reel_tension_add > 0.0f)
				{
					etc->reel_tension_add -= 0.001f;
				}
			}

			if ((etc->Big_Fish_Flag & LUREFLAG_SWING) && etc->reel_tension_add > 0.0f)
				etc->reel_tension_add -= 0.001f;

			float idk = njCos(etc->Big_Fish_Ptr->twp->ang.y - mwp->ang_aim.y) * njScalor(&etc->Big_Fish_Ptr->mwp->spd);

			if (etc->Big_Fish_Flag & LUREFLAG_ESCAPE)
			{
				if (idk >= 0.0f)
				{
					idk = weight + weight;
				}
				else
				{
					//etc->reel_tension_aim = weight * 1.1f - idk * 2.1f;
					//idk = etc->reel_tension_aim + (float)((double)rand() * 0.00003 * (0.24f - weight) * 1.2);
				}

				if (idk < 0.5f)
				{
					if (pper->on & JumpButtons)
					{
						idk += 0.5f;
					}
					else if (pper->on & AttackButtons)
					{
						idk += 0.3f;
					}
				}

				etc->reel_tension_aim = idk + etc->reel_tension_add;
				if (etc->Big_Fishing_Timer > 1800 && etc->reel_tension_aim > 0.8f)
				{
					etc->reel_tension_aim = etc->reel_tension_aim - 0.1f;
				}

				njAddVector(vec_p, &mwp->spd);
			}
			else
			{
				if (idk >= 0.0f)
				{
					idk = weight;
				}
				else
				{
					idk = weight - (idk + idk);
				}

				if (idk < 0.5f)
				{
					if (pper->on & JumpButtons)
					{
						idk += 0.5f;
					}
					else if (pper->on & AttackButtons)
					{
						idk += 0.3f;
					}
				}

				etc->reel_tension_aim = idk + etc->reel_tension_add;
			}
		}
		else
		{
			if (etc->reel_tension_add > 0.0f)
			{
				etc->reel_tension_add -= 0.001f;
			}

			if (etc->Big_Fish_Flag & LUREFLAG_SWING)
			{
				etc->reel_tension_aim = 0.1f;
			}
			else
			{
				etc->reel_tension_aim = 0.2f;
			}
		}
	}
	else
	{
		etc->reel_tension_aim = 0.1f;
	}

	etc->reel_tension_aim += (float)((double)rand() * 0.000030517578 * 0.1);
	
	if (!loop_count)
	{
		if (etc->reel_tension_aim != etc->reel_tension)
		{
			float diff = etc->reel_tension_aim - etc->reel_tension;
			if (diff <= 0.02f)
			{
				if (diff >= -0.02f)
				{
					if (diff <= 0.002f)
					{
						if (diff >= -0.002f)
						{
							etc->reel_tension = reel_tension_aim;
						}
						else
						{
							etc->reel_tension -= 0.0011764707f;
						}
					}
					else
					{
						etc->reel_tension += 0.0011764707f;
					}
				}
				else
				{
					etc->reel_tension -= 0.011764705f;
				}
			}
			else
			{
				etc->reel_tension += 0.011764705f;
			}
		}
		if (etc->reel_tension >= 0.0f)
		{
			if (etc->reel_tension >= 1.0f)
			{
				etc->reel_tension = 1.0f;
				dsStop_num(855);
				etc->Big_Fish_Flag |= LUREFLAG_MISS;
				PlayJingle(46);

				if (etc->Big_Fish_Ptr)
				{
					//CreateBigDisplayFishWeight(etc->Big_Fish_Ptr->mwp->weight, 999);
				}
			}
		}
		else
		{
			etc->reel_tension = 0.0f;
		}
	}

	if (etc->caution_timer)
	{
		--etc->caution_timer;
	}

	if (etc->reel_tension < 0.8f)
	{
		dsStop_num(855);
	}
	else if (etc->caution_timer == 0)
	{
		dsStop_num(855);
		dsPlay_oneshot(855, 0, 0, 0);
		etc->caution_timer = 120;
	}
}

static bool chkLureDistance_m(BIGETC* etc, NJS_POINT3* rod_pos_p, NJS_POINT3* lure_pos_p)
{
	NJS_POINT3 v;
	v.x = rod_pos_p->x - lure_pos_p->x;
	v.y = 0.0f;
	v.z = rod_pos_p->z - lure_pos_p->z;

	if (GetStageNumber() != LevelAndActIDs_IceCap4)
	{
		return njScalor(&v) < 20.0f;
	}

	if (njScalor(&v) >= 20.0f)
	{
		return false;
	}

	return etc->water_level - 10.0f < lure_pos_p->y;
}

static void setLureReturn_m(taskwk* twp, BIGETC* etc, int pnum)
{
	if ((GetLevelType() != 1 || GameMode == MD_GAME_FADEOUT_CHANGE2) && !IsLevelChaoGarden())
	{
		dsStop_num(845);
		etc->Big_Fish_Flag &= ~LUREFLAG_FISH;

		//if (twp->mode > 4)
		//	CameraReleaseCollisionCamera();
		//else
		//	CameraReleaseEventCamera();

		if (etc->Big_Fish_Flag & LUREFLAG_HIT)
		{
			BGM_Replay();

			if (GetDistance(&twp->pos, &playertwp[pnum]->pos) < 50.0f)
			{
				twp->mode = MODE_LURE_RETURN_GET;
				//CameraSetEventCameraFunc(CameraFishingCatch, 0, 2);
				//setCatchCameraPos(tp);
			}
			else
			{
				twp->mode = MODE_LURE_RETURN;

				if (etc->Big_Fish_Ptr)
				{
					FreeTask(etc->Big_Fish_Ptr);
					etc->Big_Fish_Ptr = nullptr;
				}

				if (GetStageNumber() == LevelAndActIDs_HotShelter1)
				{
					//CameraSetEventCameraFunc(CameraFishingCatch, 0, 2);
					//setCatchCameraPos(tp);
				}
			}
		}
		else
		{
			twp->mode = MODE_LURE_RETURN;
			//CameraSetEventCameraFunc(CameraFishing, 0, 2);
			//setCameraReturn(v4)
		}
	}
}

static void setLureSpd_D_m(motionwk* mwp, BIGETC* etc)
{
	if (mwp->spd.x > 0.0f)
	{
		mwp->spd.x -= 0.05f;
	}
	if (mwp->spd.x < 0.0f)
	{
		mwp->spd.x += 0.05f;
	}
	if (mwp->spd.x < 0.1f && mwp->spd.x > -0.1f)
	{
		mwp->spd.x = 0.0f;
	}
	if (mwp->spd.z > 0.0f)
	{
		mwp->spd.z -= 0.05f;
	}
	if (mwp->spd.z < 0.0f)
	{
		mwp->spd.z += 0.05f;
	}
	if (mwp->spd.z < 0.1f && mwp->spd.z > -0.1f)
	{
		mwp->spd.z = 0.0f;
	}

	if (etc->Big_Fish_Flag & LUREFLAG_HIT)
	{
		if (etc->Big_Fish_Flag & LUREFLAG_COL)
		{
			mwp->spd.y = 0.1f;
		}
		else if ((float)((double)rand() * 0.000030517578) >= 0.8f)
		{
			mwp->spd.y = 0.05f;
		}
		else
		{
			mwp->spd.y = -0.05f;
		}
	}
	else
	{
		mwp->spd.y -= 0.005f;

		if (GetStageNumber() != LevelAndActIDs_IceCap4)
		{
			if (mwp->spd.y < -0.05f)
			{
				mwp->spd.y = -0.05f;
			}
		}
		else
		{
			if (mwp->spd.y < -0.1f)
			{
				mwp->spd.y = -0.1f;
			}
		}
	}

	if (etc->Big_Fish_Flag & LUREFLAG_COL)
		mwp->spd.y = 0.0f;

	dsStop_num(845);
}

static void setLureSpd_S_m(taskwk* twp, motionwk* mwp, BIGETC* etc, NJS_POINT3* rod_pos_p, int pnum)
{
	float wratio = etc->Big_Fish_Ptr && etc->Big_Fish_Ptr->awp && (etc->Big_Fish_Flag & LUREFLAG_HIT) ? 5000.0f / (etc->Big_Fish_Ptr->mwp->weight + 5000.0f) : 1.0f;

	auto _reel_angle = etc->reel_angle;
	etc->reel_angle += 0x71C;
	etc->reel_length_d -= 0.1f;

	if (((_reel_angle ^ etc->reel_angle) >> 8) & 0x80u)
	{
		dsStop_num(845);
		dsPlay_oneshot(845, 0, 0, 0);
	}

	if (etc->reel_length > etc->reel_length_d)
	{
		mwp->spd.x = rod_pos_p->x - twp->pos.x;
		mwp->spd.z = rod_pos_p->z - twp->pos.z;
		if (chkLureDistance_m(etc, rod_pos_p, &twp->pos))
		{
			setLureReturn_m(twp, etc, pnum);
		}
		njUnitVector(&mwp->spd);
		mwp->spd.x = wratio * mwp->spd.x * 0.08f;
		mwp->spd.z = wratio * mwp->spd.z * 0.08f;
		if (GetStageNumber() == LevelAndActIDs_IceCap4)
		{
			mwp->spd.y = 0.06f;
		}
		else
		{
			mwp->spd.y = 0.02f;
		}
		twp->ang.z = mwp->ang_aim.z + 0x2000;
		if (per[pnum]->on & AttackButtons)
		{
			etc->Big_Fish_Flag |= LUREFLAG_REEL;
		}
	}
	else
	{
		setLureSpd_D_m(mwp, etc);
	}
}

static void setLureSpd_L_m(taskwk* twp, motionwk* mwp, BIGETC* etc, NJS_POINT3* rod_pos_p, int pnum)
{
	float wratio = etc->Big_Fish_Ptr && etc->Big_Fish_Ptr->awp && (etc->Big_Fish_Flag & LUREFLAG_HIT) ? 5000.0f / (etc->Big_Fish_Ptr->mwp->weight + 5000.0f) : 1.0f;

	auto _reel_angle = etc->reel_angle;
	etc->reel_angle += 0x11C7;
	etc->reel_length_d -= 0.2f;

	if (((_reel_angle ^ etc->reel_angle) >> 8) & 0x80u)
	{
		dsStop_num(845);
		dsPlay_oneshot(845, 0, 0, 0);
	}

	if (etc->reel_length > etc->reel_length_d)
	{
		mwp->spd.x = rod_pos_p->x - twp->pos.x;
		mwp->spd.z = rod_pos_p->z - twp->pos.z;
		if (chkLureDistance_m(etc, rod_pos_p, &twp->pos))
		{
			setLureReturn_m(twp, etc, pnum);
		}
		njUnitVector(&mwp->spd);
		mwp->spd.x = wratio * mwp->spd.x * 0.08f;
		mwp->spd.z = wratio * mwp->spd.z * 0.08f;
		if (GetStageNumber() == LevelAndActIDs_IceCap4)
		{
			mwp->spd.y = 0.18f;
		}
		else
		{
			mwp->spd.y = 0.06f;
		}
		twp->ang.z = mwp->ang_aim.z + 0x2000;
		if (per[pnum]->on & AttackButtons)
		{
			etc->Big_Fish_Flag |= LUREFLAG_REEL;
		}
	}
	else
	{
		setLureSpd_D_m(mwp, etc);
	}
}

static void setLureSpd_Swing_m(taskwk* twp, motionwk* mwp, BIGETC* etc, NJS_POINT3* rod_pos_p, int pnum)
{
	if (etc->reel_length_d > etc->reel_length + 5.0f)
	{
		if (mwp->spd.y >= 0.02f)
			mwp->spd.y = 0.02f;
	}
	else
	{
		auto ptwp = playertwp[pnum];
		auto ppwp = playerpwp[pnum];
		
		mwp->spd.x = rod_pos_p->x - twp->pos.x;
		mwp->spd.y = rod_pos_p->y - twp->pos.y;
		mwp->spd.z = rod_pos_p->z - twp->pos.z;
		njUnitVector(&mwp->spd);
		mwp->spd.x *= 0.5f;
		mwp->spd.y *= 0.5f;
		mwp->spd.z *= 0.5f;

		auto reqaction = ppwp->mj.reqaction;
		auto diffang = SubAngle(ptwp->ang.y, mwp->ang_aim.y);
		if (reqaction == 57 || reqaction == 59 || reqaction == 71 || reqaction == 73)
		{
			if (diffang < 0x1555u || diffang > 0x8000u)
			{
				NJS_POINT3 v = { 0.0f, 0.0f, 1.0f };
				njPushMatrix(_nj_unit_matrix_);
				njRotateY_(-LOWORD(twp->ang.y));
				njCalcVector(0, &v, &v);
				njPopMatrixEx();
				njAddVector(&mwp->spd, &v);
				njUnitVector(&mwp->spd);

				if (twp->ang.y - mwp->ang_aim.y > -0x2000)
				{
					twp->ang.y += 0x100;
				}

				mwp->spd.x *= 0.5f;
				mwp->spd.y = 0.04f;
				mwp->spd.z *= 0.5f;

				twp->ang.z = mwp->ang_aim.z + 0x1000;

				if (ppwp->mj.nframe == 15.0f)
					etc->Big_Fish_Flag |= 0x1000u;

				return;
			}
		}
		else if (reqaction == 56 || reqaction == 58 || reqaction == 70 || reqaction == 72)
		{
			if (diffang > 0xEAAAu || diffang < 0x8000u)
			{
				NJS_POINT3 v = { 0.0f, 0.0f, -1.0f };
				njPushMatrix(_nj_unit_matrix_);
				njRotateY_(-LOWORD(twp->ang.y));
				njCalcVector(0, &v, &v);
				njPopMatrixEx();
				njAddVector(&mwp->spd, &v);
				njUnitVector(&mwp->spd);

				if (twp->ang.y - mwp->ang_aim.y < 0x2000)
				{
					twp->ang.y += 0x100;
				}

				mwp->spd.x *= 0.5f;
				mwp->spd.y = 0.04f;
				mwp->spd.z *= 0.5f;

				twp->ang.z = mwp->ang_aim.z + 0x1000;

				if (ppwp->mj.nframe == 15.0f)
					etc->Big_Fish_Flag |= 0x1000u;

				return;
			}
		}

		mwp->spd.x = 0.0f;
		mwp->spd.z = 0.0f;

		if (mwp->spd.y >= 0.02f)
			mwp->spd.y = 0.02f;
	}

	setLureSpd_D_m(mwp, etc);
}

static bool iceWaterCheck_m(BIGETC* etc)
{
	if (GetStageNumber() != LevelAndActIDs_IceCap4)
	{
		return false;
	}

	for (size_t i = 0ui32; i < icecap_fpoint_tbl.size(); ++i)
	{
		if (GetDistance(&etc->big_item_pos, &icecap_fpoint_tbl[i]) < 70.0f)
		{
			return true;
		}
	}

	return false;
}

static int getButtons(int pnum) //custom, probably inline
{
	int ret = (AttackButtons & per[pnum]->on) != 0;
	if (per[pnum]->on & JumpButtons)
	{
		ret |= 2u;
	}
	return ret;
}

static void MoveFishingLureSink_m(taskwk* twp, motionwk* mwp, BIGETC* etc, NJS_POINT3* rod_pos, int pnum)
{
	auto ptwp = playertwp[pnum];
	auto ppwp = playerpwp[pnum];
	auto pper = per[pnum];
	float lure_radius = etc->Big_Fish_Flag & LUREFLAG_HIT ? 1.5f : 0.5f;

	Big_ypos big_ypos;
	calcFishingLureY(twp, &big_ypos, 3);

	if (big_ypos.water.ypos == 1000000.0f)
	{
		big_ypos.water.ypos = etc->water_level;
	}
	else
	{
		etc->water_level = big_ypos.water.ypos;
	}

	float gap = big_ypos.top.ypos - big_ypos.water.ypos;
	if (gap < 2.0f && gap > -2.0f)
	{
		if (iceWaterCheck_m(etc))
		{
			twp->pos.y = big_ypos.top.ypos;
			setLureReturn_m(twp, etc, pnum);
			return;
		}
	}

	if (((etc->Big_Fish_Flag & LUREFLAG_ESCAPE) || !(etc->Big_Fish_Flag & LUREFLAG_SWING)) && (etc->Big_Fish_Flag & LUREFLAG_HIT) && !(AttackButtons & pper->on) && !(JumpButtons & pper->on))
	{
		if (etc->Big_Fish_Ptr && etc->Big_Fish_Ptr->mwp)
		{
			NJS_POINT3 pos = twp->pos;
			NJS_POINT3 spd = mwp->spd;
			Angle3 ang{};

			auto test1 = MSetPosition(&pos, &spd, &ang, lure_radius);
			
			pos = twp->pos;
			Angle3 ang2{};

			auto test2 = test1 /*|| BigSetPosition(&pos, &spd, &ang2, lure_radius)*/;
			auto test3 = chkKabeAngle2(&ang);
			auto test4 = chkKabeAngle2(&ang2);

			if (test1 & test2 & test3 & test4)
			{
				spd.z = 0.0f;
				spd.y *= 3.0f;
				spd.x = 0.0f;
			}

			auto max_pos = (big_ypos.water.ypos - lure_radius) - 0.1f;

			if (spd.y + twp->pos.y > max_pos && !test3)
			{
				mwp->spd.y = 0.0f;
				twp->pos.y = max_pos;
			}

			auto min_pos = big_ypos.bottom.ypos + lure_radius;

			if (spd.y + twp->pos.y <= min_pos)
			{
				auto dst = njScalor(&spd);

				if (twp->pos.y == max_pos || dst <= 0.03f)
				{
					spd = { 0.0f, 0.0f, 0.0f };
				}
			}

			mwp->spd = spd;
			twp->pos.x += spd.x;
			twp->pos.y += spd.y;
			twp->pos.z += spd.z;

			calcTension_m(twp, mwp, etc, &spd, pnum);

			if (etc->reel_length > (ppwp->equipment & Upgrades_PowerRod ? 400.0f : 300.0f))
			{
				etc->Big_Fish_Flag |= LUREFLAG_RANGEOUT;
			}

			if (!(pper->on & AttackButtons) && !(pper->on & JumpButtons))
			{
				dsStop_num(845);
			}
		}
	}
	else
	{
		if (etc->Big_Fish_Flag & LUREFLAG_SWING)
		{
			setLureSpd_Swing_m(twp, mwp, etc, rod_pos, pnum);
		}
		else
		{
			int buttons = getButtons(pnum);

			if (buttons == 1)
			{
				setLureSpd_S_m(twp, mwp, etc, rod_pos, pnum);
			}
			else if (buttons <= 1 || buttons > 3)
			{
				setLureSpd_D_m(mwp, etc);
			}
			else
			{
				setLureSpd_L_m(twp, mwp, etc, rod_pos, pnum);
			}
		}

		NJS_POINT3 pos = twp->pos;
		NJS_POINT3 spd = mwp->spd;
		Angle3 ang{};

		auto test1 = MSetPosition(&pos, &spd, &ang, lure_radius);

		if (GetStageNumber() == LevelAndActIDs_IceCap4 && !ang.x && ang.z == 0x8000)
		{
			spd.y = mwp->spd.y;
			test1 = FALSE;
		}

		pos = twp->pos;
		Angle3 ang2{};

		auto test2 = test1/* || BigSetPosition(&pos, &spd, &ang2, lure_radius)*/;
		auto test3 = chkKabeAngle2(&ang);
		auto test4 = chkKabeAngle2(&ang2);

		if (test1 & test2 & test3 & test4)
		{
			spd.z = 0.0f;
			spd.y *= 3.0f;
			spd.x = 0.0f;
		}

		auto max_pos = (big_ypos.water.ypos - lure_radius) - 0.1f;

		if (spd.y + twp->pos.y > max_pos)
		{
			if ((test3 || test4) && GetStageNumber() != LevelAndActIDs_EmeraldCoast1 && !(etc->Big_Fish_Flag & LUREFLAG_SWING))
			{
				setLureReturn_m(twp, etc, pnum);
				return;
			}

			twp->pos.y = max_pos;
			mwp->spd.y = 0.0f;
		}

		auto min_pos = big_ypos.bottom.ypos + lure_radius;

		if (spd.y + twp->pos.y <= min_pos)
		{
			twp->pos.y = min_pos;

			auto dst = njScalor(&spd);
			etc->Big_Fish_Flag |= LUREFLAG_COL;

			if (!(etc->Big_Fish_Flag & LUREFLAG_PTCL) && dst >= 0.15f)
			{
				setDustTextureCtrl_m(&twp->pos, twp->ang.y, pnum);
				dsPlay_oneshot(854, 0, 0, 0);
				etc->Big_Fish_Flag |= LUREFLAG_PTCL;
			}

			if (dst <= 0.03f)
			{
				spd = { 0.0f, 0.0f, 0.0f };
			}
		}

		twp->pos.x += spd.x;
		twp->pos.y += spd.y;
		twp->pos.z += spd.z;

		calcTension_m(twp, mwp, etc, &spd, pnum);

		etc->reel_tension_aim += (float)((double)rand() * 0.000030517578 * 0.1);

		if (twp->pos.y > max_pos + 0.1f)
		{
			setLureReturn_m(twp, etc, pnum);
		}
	}
}

static void moveFishingRotX_m(taskwk* twp, motionwk* mwp, BIGETC* etc, int pnum)
{
	auto ptwp = playertwp[pnum];
	auto pper = per[pnum];

	mwp->ang_aim.y = 0x4000 - NJM_RAD_ANG(atan2f(twp->pos.x - ptwp->pos.x, twp->pos.z - ptwp->pos.z));

	if (etc->Big_Fish_Flag & LUREFLAG_HIT)
	{
		if (twp->ang.x <= 0x8000)
		{
			twp->ang.x -= 0x80;
			if (twp->ang.x <= 0)
			{
				twp->ang.x = 0;
			}
		}
		else
		{
			twp->ang.x += 0x80;
			if (twp->ang.x >= 0x10000)
			{
				twp->ang.x = 0;
			}
		}
	}
	else if (etc->Big_Fish_Flag & LUREFLAG_COL)
	{
		if (twp->ang.x < 0)
		{
			twp->ang.x -= 0x80;
			if (twp->ang.x < -0x1800)
			{
				twp->ang.x = -0x1800;
			}
		}
		else
		{
			twp->ang.x += 0x80;
			if (twp->ang.x > 0x180)
			{
				twp->ang.x = 0x180;
			}
		}
	}
	else
	{
		if (mwp->spd.x == 0.0f && mwp->spd.z == 0.0f)
		{
			if (twp->ang.x <= 0x8000)
			{
				twp->ang.x -= 0x80;
				if (twp->ang.x <= 0)
				{
					twp->ang.x = 0;
				}
			}
			else
			{
				twp->ang.x += 0x80;
				if (twp->ang.x >= 0x10000)
				{
					twp->ang.x = 0;
				}
			}
		}
		else
		{
			int buttons = getButtons(pnum);
			Angle min, max;

			if (buttons < 2 || buttons > 3)
			{
				min = 0x200;
				max = 0x800;
			}
			else
			{
				min = 0xC00;
				max = 0x1400;
			}

			if (twp->ang.x > max && twp->ang.x < max + 0x3000)
			{
				mwp->ang_aim.x = 0x10000 - max;
				twp->ang.x -= min;
			}
			if (twp->ang.x > 0xD000 - max && twp->ang.x < 0x10000 - max)
			{
				mwp->ang_aim.x = max;
				twp->ang.x -= min;
			}
			if (mwp->ang_aim.x < 0x8000)
			{
				twp->ang.x += min;
			}
		}
	}
}

static void moveFishingRotZ_m(taskwk* twp, motionwk* mwp, BIGETC* etc, NJS_POINT3* rod_pos)
{
	if ((etc->Big_Fish_Flag & LUREFLAG_COL) || GetStageNumber() != LevelAndActIDs_IceCap4)
	{
		mwp->ang_aim.z = 0;
	}
	else
	{
		NJS_POINT3 v;
		v.x = rod_pos->x - twp->pos.x;
		v.y = rod_pos->y - twp->pos.y;
		v.z = rod_pos->z - twp->pos.z;
		njUnitVector(&v);
		mwp->ang_aim.z = NJM_RAD_ANG(-asinf(v.y));
	}

	Angle range = twp->ang.z - mwp->ang_aim.z;

	if (range >= 0x8000)
	{
		if (range > 0x8000)
		{
			twp->ang.z += 128;
		}
	}
	else
	{
		twp->ang.z -= 128;
	}

	if (range < 128 || range > 0xFF80)
	{
		twp->ang.z = range;
	}
}

static bool ReturnFishingLure_m(taskwk* twp, motionwk* mwp, NJS_POINT3* rod_pos)
{
	mwp->spd.x = rod_pos->x - twp->pos.x;
	mwp->spd.y = rod_pos->y - twp->pos.y;
	mwp->spd.z = rod_pos->z - twp->pos.z;
	njUnitVector(&mwp->spd);

	float scl = twp->mode == MODE_LURE_RETURN ? 10.0f : 2.0f;
	
	twp->pos.x += scl * mwp->spd.x;
	twp->pos.y += scl * mwp->spd.y;
	twp->pos.z += scl * mwp->spd.z;

	return twp->pos.x + 5.0f > rod_pos->x
		&& twp->pos.x - 5.0f < rod_pos->x
		&& twp->pos.y + 5.0f > rod_pos->y
		&& twp->pos.y - 5.0f < rod_pos->y
		&& twp->pos.z + 5.0f > rod_pos->z
		&& twp->pos.z - 5.0f < rod_pos->z;
}

static void CalcLinePos_m(taskwk* twp, NJS_POINT3* ret)
{
	*ret = { -0.6f, 0.3f, 0.0f };
	njPushMatrix(_nj_unit_matrix_);
	njRotateY_(-LOWORD(twp->ang.y));
	njRotateX_(twp->ang.x);
	njRotateZ_(twp->ang.z);
	njCalcVector(0, ret, ret);
	ret->x = twp->pos.x + ret->x;
	ret->y = twp->pos.y + ret->y;
	ret->z = twp->pos.z + ret->z;
	njPopMatrixEx();
}

static void CalcLineLength(BIGETC* etc, taskwk* ptwp, NJS_POINT3* line_pos_p) // custom, originally in display function
{
	etc->reel_length = GetDistance(line_pos_p, &ptwp->pos);

	if (etc->reel_length > etc->reel_length_d)
	{
		etc->reel_length_d = etc->reel_length;
	}

	if (etc->reel_length_d >= etc->reel_length + 5.0f)
	{
		etc->reel_length_d = etc->reel_length + 5.0f;
	}
}

static void CalcRodPos_m(taskwk* ptwp, playerwk* ppwp, NJS_POINT3* rod_pos_p)
{
	NJS_VECTOR v = { 0.0f, ppwp->p.center_height, 0.0f };
	njPushMatrix(_nj_unit_matrix_);
	njRotateZ_(ptwp->ang.z);
	njRotateX_(ptwp->ang.x);
	njRotateY_(0x8000ui16 - LOWORD(ptwp->ang.y));
	njCalcVector(0, &v, rod_pos_p);

	float dist = (ppwp->equipment & Upgrades_PowerRod) != 0 ? 17.0f : 16.0f;
	
	v.x = ppwp->user0_pos.x - dist * ppwp->user0_vec.x;
	v.y = ppwp->user0_pos.y - dist * ppwp->user0_vec.y;
	v.z = ppwp->user0_pos.z - dist * ppwp->user0_vec.z;

	if (ppwp->equipment & Upgrades_PowerRod)
	{
		v.y -= 1.8f;
		v.z += 0.6f;
	}
	else
	{
		v.y -= 0.6f;
	}

	njCalcVector(0, &v, &v);

	rod_pos_p->x = ptwp->pos.x + rod_pos_p->x + v.x;
	rod_pos_p->y = ptwp->pos.y + rod_pos_p->y + v.y;
	rod_pos_p->z = ptwp->pos.z + rod_pos_p->z + v.z;
	
	njPopMatrixEx();
}

static void setLurePosWait_m(task* tp, BIGETC* etc, taskwk* ptwp, NJS_POINT3* rod_pos_p)
{
	auto twp = tp->twp;
	auto mwp = tp->mwp;

	twp->pos = *rod_pos_p;
	twp->ang.y = tp->mwp->ang_aim.y;
	mwp->ang_aim.y = ptwp->ang.y;

	if (twp->ang.x >= 0x5800 && twp->ang.x <= 0xA800)
	{
		if (twp->ang.x < 0x6000)
		{
			mwp->ang_aim.x = 0xA000;
		}
	}
	else
	{
		twp->ang.x = 0x5800;
	}

	if (twp->ang.x <= 0xA000)
	{
		if (mwp->ang_aim.x >= 0x8000)
		{
			twp->ang.x += 0x200;
		}
		else
		{
			twp->ang.x -= 0x200;
		}
	}
	else
	{
		mwp->ang_aim.x = 0x6000;
		twp->ang.x -= 0x200;
	}

	if (twp->ang.z >= 0x2800 && twp->ang.z <= 0x5800)
	{
		if (twp->ang.z < 0x3000)
		{
			mwp->ang_aim.z = 0x5000;
		}
		if (twp->ang.z <= 0x5000)
		{
			if (mwp->ang_aim.z >= 0x4000)
			{
				twp->ang.z += 0x80;
			}
			else
			{
				twp->ang.z -= 0x80;
			}
		}
		else
		{
			mwp->ang_aim.z = 0x3000;
			twp->ang.z -= 0x80;
		}
	}
	else
	{
		mwp->ang_aim.z = 0x3000;
		twp->ang.z = 0x5780;
	}
}

static void setLureSetup_m(task* tp, BIGETC* etc)
{
	tp->twp->mode = MODE_LURE_SETUP;
	auto awp = tp->awp;
	awp->work.uw[2] = 0i16;
	awp->work.uw[3] = 0i16;
	etc->reel_angle = 0;
	etc->reel_tension = 0.0f;
	etc->reel_tension_aim = 0.0f;
	etc->reel_length_d = 0.0f;
	etc->reel_length = 0.0f;
	if (etc->Big_Fish_Flag & 1)
		BGM_Replay();
	dsStop_num(845);
}

static void fishingLureCtrl_m(task* tp)
{
	auto twp = tp->twp;
	auto pnum = twp->btimer;
	auto ptwp = playertwp[pnum];

	if (!ptwp)
	{
		FreeTask(tp);
		return;
	}

	auto mwp = tp->mwp;
	auto lure = (lurewk*)tp->awp;
	auto ppwp = playerpwp[pnum];
	auto etc = GetBigEtc(pnum);

	NJS_POINT3 rod_pos;
	NJS_POINT3 line_pos;
	CalcRodPos_m(ptwp, ppwp, &rod_pos);

	switch (twp->mode)
	{
	case MODE_LURE_INIT:
		setLureSetup_m(tp, etc);
		tp->dest = exitFishingLure;
		tp->disp = dispFishingLure;
		break;
	case MODE_LURE_SETUP:
		setLurePosWait_m(tp, etc, ptwp, &rod_pos);

		if (etc->Big_Fish_Flag & LUREFLAG_LAUNCH)
		{
			etc->Big_Fish_Flag &= ~LUREFLAG_LAUNCH;
			twp->mode = MODE_LURE_WAIT;
			twp->timer.l = 10;

			task* cursor_tp = (task*)ppwp->free.ptr[1];
			if (cursor_tp)
			{
				lure->pos->x = cursor_tp->twp->pos.x;
				lure->pos->y = cursor_tp->awp->work.f[1];
				lure->pos->z = cursor_tp->twp->pos.z;
				FreeTask(cursor_tp);
			}

			etc->Big_Fish_Flag = 0;
		}
		break;
	case MODE_LURE_WAIT:
		setLurePosWait_m(tp, etc, ptwp, &rod_pos);

		if (--twp->timer.l <= 0)
		{
			CCL_Init(tp, &lure_colli_tbl, 1, 1u);
			NJS_POINT3 v = { lure->pos->x - twp->pos.x, 0.0f, lure->pos->z - twp->pos.z };
			float dist = njScalor(&v) * 0.3f;
			float scl = 1.0f / dist;

			etc->distancep = 99999.0f;

			mwp->ang_aim.x = 0;
			mwp->spd.x = v.x * scl;
			mwp->spd.y = (lure->pos->y - twp->pos.y) * scl + dist * 0.01f;
			mwp->spd.z = v.z * scl;

			if (lure->string)
			{
				CalcLinePos_m(twp, &line_pos);
				String_IniEasy(lure->string, &rod_pos, &line_pos);
			}

			twp->mode = MODE_LURE_NORMAL;
			dsPlay_oneshot(842, 0, 0, 0);
			dsPlay_oneshot(1318, 0, 0, 0);
		}

		break;
	case MODE_LURE_NORMAL:
		++twp->timer.l;

		if (sub_46D0D0(tp))
		{
			Big_ypos big_ypos;
			calcFishingLureY(twp, &big_ypos, 0);

			if (sub_46D970(&big_ypos, tp))
			{
				twp->mode = MODE_LURE_FISHING_INIT;
				twp->pos.x = lure->pos->x;
				twp->pos.y = big_ypos.water.ypos - 0.6f;
				twp->pos.z = lure->pos->z;
			}
			else
			{
				etc->Big_Fish_Flag &= ~LUREFLAG_FISH;
				twp->mode = MODE_LURE_RETURN;
				dsPlay_oneshot(844, 0, 0, 0);
			}
		}
		else
		{
			etc->distance = GetDistance(&twp->pos, lure->pos);
		}

		if (etc->distance < 3.0f)
		{
			twp->pos = *lure->pos;
			twp->mode = MODE_LURE_FISHING_INIT;
			Big_ypos big_ypos;
			calcFishingLureY(twp, &big_ypos, 0);

			if (sub_46D970(&big_ypos, tp))
			{
				twp->pos.y = big_ypos.water.ypos - 0.6f;
			}
			else
			{
				etc->Big_Fish_Flag &= ~LUREFLAG_FISH;
				twp->mode = MODE_LURE_RETURN;
				dsPlay_oneshot(844, 0, 0, 0);
			}
		}
		else if (etc->distance > 500.0f && etc->distancep < etc->distance)
		{
			etc->Big_Fish_Flag &= ~LUREFLAG_FISH;
			twp->mode = MODE_LURE_RETURN;
			dsPlay_oneshot(844, 0, 0, 0);
		}

		etc->distancep = etc->distance;

		if (lure->string)
		{
			CalcLinePos_m(twp, &line_pos);
			String_Exe(lure->string, &rod_pos, &line_pos, 0);
			CalcLineLength(etc, ptwp, &line_pos);
		}

		EntryColliList(twp);

		break;
	case MODE_LURE_FISHING_INIT:
		if ((GetLevelType() != 1 || GameMode == MD_GAME_FADEOUT_CHANGE2) && !IsLevelChaoGarden())
		{
			etc->big_item_pos = twp->pos;
		}

		twp->mode = MODE_LURE_FISHING;
		twp->pos.y -= 0.5f;

		//setLureCameraPos_m(tp);
		MoveFishingLureSink_m(twp, mwp, etc, &rod_pos, pnum);

		if (lure->string)
		{
			CalcLinePos_m(twp, &line_pos);
			String_Exe(lure->string, &rod_pos, &line_pos, 0);
			CalcLineLength(etc, ptwp, &line_pos);
		}

		if (GetLevelType() == 1 && GameMode != MD_GAME_FADEOUT_CHANGE2 || IsLevelChaoGarden())
		{
			if (GetDistance(&etc->big_item_pos, &ptwp->pos) > 20.0f)
			{
				int item = sub_46EE90();
				if (item > 0)
				{
					DoThingWithItemBoxPowerupIndex(item);
					etc->big_item_pos = ptwp->pos;
				}
			}

			etc->Big_Fish_Flag &= ~LUREFLAG_FISH;
			twp->mode = MODE_LURE_RETURN;
			//CameraReleaseEventCamera();
		}
		else
		{
			if (ChkFishingThrowNow_m(pnum))
			{
				setLureSetup_m(tp, etc);
				break;
			}

			if (chkLureDistance_m(etc, &rod_pos, &twp->pos))
			{
				etc->Big_Fish_Flag &= ~LUREFLAG_FISH;
				twp->mode = MODE_LURE_RETURN;
				break;
			}

			if (twp->mode == MODE_LURE_FISHING)
			{
				etc->Big_Fish_Flag |= LUREFLAG_FISH;
				etc->big_item_pos = twp->pos;
				etc->reel_length_d = etc->reel_length;
				mwp->spd.x *= 0.25f;
				mwp->spd.y *= 0.25f;
				mwp->spd.z *= 0.25f;
				twp->ang.x = 0;
				dsPlay_timer(843, (int)twp, 1, 0, 27);
				//CameraReleaseEventCamera();
				//CameraSetCollisionCameraFunc(CameraLureAndFish, 0, 2);
			}
		}
		break;
	case MODE_LURE_FISHING:
		//setLureCameraPos_m(tp);
		MoveFishingLureSink_m(twp, mwp, etc, &rod_pos, pnum);
		moveFishingRotX_m(twp, mwp, etc, pnum);
		moveFishingRotY(tp);
		moveFishingRotZ_m(twp, mwp, etc, &rod_pos);

		if (lure->string)
		{
			CalcLinePos_m(twp, &line_pos);
			String_Exe(lure->string, &rod_pos, &line_pos, 1);
			CalcLineLength(etc, ptwp, &line_pos);
		}

		if ((per[pnum]->on & Buttons_L) && (per[pnum]->on & Buttons_L) && !(etc->Big_Fish_Flag & LUREFLAG_HIT))
		{
			etc->Big_Fish_Flag |= LUREFLAG_CANCEL;

			/*
			CameraReleaseCollisionCamera();
			if (GetStageNumber() == LevelAndActIDs_HotShelter1)
			{
				//CameraSetEventCameraFunc(CameraFishingCatch, 0, 2);
				//setCatchCameraPos(tp);
			}
			else
			{
				CameraSetEventCameraFunc(CameraFishing, 0, 2);
				setCameraReturn(tp);

				if (checkturipoint2())
				{
					CameraReleaseEventCamera();
					ResetCameraTimer();
				}
			}
			*/
		}

		if ((etc->Big_Fish_Flag & LUREFLAG_HOOK) && GetStageNumber() == LevelAndActIDs_HotShelter1 && !(etc->Big_Fish_Flag & LUREFLAG_HIT))
		{
			etc->Big_Fish_Flag |= LUREFLAG_CANCEL;
			etc->Big_Fish_Flag &= ~LUREFLAG_HOOK;

			/*
			CameraReleaseCollisionCamera();
			SetCameraEvent(sub_46E4C0, 0, 2);
			setCatchCameraPos(tp);
			*/
		}

		if (etc->Big_Fish_Flag & LUREFLAG_CANCEL)
		{
			if (etc->Big_Fish_Ptr)
			{
				etc->Big_Fish_Flag &= ~LUREFLAG_FISH;
				twp->mode = MODE_LURE_RETURN;
				dsStop_num(845);
			}
		}
		else
		{
			if (ChkFishingThrowNow_m(pnum))
			{
				setLureSetup_m(tp, etc);
				break;
			}

			if (etc->Big_Fish_Flag & LUREFLAG_HIT)
			{
				twp->mode = MODE_LURE_HIT;
				if (etc->Big_Fish_Ptr)
				{
					if (GetStageNumber() == LevelAndActIDs_HotShelter1)
					{
						auto fish_twp = etc->Big_Fish_Ptr->twp;
						if (fish_twp && fish_twp->pos.x == 750.0f && fish_twp->pos.y == 75.5f && fish_twp->pos.z == -650.0f)
							etc->Big_Fish_Flag |= LUREFLAG_HOOK;
					}
				}
			}
		}

		break;
	case MODE_LURE_HIT:
		//setLureCameraPos_m(tp);
		MoveFishingLureSink_m(twp, mwp, etc, &rod_pos, pnum);
		moveFishingRotX_m(twp, mwp, etc, pnum);
		moveFishingRotY(tp);
		moveFishingRotZ_m(twp, mwp, etc, &rod_pos);

		if (lure->string)
		{
			CalcLinePos_m(twp, &line_pos);
			String_Exe(lure->string, &rod_pos, &line_pos, 1);
			CalcLineLength(etc, ptwp, &line_pos);
		}

		if (etc->Big_Lure_Ptr || etc->Big_Fish_Flag & LUREFLAG_HIT)
		{
			if (ChkFishingThrowNow_m(pnum))
			{
				setLureSetup_m(tp, etc);
			}
		}
		else
		{
			if (twp->mode == MODE_LURE_HIT)
			{
				twp->mode = MODE_LURE_FISHING;
			}
			BGM_Replay();
		}
		break;
	case MODE_LURE_RETURN:
	case MODE_LURE_RETURN_GET:
		dsStop_num(855);

		if (ReturnFishingLure_m(twp, mwp, &rod_pos))
		{
			ppwp->free.sb[0] |= 4; // cancel fish mode

			etc->Big_Fish_Flag &= ~LUREFLAG_FISH;
			if (etc->Big_Fish_Ptr)
			{
				if (etc->Big_Fish_Flag & LUREFLAG_HIT)
				{
					//CameraReleaseEventCamera();
					//CameraSetEventCameraFunc(CameraFishingCatch, 0, 2);
					etc->Big_Fish_Flag |= LUREFLAG_RELEASE;
					//setCatchCameraPos(tp)
				}
			}

			setLureSetup_m(tp, etc);
		}
		else
		{
			if (lure->string)
			{
				CalcLinePos_m(twp, &line_pos);
				String_Exe(lure->string, &rod_pos, &line_pos, 1);
				CalcLineLength(etc, ptwp, &line_pos);
			}
		}
		break;
	}

	tp->disp(tp);
}

static void __cdecl fishingLureCtrl_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		fishingLureCtrl_m(tp);
	}
	else
	{
		TARGET_DYNAMIC(fishingLureCtrl)(tp);
	}
}

static task* __cdecl SetFishingLureTask_r(task* tp)
{
	auto lure_tp = TARGET_DYNAMIC(SetFishingLureTask)(tp);
	auto pnum = lure_tp->twp->btimer = TASKWK_PLAYERID(tp->twp);
	bigetc_m[pnum]->Big_Lure_Ptr = lure_tp;
	bigetc_m[pnum]->Big_Fish_Flag = 0;
	return lure_tp;
}
#pragma endregion

#pragma region fishingCursorCtrl
static auto MoveFishingCursor = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x46F850, rEAX);

static void MoveFishingCursor_m(task* tp, int pnum) // todo: rewrite properly
{
	auto backup_twp = playertwp[0];
	auto backup_mwp = playermwp[0];
	playertwp[0] = playertwp[pnum];
	playermwp[0] = playermwp[pnum];
	MoveFishingCursor(tp);
	playertwp[0] = backup_twp;
	playermwp[0] = backup_mwp;
}

static void fishingCursorCtrl_m(task* tp)
{
	auto twp = tp->twp;

	if (twp->mode == 1)
	{
		auto pnum = TASKWK_PLAYERID(twp);
		auto ptwp = playertwp[pnum];

		if (!ptwp || ChkFishingThrowNow_m(pnum))
		{
			FreeTask(tp);
			return;
		}

		auto etc = GetBigEtc(pnum);
		etc->water_level = tp->awp->work.f[1];

		twp->ang.y += 0x200;

		//setCameraFishingTgt_m(tp);
		MoveFishingCursor_m(tp, pnum);

		ptwp->ang.y = 0x4000 - NJM_RAD_ANG(atan2f(twp->pos.x - ptwp->pos.x, twp->pos.z - ptwp->pos.z));
		playermwp[pnum]->ang_aim.y = ptwp->ang.y;
	}

	tp->disp(tp);
}

static void __cdecl fishingCursorCtrl_r(task* tp)
{
	if (multiplayer::IsEnabled() && tp->twp->mode != 0)
	{
		fishingCursorCtrl_m(tp);
	}
	else
	{
		TARGET_DYNAMIC(fishingCursorCtrl)(tp);
	}
}

static task* __cdecl SetFishingCursorTask_r(task* tp)
{
	auto cursor_tp = TARGET_DYNAMIC(SetFishingCursorTask)(tp);
	TASKWK_PLAYERID(cursor_tp->twp) = TASKWK_PLAYERID(tp->twp);
	return cursor_tp;
}
#pragma endregion

static void BigStateInit_r()
{
	for (auto& i : bigetc_m)
	{
		i->Big_Sakana_Weight = 0;
		i->Big_Sakana_Weight_High = 0i16;
		i->Big_Sakana_Kind_High = 0;
		i->Big_Sakana_Weight_Limit = static_cast<int>(BigChkHeavyWeight());
		i->big_item_pos.z = 0.0f;
		i->big_item_pos.y = 0.0f;
		i->big_item_pos.x = 0.0f;
	}
}

void InitFishing()
{
	dispFishingLure_t       = new Trampoline(0x470580, 0x470588, dispFishingLure_r);
	dispFishingLureSwitch_t = new Trampoline(0x4703F0, 0x4703F8, dispFishingLureSwitch_r);
	fishingLureCtrl_t       = new Trampoline(0x471580, 0x471589, fishingLureCtrl_r);
	SetFishingLureTask_t    = new Trampoline(0x471ED0, 0x471ED6, SetFishingLureTask_r);
	fishingCursorCtrl_t     = new Trampoline(0x46FA10, 0x46FA18, fishingCursorCtrl_r);
	SetFishingCursorTask_t  = new Trampoline(0x470330, 0x470336, SetFishingCursorTask_r);

	WriteJump((void*)0x470120, BigStateInit_r);

	WriteData<2>((void*)0x490C52, 0x90ui8); // force fishingLureCtrl to load for every player
	WriteData<6>((void*)0x470425, 0x90ui8); // force things to display even if player 0 is not Big
}
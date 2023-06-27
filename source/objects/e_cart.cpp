#include "pch.h"
#include "camera.h"
#include "../race.h"
#include "e_cart.h"

#define CART_PNUM(twp) twp->btimer

enum CARTMD
{
	CARTMD_INIT,
	CARTMD_SARU,
	CARTMD_WAIT,
	CARTMD_WARP,
	CARTMD_CTRL, // Player controlled
	CARTMD_PASS, // Looping
	CARTMD_GOAL, // IA after course
	CARTMD_EXPL,
	CARTMD_8, // Object controlled
	CARTMD_9,
	CARTMD_10,
	CARTMD_DEST
};

enum CARTTYPE
{
	CARTTYPE_SARU,
	CARTTYPE_BIG,
	CARTTYPE_E102,
};

enum CARTFLG
{
	CARTFLG_SHOWSARU = 1,
	CARTFLG_20 = 0x20,
	CARTFLG_4000 = 0x4000
};

DataArray(float, cartOffset, 0x38A6FE0, 3); // local name "offset" in symbol
DataArray(__int16, color, 0x38A7034, 8); // local name, cartColor[color[charid]]
DataPointer(NJS_POINT3, _velo, 0x7E6FD8); // up vector
VoidFunc(searchAnimationMaterial, 0x7972A0); // local name
DataPointer(int, player_no, 0x3D08E04); // local name
DataArray(int*, panelList, 0x3D08E14, 3 * 3);
DataArray(__int16, cartNumber, 0x88BFEC, 12);
DataArray(int, bodyListNum, 0x3D08EBC, 3);
DataArray(int*, dword_3D08E40, 0x3D08E40, 30);
DataArray(int, cart_se_num, 0x38A6D70, 6 * 2);
DataPointer(float, hamariDist, 0x3D08E10);

task* taskOfPlayerOn_m[PLAYER_MAX];

task* CartChangeForceMode(int num)
{
	auto tp = taskOfPlayerOn_m[num];
	if (tp)
		tp->twp->mode = CARTMD_8;
	return tp;
}

Characters GetPlayerNumberM(int pnum)
{
	return playertwp[pnum] ? (Characters)TASKWK_CHARID(playertwp[pnum]) : Characters_Sonic;
}

int GetParamNumberM(int pnum)
{
	return (ssStageNumber == STAGE_TWINKLEPARK && ssActNumber == 0) ? 0 : GetPlayerNumberM(pnum);
}

void cartDisplayM(task* tp)
{
	auto twp = tp->twp;

	if (dsCheckViewV(&twp->pos, 20.0f))
	{
		cart_data = (ENEMY_CART_DATA*)tp->awp;
		auto pnum = twp->btimer;
		auto cartparam = &CartParameter[GetParamNumberM(pnum)];

		// list of those color effects
		for (int i = 0; i < bodyListNum[cart_data->cart_type]; ++i)
		{
			int v8 = 10 * cart_data->cart_type + i;
			*dword_3D08E40[v8] = cart_data->cart_color + (*dword_3D08E40[v8] & 0xC000);
		}

		// Show ring count
		if (twp->mode == 4 || twp->mode == 5)
		{
			auto rings = max(0, min(99, GetNumRingM(pnum)));
			*panelList[3 * cart_data->cart_type] = cartNumber[rings / 10];
			*panelList[3 * cart_data->cart_type + 1] = cartNumber[rings % 10];
		}
		else
		{
			*panelList[3 * cart_data->cart_type] = 89;
			*panelList[3 * cart_data->cart_type + 1] = 89;
		}

		njSetTexture(&OBJ_SHAREOBJ_TEXLIST);
		njPushMatrixEx();
		// Use the previous position to prevent player offset (SADX calls the display before physics, but we can't do that in splitscreen)
		njTranslate(0, cart_data->last_pos.x, cart_data->last_pos.y + cartOffset[cart_data->cart_type], cart_data->last_pos.z);
		njRotateZ_(twp->ang.z);
		njRotateX_(twp->ang.x);
		njRotateY_(twp->ang.y + 0x4000);
		njRotateZ_(cart_data->unstable.z);
		njRotateX_(cart_data->unstable.x);

		switch (cart_data->cart_type)
		{
		case CARTTYPE_SARU:
			DrawCustomObject(&object_sarucart_sarucart_sarucart, fan_model);
			break;
		case CARTTYPE_BIG:
			DrawCustomObject(&object_b_cart_cart_cart, fan_model_big);
			break;
		case CARTTYPE_E102:
			DrawCustomObject(&object_e_cart_cart_cart, fan_model_e102);
			break;
		}

		if (cart_data->flag & CARTFLG_SHOWSARU)
		{
			dsDrawObject(&object_sarucart_saru_body_saru_body);
		}

		njPopMatrixEx();

		NJS_VECTOR shadowpos = { twp->pos.x, cart_data->shadow_pos + 0.2f, twp->pos.z };
		DrawShadow_(&cart_data->shadow_ang, &shadowpos, cart_data->cart_type == 0 ? 1.2f : 1.5f);

		if (cart_data->vitality <= cartparam->smoke_vitality)
		{
			if (!(GetGlobalTime() % (cart_data->vitality + 8)))
			{
				CreateSmoke(&twp->pos, &_velo, 1.0);
			}
		}
	}
}

void cartDisplayExplosionM(task* tp)
{
	auto twp = tp->twp;

	if (dsCheckViewV(&twp->pos, 20.0f))
	{
		cart_data = (ENEMY_CART_DATA*)tp->awp;
		auto pnum = twp->btimer;
		auto cartparam = &CartParameter[GetParamNumberM(pnum)];

		// list of those color effects
		for (int i = 0; i < bodyListNum[cart_data->cart_type]; ++i)
		{
			int v8 = 10 * cart_data->cart_type + i;
			*dword_3D08E40[v8] = cart_data->cart_color + (*dword_3D08E40[v8] & 0xC000);
		}

		// Show ring count
		if (twp->mode == 4 || twp->mode == 5)
		{
			auto rings = max(0, min(99, GetNumRingM(pnum)));
			*panelList[3 * cart_data->cart_type] = cartNumber[rings / 10];
			*panelList[3 * cart_data->cart_type + 1] = cartNumber[rings % 10];
		}
		else
		{
			*panelList[3 * cart_data->cart_type] = 89;
			*panelList[3 * cart_data->cart_type + 1] = 89;
		}

		njSetTexture(&OBJ_SHAREOBJ_TEXLIST);

		for (int i = 0; i < 10; ++i)
		{
			njPushMatrixEx();
			njTranslateEx(&cart_data->explose_point[i]);
			if (cart_data->explose_angle[i].z) njRotateZ(0, cart_data->explose_angle[i].z);
			if (cart_data->explose_angle[i].x) njRotateX(0, cart_data->explose_angle[i].x);
			if (cart_data->explose_angle[i].y) njRotateY(0, cart_data->explose_angle[i].y + 0x4000);
			dsDrawModel(cart_model[i * cart_data->cart_type]);
			njPopMatrixEx();
		}
	}
}

void cartInitM(task* tp, taskwk* twp, CART_PLAYER_PARAMETER* cartparam, int pnum)
{
	cart_data = (ENEMY_CART_DATA*)AllocateMemory(sizeof(ENEMY_CART_DATA));
	memset(cart_data, 0, sizeof(ENEMY_CART_DATA));
	tp->awp = (anywk*)cart_data;

	CCL_Init(tp, &cci_cart[player_no], 1, 3u);

	// Start mode:
	if (static_cast<int>(twp->scl.y) == 1)
	{
		twp->mode = CARTMD_WAIT;
		cart_data->flag &= ~CARTFLG_SHOWSARU;
	}
	else
	{
		twp->mode = CARTMD_SARU;
		cart_data->flag |= CARTFLG_SHOWSARU;
	}

	// Get type and color
	if (CurrentLevel == LevelIDs_TwinkleCircuit)
	{
		auto chara = GetPlayerNumberM(pnum);
		cart_data->cart_color = cartColor[color[chara]];

		if (chara == Characters_Gamma)
		{
			cart_data->cart_type = CARTTYPE_E102;
		}
		else if (chara == Characters_Big)
		{
			cart_data->cart_type = CARTTYPE_BIG;
		}
	}
	else
	{
		// Model variant:
		char type_param = static_cast<char>(twp->scl.z);
		if (type_param >= 0i8 && type_param < 3i8)
		{
			cart_data->cart_type = type_param;
		}

		// Color variant:
		auto color_param = static_cast<int>(twp->scl.x);
		if (color_param >= 0 && color_param < 7)
		{
			cart_data->cart_color = cartColor[color_param];
		}
	}

	cart_data->vitality = cartparam->max_vitality;

	searchAnimationMaterial();

	twp->id = 10; // "I am a cart!"
	tp->disp = cartDisplayM;
}

void cartSetVectorM(taskwk* twp, int pnum)
{
	if (twp->mode == CARTMD_CTRL)
	{
		taskwk* pltwp = playertwp[pnum];

		twp->pos = pltwp->pos;

		if (pltwp->cwp->flag & 1)
		{
			cart_data->vector.x = twp->pos.x - cart_data->last_pos.x;
			cart_data->vector.y = twp->pos.y - cart_data->last_pos.y;
			cart_data->vector.z = twp->pos.z - cart_data->last_pos.z;
		}

		if (GetPlayerNumberM(pnum) == Characters_Big && njScalor(&cart_data->vector) > 30.0f && njScalor(&cart_data->vector) != 0.0)
		{
			// Don't ask me why
			njUnitVector(&cart_data->vector);
			cart_data->vector.x = 0.0f;
			cart_data->vector.y = 0.0f;
			cart_data->vector.z = 0.0f;
		}
	}

	cart_data->last_pos = twp->pos;
}

void cartThinkM(task* tp, taskwk* twp, int pnum)
{
	// todo: rewrite
	auto backup = playertwp[0];
	playertwp[0] = playertwp[pnum];
	cartThink(twp, tp);
	playertwp[0] = backup;
}

void cartTopographicalCollisionM(task* tp, taskwk* twp, int pnum)
{
	// todo: rewrite
	auto backup1 = playertp[0];
	auto backup2 = playertwp[0];
	auto backup3 = camera_twp->pos;
	playertp[0] = playertp[pnum];
	playertwp[0] = playertwp[pnum];
	camera_twp->pos = *GetCameraPosition(pnum);
	cartTopographicalCollision(tp, twp);
	playertp[0] = backup1;
	playertwp[0] = backup2;
	camera_twp->pos = backup3;
}

void cartCollisionM(taskwk* twp)
{
	cart_data->ignor_collision = FALSE;
	cart_data->flag |= CARTFLG_20;
	EntryColliList(twp);
}

void cartCharactorCollisionM(taskwk* twp, int pnum)
{
	if (cart_data->invincible_timer <= 0)
	{
		auto cwp = twp->cwp;
		auto ptwp = playertwp[pnum];

		switch (twp->mode)
		{
		case CARTMD_SARU:
			if ((cwp->flag & 1) != 0 && cwp->hit_cwp == ptwp->cwp)
			{
				cart_data->rest_timer = CartOtherParam.rest_time;
			}

			if (ptwp->smode == 18)
			{
				if ((twp->flag & Status_Hurt) != 0)
				{
					cart_data->vitality = 0;
					RumbleA(0, 0);
				}
			}
			else if ((twp->flag & Status_Hurt) != 0 && (ptwp->flag & Status_Ground) == 0)
			{
				CreateFlash(twp, 1.0f);
				SetVelocityP(pnum, 0.0f, 2.0f, 0.0f);
				VibShot(pnum, 0);
				twp->mode = CARTMD_WAIT;
			}

			cwp->info->damage = cwp->info->damage & ~0xF | 1;
			cartCollisionM(twp);
			break;
		case CARTMD_WAIT:
			cart_data->flag &= ~CARTFLG_4000;
			cwp->info->damage = cwp->info->damage & ~0xF | 4;
			cartCollisionM(twp);
			break;
		case CARTMD_WARP:
			ptwp->cwp->info->damage &= 0xFCu;
			ptwp->cwp->info->damage |= 0xCu;
			break;
		case CARTMD_CTRL:
			// If player is hurt (and not in TC)
			if (CurrentLevel != LevelIDs_TwinkleCircuit && ptwp->wtimer)
			{
				cart_data->vector.x *= 0.5f;
				cart_data->vector.y *= 0.5f;
				cart_data->vector.z *= 0.5f;

				if (GetNumRingM(pnum))
				{
					DamegeRingScatter(pnum);
				}
				else
				{
					KillHimP(pnum);
					cart_data->vitality = 0;
				}

				cart_data->invincible_timer = CartOtherParam.rest_time;
			}
			else
			{
				Angle diff = (twp->ang.y - (0x4000 - playermwp[pnum]->ang_aim.y));
				if (diff > 0x8000) diff = 0x10000 - diff;
				if (diff >= 0x2000)
				{
					ptwp->cwp->info->damage &= ~3;
					ptwp->cwp->info->damage &= ~0xC;
				}
				else
				{
					ptwp->cwp->info->damage = ~3 | 2;
					ptwp->cwp->info->damage = ~0xC | 4;
				}
			}
			break;
		}

		// Kill player if too far away from camera in TC
		if (twp->mode > 2 && cart_data->hamari_cnt > 0x258 && CurrentLevel == LevelIDs_TwinkleCircuit)
		{
			NJS_VECTOR pos = *GetCameraPosition(pnum);
			njSubVector(&pos, &ptwp->pos);
			if (njScalor(&pos) > 400.0f && hamariDist < 100.0f)
			{
				KillHimP(pnum);
				cart_data->vitality = 0;
			}
		}

		// Handle death
		if (cart_data->vitality <= 0)
		{
			cart_data->invincible_timer = 0;

			if (twp->mode == CARTMD_CTRL)
			{
				twp->cwp->info->attr &= ~0x10u;
				ptwp->cwp->info->a = cart_data->player_colli_r;
				// if (CurrentLevel == LevelIDs_TwinkleCircuit) CameraReleaseEventCamera();
			}

			dsPlay_oneshot(25, 0, 0, 0);
			twp->mode = CARTMD_EXPL;
		}
	}
	else
	{
		--cart_data->invincible_timer;
	}
}

static bool distCheck(taskwk* twp)
{
	NJS_VECTOR pos = *GetCameraPosition(twp->btimer);
	njSubVector(&pos, &twp->pos);
	return njScalor(&pos) < 500.0f;
}

void cartSELoopM(task* tp, int se_no)
{
	taskwk* twp = tp->twp;

	if (ssStageNumber == LevelIDs_TwinklePark)
	{
		if (distCheck(twp))
		{
			dsPlay_timer_v(cart_se_num[2 * se_no], (int)tp, 1, 0, 2, twp->pos.x, twp->pos.y, twp->pos.z);
		}
	}
	else
	{
		dsPlay_timer_v(cart_se_num[2 * se_no + 1], (int)tp, 1, 0, 2, twp->pos.x, twp->pos.y, twp->pos.z);
	}
}

void setupCartStageM(task* tp, taskwk* twp, int pnum)
{
	if (CurrentLevel == LevelIDs_TwinkleCircuit)
	{
		twp->pos = { 1513.0f, 9.0f, 74.0f };
		twp->ang.y = 0xC000;

		static const float dists[]
		{
			-15.0f,
			15.0f,
			-45.0f,
			45.0f
		};

		twp->pos.x += njCos(twp->ang.y) * dists[pnum];
		twp->pos.z += njSin(twp->ang.y) * dists[pnum];

		cart_data->last_pos = twp->pos;

		if (playertwp[pnum])
		{
			playertwp[pnum]->pos = twp->pos;
			playertwp[pnum]->pos.x -= 10.0f;
		}

		if (++cart_data->start_wait >= 10)
		{
			twp->mode = CARTMD_WARP;
			SetInputP(pnum, PL_OP_PLACEWITHCART);
		}
	}
}

void cartRideButtonCheckM(taskwk* twp, CART_PLAYER_PARAMETER* cartparam, int pnum)
{
	auto ptwp = playertwp[pnum];

	if ((perG[pnum].press & Buttons_A) != 0 && !ptwp->wtimer)
	{
		auto smode = ptwp->smode;
		if (smode != 50 && smode != 18 && (cart_data->last_player_flag & 1) != 0)
		{
			float p1[4] = { twp->pos.x, twp->pos.y, twp->pos.z, 20.0f };
			float p2[4] = { ptwp->pos.x, ptwp->pos.y, ptwp->pos.z, 20.0f };

			if (njCollisionCheckSS(p1, p2) && !(ptwp->flag & Status_HoldObject))
			{
				twp->mode = CARTMD_WARP;
				twp->btimer = pnum;
				cart_data->motion_timer = 0;
				cart_data->vitality = cartparam->max_vitality; // custom
				SetInputP(pnum, PL_OP_PLACEWITHCART);
			}
		}
	}

	cart_data->add_key.x = 0.0f;
	cart_data->add_key.y = 0.0f;
	cart_data->add_key.z = 0.0f;
	cart_data->last_player_flag = ptwp->flag;
}

static Angle calcAngle(Angle a, Angle b)
{
	Angle ang = SubAngle(a, b);
	return ang <= 0x8000 ? ang / 30 : (0x10000 - ang) / -30;
}

void cartSonicRidingCartM(task* tp, taskwk* twp, int pnum)
{
	auto pltwp = playertwp[pnum];

	if (cart_data->motion_timer == 0)
	{
		cart_data->tmp_angle[0] = calcAngle(pltwp->ang.x, twp->ang.x);
		cart_data->tmp_angle[1] = calcAngle(pltwp->ang.y, twp->ang.y);
		cart_data->tmp_angle[2] = calcAngle(pltwp->ang.z, twp->ang.z);
		cart_data->tmp_posi[0] = (twp->pos.x - pltwp->pos.x) * 0.033f;
		cart_data->tmp_posi[1] = 4.5f;
		cart_data->tmp_posi[2] = (twp->pos.z - pltwp->pos.z) * 0.033f;
	}

	if (cart_data->motion_timer <= 15 || twp->pos.y + 5.0f <= pltwp->pos.y)
	{
		cart_data->motion_timer += 1;
		pltwp->ang.x += cart_data->tmp_angle[0];
		pltwp->ang.y += cart_data->tmp_angle[1];
		pltwp->ang.z += cart_data->tmp_angle[2];
		pltwp->pos.x += cart_data->tmp_posi[0];
		pltwp->pos.y += cart_data->tmp_posi[1];
		pltwp->pos.z += cart_data->tmp_posi[2];
		cart_data->tmp_posi[1] -= 0.3f;
	}
	else if (fabs(pltwp->pos.x - twp->pos.x) <= 4.0f && fabs(pltwp->pos.z - twp->pos.z) <= 4.0f)
	{
		// Let's go
		twp->mode = CARTMD_CTRL;

		pltwp->ang.x = twp->ang.x;
		pltwp->ang.y = 0x4000 - twp->ang.y;
		pltwp->ang.z = twp->ang.z;
		pltwp->pos.x = twp->pos.x;
		pltwp->pos.y = twp->pos.y + 1.0f;
		pltwp->pos.z = twp->pos.z;

		cart_data->motion_timer = 0;
		cart_data->ring_timer = CartOtherParam.ring_sub_timer;

		// Change collision
		cart_data->player_colli_r = pltwp->cwp->info->a;
		pltwp->cwp->info->a = cci_cart[GetPlayerNumberM(pnum)].a;

		// Tell on which cart task the player is
		taskOfPlayerOn_m[pnum] = tp;

		CameraSetEventCamera_m(pnum, CAMMD_CART, CAMADJ_TIME);
	}
	else
	{
		twp->mode = CARTMD_WAIT;
		twp->cwp->info->attr &= ~0x10u;
		GetOutOfCartP(pnum, 0.0f, 0.0f, 0.0f);
		cart_data->ignor_collision = 30;
	}
}

void cartSpdControlSonicOnTheCartM(taskwk* twp, int pnum)
{
	// todo: rewrite
	auto backup1 = perG[0];
	auto backup2 = Rings;
	auto backup3 = player_no;
	Rings = GetNumRingM(pnum);
	perG[0] = perG[pnum];
	player_no = GetParamNumberM(pnum);
	cartSpdControlSonicOnTheCart(twp);
	player_no = backup3;
	perG[0] = backup1;
	Rings = backup2;
}

void cartTakeSonicM(taskwk* twp, int pnum)
{
	if (playerpwp[pnum]->item & Powerups_Dead)
	{
		twp->pos = playertwp[pnum]->pos;
		twp->ang.y = 0xC000;
		cart_data->vector = { 0.0f, 0.0f, 0.0f };
		return;
	}

	playertwp[pnum]->pos = twp->pos;
	playertwp[pnum]->ang.x = twp->ang.x;
	playertwp[pnum]->ang.y = 0x4000 - twp->ang.y;
	playertwp[pnum]->ang.z = twp->ang.z;
	pLockingOnTargetEnemy2(playermwp[pnum], playertwp[pnum], playerpwp[pnum]);

	auto param = GetCamAnyParam(pnum);
	if (param)
	{
		param->camAnyParamPos = twp->pos;
		param->camAnyParamAng = twp->ang;

		if (0 /* ego_flag == 2 */)
		{
			param->camAnyParamTgt.x = 0.0f;
		}
		else
		{
			param->camAnyParamTgt.x = (Float)(cart_data->flag & 0x10);
		}
	}
}

void cartCheckGoalM(taskwk* twp, int pnum)
{
	if (cartGoalFlagM[pnum] == true)
	{
		twp->mode = CARTMD_GOAL;
		cart_data->load_line = 1;
		cart_data->next_point = 1;
	}
}

void cartRunPassM(taskwk* twp, int pnum)
{
	auto pos1 = cart_load[16].load_data[cart_data->loop_pos];
	auto pos2 = cart_load[17].load_data[cart_data->loop_pos];

	auto pos = twp->pos;
	twp->pos.x = (pos2.x + pos1.x) * 0.5f - twp->pos.x + twp->pos.x;
	twp->pos.y = ((pos2.y + pos1.y) * 0.5f - twp->pos.y) * 0.5f + twp->pos.y;
	twp->pos.z = (pos2.z + pos1.z) * 0.5f - twp->pos.z + twp->pos.z;

	pos.x = twp->pos.x - pos.x;
	pos.y = twp->pos.y - pos.y;
	pos.z = twp->pos.z - pos.z;
	DirectionToRotation(&pos, &twp->ang.x, &twp->ang.y);

	cart_data->unstable.x = 0;
	cart_data->unstable.z = 0;
	++cart_data->loop_pos;

	if (cart_data->loop_pos == cart_load[16].point_num)
	{
		twp->mode = 4;
		cart_data->vector.x = pos.x;
		cart_data->vector.y = pos.y;
		cart_data->vector.z = pos.z;
		//SetCameraType(49, 5u);
	}
}

void EnemyCartM(task* tp)
{
	if (CurrentLevel != LevelIDs_TwinkleCircuit && CheckRange(tp))
	{
		return;
	}

	auto twp = tp->twp;
	cart_data = (ENEMY_CART_DATA*)tp->awp;
	auto pnum = twp->mode < 3 ? GetClosestPlayerNum(&twp->pos) : CART_PNUM(twp);
	player_no = GetPlayerNumberM(pnum);
	auto cartparam = &CartParameter[GetParamNumberM(pnum)];

	switch (twp->mode)
	{
	case CARTMD_INIT:
		cartInitM(tp, twp, cartparam, twp->btimer);
		break;
	case CARTMD_SARU:
		cart_data->flag |= 1u;
		cartSetVectorM(twp, pnum);
		cartShadowPos(twp);
		cartSpdForceOfNature(twp);
		cartThinkM(tp, twp, pnum);
		cartCharactorCollisionM(twp, pnum);
		cartTopographicalCollisionM(tp, twp, pnum);
		cartSELoopM(tp, 0);
		break;
	case CARTMD_WAIT:
		cart_data->flag &= ~1u;
		setupCartStageM(tp, twp, twp->btimer);
		cartSetVectorM(twp, pnum);
		cartRideButtonCheckM(twp, cartparam, pnum);
		cartShadowPos(twp);
		cartSpdForceOfNature(twp);
		cartCharactorCollisionM(twp, pnum);
		cartTopographicalCollisionM(tp, twp, pnum);
		break;
	case CARTMD_WARP:
		cart_data->flag &= ~1u;
		cartSetVectorM(twp, pnum);
		cartShadowPos(twp);
		cartSpdForceOfNature(twp);
		cartCharactorCollisionM(twp, pnum);
		cartTopographicalCollisionM(tp, twp, pnum);
		cartSonicRidingCartM(tp, twp, pnum);
		break;
	case CARTMD_CTRL:
		cart_data->flag &= ~1u;
		cartSetVectorM(twp, pnum);
		cartShadowPos(twp);
		cartSpdForceOfNature(twp);
		cartSpdControlSonicOnTheCartM(twp, pnum);
		cartCharactorCollisionM(twp, pnum);
		cartTopographicalCollisionM(tp, twp, pnum);
		cartTakeSonicM(twp, pnum);
		cartCheckPass(twp);
		cartCheckGoalM(twp, pnum);
		cartSELoopM(tp, 0);
		break;
	case CARTMD_PASS:
		cartSetVectorM(twp, pnum); // custom
		cartRunPassM(twp, pnum);
		cartTakeSonicM(twp, pnum);
		cartSELoopM(tp, 0);
		break;
	case CARTMD_GOAL:
		cart_data->flag &= ~1u;
		cartSetVectorM(twp, pnum);
		cartShadowPos(twp);
		cartSpdForceOfNature(twp);
		//cartThinkM(tp, twp, pnum); <- bot control once finish line passed, crashes in multiplayer
		cartCharactorCollisionM(twp, pnum);
		cartTopographicalCollisionM(tp, twp, pnum);
		cartTakeSonicM(twp, pnum);
		break;
	case CARTMD_EXPL:
		cart_data->flag &= ~1u;
		cartSetVectorM(twp, pnum); // custom
		tp->disp = cartDisplayExplosionM;
		cartExplosion(twp);
		// custom: cartExplosion doesn't work well for some reason, let's help it
		if (++cart_data->invincible_timer > CartOtherParam.dead_wait_time)
			twp->mode = CARTMD_DEST;
		break;
	case CARTMD_8:
		cart_data->flag &= ~1u;
		cartSetVectorM(twp, pnum); // custom
		cartShadowPos(twp);
		cartCharactorCollisionM(twp, pnum);
		cartTakeSonicM(twp, pnum);
		cartSELoop(0, tp);
		break;
	case CARTMD_9:
		cart_data->flag &= ~1u;
		cartSetVectorM(twp, pnum); // custom
		cartShadowPos(twp);
		cartCharactorCollisionM(twp, pnum);
		break;
	case CARTMD_10:
		cart_data->flag &= ~1u;
		cartSetVectorM(twp, pnum); // custom
		cartShadowPos(twp);
		cartSpdForceOfNature(twp);
		cartThinkM(tp, twp, pnum);
		cartCharactorCollisionM(twp, pnum);
		cartTopographicalCollisionM(tp, twp, pnum);
		cartTakeSonicM(twp, pnum);
		break;
	case CARTMD_DEST:
	default:
		// custom: condition to respawn cart in TP1
		if (tp->ocp && !(ssStageNumber == STAGE_TWINKLEPARK && ssActNumber == 0))
			DeadOut(tp);
		else
			FreeTask(tp);
		return;
	}

	tp->disp(tp);
	++cart_data->hamari_cnt;
}

void __cdecl EnemyCart_r(task* tp);
Trampoline EnemyCart_t(0x79A9E0, 0x79A9E5, EnemyCart_r);
void __cdecl EnemyCart_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		EnemyCartM(tp);
	}
	else
	{
		TARGET_STATIC(EnemyCart)(tp);
	}
}

static bool IsTaskPlayerTask(task* tp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertp[i] == tp)
		{
			return true;
		}
	}

	return false;
}

void __cdecl SetCartVelocity_r(task* tp, NJS_POINT3* spd);
Trampoline SetCartVelocity_t(0x79AF90, 0x79AF96, SetCartVelocity_r);
void __cdecl SetCartVelocity_r(task* tp, NJS_POINT3* spd)
{
	if (multiplayer::IsActive())
	{
		if (IsTaskPlayerTask(tp))
		{
			auto pnum = TASKWK_PLAYERID(tp->twp);
			auto ctp = taskOfPlayerOn_m[pnum];

			if (ctp)
			{
				auto twp = ctp->twp;
				cart_data = (ENEMY_CART_DATA*)ctp->awp;

				setCartDirection(twp, spd);
				twp->pos.x = cart_data->last_pos.x + spd->x;
				twp->pos.y = cart_data->last_pos.y + spd->y;
				twp->pos.z = cart_data->last_pos.z + spd->z;
				playertwp[pnum]->pos = twp->pos;

				cart_data->vector.x = spd->x;
				cart_data->vector.y = spd->y;
				cart_data->vector.z = spd->z;

				// Onion effects
			}
			else
			{
				SetVelocityP(pnum, spd->x, spd->y, spd->z);
			}
		}
		else if (tp->twp->id == 10)
		{
			auto twp = tp->twp;
			cart_data = (ENEMY_CART_DATA*)tp->awp;

			setCartDirection(twp, spd);
			twp->pos.x = cart_data->last_pos.x + spd->x;
			twp->pos.y = cart_data->last_pos.y + spd->y;
			twp->pos.z = cart_data->last_pos.z + spd->z;
			cart_data->vector.x = spd->x;
			cart_data->vector.y = spd->y;
			cart_data->vector.z = spd->z;
		}
	}
	else
	{
		TARGET_STATIC(SetCartVelocity)(tp, spd);
	}
}

void __cdecl CartGetOffPlayer_r(task* tp);
Trampoline CartGetOffPlayer_t(0x798C60, 0x798C65, CartGetOffPlayer_r);
void __cdecl CartGetOffPlayer_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto pnum = CART_PNUM(tp->twp);
		cart_data = (ENEMY_CART_DATA*)tp->awp;

		tp->twp->mode = CARTMD_9;
		tp->twp->cwp->info->attr &= ~0x10;
		taskOfPlayerOn_m[pnum] = nullptr;
		GetOutOfCartP(pnum, 0.0f, 2.0f, 2.0f);
		cart_data->ignor_collision = 30;
		playertwp[pnum]->cwp->info->a = cart_data->player_colli_r;

		if (ssStageNumber != STAGE_MG_CART)
			CameraReleaseEventCamera_m(pnum);
	}
	else
	{
		TARGET_STATIC(CartGetOffPlayer)(tp);
	}
}

void __cdecl SetCartPos_r(task* tp, NJS_POINT3* pos, Angle3* ang);
Trampoline SetCartPos_t(0x796C50, 0x796C57, SetCartPos_r);
void __cdecl SetCartPos_r(task* tp, NJS_POINT3* pos, Angle3* ang)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;
		auto pnum = CART_PNUM(tp->twp);

		if (twp->mode == CARTMD_8 || twp->mode == CARTMD_9)
		{
			twp->pos = *pos;
			taskOfPlayerOn_m[pnum]->twp->ang = *ang;
		}
	}
	else
	{
		return TARGET_STATIC(SetCartPos)(tp, pos, ang);
	}
}

void KillPlayerInKart(taskwk* data, playerwk* co2, char mode, uint16_t anm)
{
	if ((data->flag & Status_DoNextAction) != 0)
	{
		if (data->smode == 50)
		{
			data->mode = mode;
			co2->mj.reqaction = anm;
		}
	}
}
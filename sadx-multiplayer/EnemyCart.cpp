#include "pch.h"
#include "camera.h"

enum CARTMD
{
	CARTMD_INIT,
	CARTMD_SARU,
	CARTMD_WAIT,
	CARTMD_WARP,
	CARTMD_CTRL, // Player controlled
	CARTMD_5, // Player controlled
	CARTMD_6,
	CARTMD_EXPL,
	CARTMD_8,
	CARTMD_9,
	CARTMD_10,
	CARTMD_11
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

Characters GetPlayerNumberM(int pnum)
{
	return playertwp[pnum] ? (Characters)TASKWK_CHARID(playertwp[pnum]) : Characters_Sonic;
}

void cartDisplayM(task* tp)
{
	auto twp = tp->twp;

	if (dsCheckViewV(&twp->pos, 20.0f))
	{
		cart_data = (ENEMY_CART_DATA*)tp->awp;
		auto pnum = twp->btimer;
		auto cartparam = &CartParameter[GetPlayerNumberM(pnum)];

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
		njPushMatrix(0);
		njTranslate(0, twp->pos.x, twp->pos.y + cartOffset[cart_data->cart_type], twp->pos.z);
		if (twp->ang.z) njRotateZ(0, twp->ang.z);
		if (twp->ang.x) njRotateX(0, twp->ang.x);
		if (twp->ang.y) njRotateY(0, twp->ang.y + 0x4000);
		if (cart_data->unstable.z) njRotateZ(0, cart_data->unstable.z);
		if (cart_data->unstable.x) njRotateX(0, cart_data->unstable.x);

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
			___dsDrawObject(&object_sarucart_saru_body_saru_body);
		}

		njPopMatrixEx();

		NJS_VECTOR shadowpos = { twp->pos.x, cart_data->shadow_pos + 0.2f, twp->pos.z };
		DrawShadow_(&cart_data->shadow_ang, &shadowpos, cart_data->cart_type == 0 ? 1.2f : 1.5f);

		if (cart_data->vitality <= cartparam->smoke_vitality)
		{
			if (!(GetGlobalTime() % cart_data->vitality + 8))
			{
				CreateSmoke(&twp->pos, &_velo, 1.0);
			}
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

	cart_data->vitality = cartparam->max_vitality; // todo: refresh with player
	
	searchAnimationMaterial();

	twp->id = 10; // "I am a cart!"
	tp->disp = cartDisplayM;
}

void cartSetVectorM(taskwk* twp, int pnum)
{
	if (twp->mode == 4)
	{
		taskwk* pltwp = playertwp[pnum];

		twp->pos = playertwp[0]->pos;

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

void cartTopographicalCollisionM(task* tp, taskwk* twp)
{
	// todo: rewrite
	auto backup1 = playertp[0];
	auto backup2 = playertwp[0];
	auto backup3 = camera_twp->pos;
	playertp[0] = playertp[twp->btimer];
	playertwp[0] = playertwp[twp->btimer];
	camera_twp->pos = *GetCameraPosition(twp->btimer);
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

void setupCartStageM(task* tp, taskwk* twp)
{
	setupCartStage(tp);

	// place all players on the starting line
	if (twp->mode == CARTMD_WARP)
	{
		static const int dists[]
		{
			-10.0f,
			10.0f,
			-20.0f,
			20.0f
		};

		twp->pos.x += njCos(twp->ang.y + 0x4000) * dists[twp->btimer];
		twp->pos.z += njSin(twp->ang.y + 0x4000) * dists[twp->btimer];
	}													 
}

void cartRideButtonCheckM(taskwk* twp, int pnum)
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
				SetInputP(pnum, 18);
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

void cartSonicRidingCartM(taskwk* twp, int pnum)
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

		//taskOfPlayerOn = tp;
		//CameraSetEventCamera(49, 5u);
	}
	else
	{
		twp->mode = CARTMD_WAIT;
		twp->cwp->info->attr &= ~0x10u;
		GetOutOfCartP(pnum, 0.0f, 0.0f, 0.0f);
		cart_data->ignor_collision = 30;
	}
}

void EnemyCartM(task* tp)
{
	if (CheckRange(tp))
	{
		return;
	}

	auto twp = tp->twp;
	cart_data = (ENEMY_CART_DATA*)tp->awp;
	auto pnum = twp->mode < 2 ? GetTheNearestPlayerNumber(&twp->pos) : twp->btimer;
	player_no = GetPlayerNumberM(pnum);
	auto cartparam = &CartParameter[GetPlayerNumberM(pnum)];

	switch (twp->mode)
	{
	case CARTMD_INIT:
		cartInitM(tp, twp, cartparam, pnum);
		break;
	case CARTMD_SARU:
		cart_data->flag |= 1u;
		cartSetVectorM(twp, pnum);
		cartShadowPos(twp);
		cartSpdForceOfNature(twp);
		cartThinkM(tp, twp, pnum);
		cartCharactorCollisionM(twp, pnum);
		cartTopographicalCollisionM(tp, twp);
		cartSELoopM(tp, 0);
		break;
	case CARTMD_WAIT:
		cart_data->flag &= ~1u;
		setupCartStageM(tp, twp);
		cartSetVectorM(twp, pnum);
		cartRideButtonCheckM(twp, pnum);
		cartShadowPos(twp);
		cartSpdForceOfNature(twp);
		cartCharactorCollisionM(twp, pnum);
		cartTopographicalCollisionM(tp, twp);
		break;
	case CARTMD_WARP:
		cartSetVectorM(twp, pnum);
		cartShadowPos(twp);
		cartSpdForceOfNature(twp);
		cartCharactorCollisionM(twp, pnum);
		cartTopographicalCollisionM(tp, twp);
		cartSonicRidingCartM(twp, pnum);
		break;
	}

	++cart_data->hamari_cnt;
	tp->disp(tp);
}

void __cdecl EnemyCart_r(task* tp);
Trampoline EnemyCart_t(0x79A9E0, 0x79A9E5, EnemyCart_r);
void __cdecl EnemyCart_r(task* tp)
{
	if (IsMultiplayerEnabled())
	{
		EnemyCartM(tp);
	}
	else
	{
		TARGET_STATIC(EnemyCart)(tp);
	}
}
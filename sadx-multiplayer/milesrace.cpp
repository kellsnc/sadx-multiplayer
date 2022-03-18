#include "pch.h"
#include "multiplayer.h"
#include "milesrace.h"
#include "splitscreen.h"

DataPointer(float, AnalogRatio_High, 0x3C539E8);
DataArray(int, AnalogTbl, 0x7E4AC4, 4);

auto Casino_Init = GenerateUsercallWrapper<void (*)(taskwk* twp, taskwk* stwp, taskwk* mtwp)>(noret, 0x47C540, rESI, rEAX, rEDI);
auto MRaceVoiceCtrl = GenerateUsercallWrapper<void (*)(task* tp, __int16 new_m_path, __int16 new_s_path)>(noret, 0x47D1D0, rEAX, rDX, rCX);
auto MakeDirAngle = GenerateUsercallWrapper<int (*)(taskwk* stwp, sSonicCtrl* sonic_ctrl, taskwk* twp)>(rEAX, 0x47B9B0, rEBX, rESI, stack4);

static bool ChkEndSonicFakeJump_m(taskwk* stwp)
{
	auto pnum = TASKWK_PLAYERID(stwp);
	SetPositionP(pnum, stwp->pos.x + SonicPaboBuff.speed.x, stwp->pos.y + SonicPaboBuff.speed.y, stwp->pos.z + SonicPaboBuff.speed.z);

	SonicPaboBuff.speed.y -= SonicPaboBuff.gravity;

	if (SonicPaboBuff.speed.y < 0.0f)
	{
		CharColliOn(stwp);
	}

	auto stat = ChkParabolaEnd(&SonicPaboBuff);

	if (stat)
	{
		SetInputP(pnum, 24);
		if (stat == 2)
			SetPositionP(pnum, SonicPaboBuff.pos_end.x, SonicPaboBuff.pos_end.y, SonicPaboBuff.pos_end.z);
		return true;
	}
	else
	{
		return false;
	}
}

static void InitJumpSonicToPoint_m(taskwk* stwp, NJS_POINT3* pos, int time)
{
	auto pnum = TASKWK_PLAYERID(stwp);
	SonicCtrlBuff.path_flag_bak = SonicCtrlBuff.path_flag;
	SetInputP(pnum, 13);
	SonicPaboBuff.pos_start.x = stwp->pos.x;
	SonicPaboBuff.pos_start.y = stwp->pos.y;
	SonicPaboBuff.pos_start.z = stwp->pos.z;
	SonicPaboBuff.pos_end = *pos;
	SonicPaboBuff.gravity = playerpwp[pnum]->p.weight;
	SonicPaboBuff.time = time;
	MakeParabolaInitSpeed(&SonicPaboBuff);
	CharColliOff(stwp);
}

static __int16 SetSonicNextPath_m(taskwk* twp, taskwk* stwp, sSonicCtrl* sonic_ctrl, __int16 sonic_path)
{
	__int16 max_path = twp->counter.w[1];
	__int16 next_path = sonic_path + 1;

	bool jump = false;
	if (next_path < max_path && LOBYTE(sonic_ctrl->path_flag) != 18)
	{
		twp->smode = 0;
		twp->wtimer = 0;
		twp->value.w[0] = 0;

		if (sonic_ctrl->path_flag & 0x20000000)
		{
			Controllers[TASKWK_PLAYERID(stwp)].PressedButtons |= JumpButtons;
		}

		if (sonic_ctrl->path_flag & 0x10000000)
		{
			jump = true;
		}

		auto item = &PathTbl_Sonic[next_path];
		sonic_ctrl->now_path_pos = item->pos;
		sonic_ctrl->tgt_path_pos = item[1].pos;
		sonic_ctrl->path_flag = item[1].flag;
		sonic_ctrl->vec_snc_tgt.x = sonic_ctrl->tgt_path_pos.x - stwp->pos.x;
		sonic_ctrl->vec_snc_tgt.y = sonic_ctrl->tgt_path_pos.y - stwp->pos.y;
		sonic_ctrl->vec_snc_tgt.z = sonic_ctrl->tgt_path_pos.z - stwp->pos.z;
		sonic_ctrl->dist_snc_tgt = njScalor2(&sonic_ctrl->vec_snc_tgt);
		sonic_ctrl->pl_last_spd = 0.0;
		if (jump)
		{
			InitJumpSonicToPoint_m(stwp, &sonic_ctrl->tgt_path_pos, 60);
			sonic_ctrl->path_flag = 19;
		}
		return next_path;
	}
	return max_path;
}

static __int16 ChkSonicPathPos_m(taskwk* twp, taskwk* stwp, sSonicCtrl* sonic_ctrl)
{
	__int16 pathnum = twp->timer.w[1];
	auto pnum = TASKWK_PLAYERID(stwp);

	switch (LOBYTE(sonic_ctrl->path_flag))
	{
	case 0:
	case 6:
	case 13:
		VecTemp0.x = sonic_ctrl->tgt_path_pos.x - sonic_ctrl->now_path_pos.x;
		VecTemp0.y = sonic_ctrl->tgt_path_pos.y - sonic_ctrl->now_path_pos.y;
		VecTemp0.z = sonic_ctrl->tgt_path_pos.z - sonic_ctrl->now_path_pos.z;
		if (njInnerProduct(&sonic_ctrl->vec_snc_tgt, &VecTemp0) <= 0.001f || sonic_ctrl->dist_snc_tgt < 25.0f)
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
		break;
	case 1:
		VecTemp0.x = sonic_ctrl->tgt_path_pos.x - sonic_ctrl->now_path_pos.x;
		VecTemp0.y = 0.0f;
		VecTemp0.z = sonic_ctrl->tgt_path_pos.z - sonic_ctrl->now_path_pos.z;

		VecTemp1.x = sonic_ctrl->vec_snc_tgt.x;
		VecTemp1.y = 0.0f;
		VecTemp1.z = sonic_ctrl->vec_snc_tgt.z;

		if (njInnerProduct(&VecTemp1, &VecTemp0) <= 0.001f || sonic_ctrl->dist_snc_tgt < 25.0f)
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
		break;
	case 3:
	case 11:
		if (sonic_ctrl->dist_snc_tgt < 25.0f)
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
		break;
	case 4:
	case 5:
	case 14:
	case 15:
		VecTemp0.x = sonic_ctrl->tgt_path_pos.x - sonic_ctrl->now_path_pos.x;
		VecTemp0.y = sonic_ctrl->tgt_path_pos.y - sonic_ctrl->now_path_pos.y;
		VecTemp0.z = sonic_ctrl->tgt_path_pos.z - sonic_ctrl->now_path_pos.z;
		if (njInnerProduct(&sonic_ctrl->vec_snc_tgt, &VecTemp0) <= 0.0f)
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
		break;
	case 7:
		if (stwp->smode == 4 || stwp->smode == 3 || stwp->smode == 25)
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
		break;
	case 8:
		if (stwp->flag & 0x1000)
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
		break;
	case 9:
		if (((sonic_ctrl->vec_snc_tgt.x * sonic_ctrl->vec_snc_tgt.x)
			+ (sonic_ctrl->vec_snc_tgt.z * sonic_ctrl->vec_snc_tgt.z)) < 25.0f)
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
		break;
	case 10:
		if (stwp->smode == 31)
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
		break;
	case 12:
		if (stwp->flag & (Status_Ground | Status_OnColli))
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
		break;
	case 16:
		if (twp->timer.w[0] > pathnum || MRaceLevel || ((sonic_ctrl->path_flag & 0xF00000) != 0 && twp->value.w[0] > 120 * ((sonic_ctrl->path_flag & 0xF00000) >> 20)))
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		}
	case 17:
		VecTemp0.x = sonic_ctrl->tgt_path_pos.x - sonic_ctrl->now_path_pos.x;
		VecTemp0.y = sonic_ctrl->tgt_path_pos.y - sonic_ctrl->now_path_pos.y;
		VecTemp0.z = sonic_ctrl->tgt_path_pos.z - sonic_ctrl->now_path_pos.z;
		if (njInnerProduct(&sonic_ctrl->vec_snc_tgt, &VecTemp0) <= 0.001 || sonic_ctrl->dist_snc_tgt < 25.0)
		{
			pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
			if ((sonic_ctrl->path_flag & 0x8000000) != 0)
			{
				SetInputP(pnum, 24);
				PadReadOffP(1u);
			}
		}
		break;
	}

	if (pathnum == twp->timer.w[1])
	{
		auto v15 = twp->value.w[0];
		twp->value.w[0] = v15 + 1;
		if (sonic_ctrl->path_flag == 17 && v15 > 180)
		{
			if (twp->timer.w[0] - twp->timer.w[1] >= 20)
			{
				pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
				stwp->pos.x = sonic_ctrl->now_path_pos.x;
				stwp->pos.y = sonic_ctrl->now_path_pos.y;
				stwp->pos.z = sonic_ctrl->now_path_pos.z;
			}
			else
			{
				perG[pnum].press |= JumpButtons;
			}
		}
	}

	if (sonic_ctrl->dist_snc_tgt > 1000000.0f || twp->timer.w[0] - pathnum > (MRaceLevel != 1 ? 40 : 20))
	{
		pathnum = SetSonicNextPath_m(twp, stwp, sonic_ctrl, pathnum);
		SetPositionP(pnum, sonic_ctrl->now_path_pos.x, sonic_ctrl->now_path_pos.y + 5.0f, sonic_ctrl->now_path_pos.z);
		if (sonic_ctrl->path_flag != 17)
		{
			SetInputP(pnum, 24);
		}
	}

	return pathnum;
}

static float MakeStrokeWithDist_m(NJS_POINT3* vec, float range, float ratio)
{
	auto r = min(sqrtf((vec->x * vec->x) + (vec->z * vec->z)), range);
	return max(r / range * ratio, 0.2f);
}

static float MakeStroke_m(sSonicCtrl* sonic_ctrl)
{
	if (sonic_ctrl->path_flag & 0x4000000)
	{
		return 1.0f;
	}
	else if (sonic_ctrl->path_flag & 0x2000000)
	{
		return 0.8f;
	}
	else
	{
		return AnalogRatio_High;
	}
}

static bool ChkSonicStack_m(taskwk* stwp)
{
	if (SonicCtrlBuff.path_flag & 0x1000000)
	{
		NJS_POINT3 v;
		GetPlayerPosition(TASKWK_PLAYERID(stwp), 0x78u, &v, 0);
		VecTemp0.x = stwp->pos.x - v.x;
		VecTemp0.y = stwp->pos.y - v.y;
		VecTemp0.z = stwp->pos.z - v.z;

		if (njScalor2(&VecTemp0) < 25.0f)
		{
			return true;
		}
	}

	return false;
}

static void SonicControl_m(taskwk* twp, taskwk* stwp, sSonicCtrl* sonic_ctrl)
{
	__int16 pathnum = twp->timer.w[1];
	auto pnum = TASKWK_PLAYERID(stwp);
	auto spwp = playerpwp[pnum];

	switch (LOBYTE(sonic_ctrl->path_flag))
	{
	case 0:
	case 1:
	{
		input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
		input_dataG[pnum].stroke = MakeStroke_m(sonic_ctrl);

		auto slope = -njInnerProduct(&Gravity, &spwp->floor_normal);
		if (njCos(0x2000) < slope && slope < njCos(4551))
		{
			if (spwp->spd.x < sonic_ctrl->pl_last_spd && spwp->spd.x < 0.1f && njScalor(&spwp->spd) < 0.1f)
			{
				sonic_ctrl->path_flag_bak = sonic_ctrl->path_flag;
				sonic_ctrl->path_flag = 13;
				perG[pnum].press |= AttackButtons;
				twp->value.w[1] = 0;
			}
		}

		if (ChkSonicStack_m(stwp))
		{
			InitJumpSonicToPoint_m(stwp, &sonic_ctrl->tgt_path_pos, 60);
			sonic_ctrl->path_flag = 19;
		}

		break;
	}
	case 4:
	case 11:
		input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
		input_dataG[pnum].stroke = 1.0f;

		if (ChkSonicStack_m(stwp))
		{
			InitJumpSonicToPoint_m(stwp, &sonic_ctrl->tgt_path_pos, 60);
			sonic_ctrl->path_flag = 19;
		}

		break;
	case 6:
		input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
		input_dataG[pnum].stroke = MakeStrokeWithDist_m(&sonic_ctrl->vec_snc_tgt, 40.0f, 0.6f);
		break;
	case 7:
	case 8:
		input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
		input_dataG[pnum].stroke = MakeStrokeWithDist_m(&sonic_ctrl->vec_snc_tgt, 40.0f, 1.0f);
		
		if (ChkSonicStack_m(stwp) && stwp->flag & (Status_Ground | Status_OnColli))
		{
			InitJumpSonicToPoint_m(stwp, &sonic_ctrl->tgt_path_pos, 60);
			sonic_ctrl->path_flag = 19;
		}
		break;
	case 9:
	{
		auto idk = AnalogTbl[4 * ((sonic_ctrl->path_flag >> 30) & 3)];
		perG[pnum].x1 = HIWORD(idk);
		perG[pnum].y1 = LOWORD(idk);
		break;
	}
	case 10:
		input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
		input_dataG[pnum].stroke = MakeStrokeWithDist_m(&sonic_ctrl->vec_snc_tgt, 40.0f, AnalogRatio_High);
		if (((sonic_ctrl->vec_snc_tgt.x * sonic_ctrl->vec_snc_tgt.x) + (sonic_ctrl->vec_snc_tgt.z * sonic_ctrl->vec_snc_tgt.z)) < 25.0f && (stwp->flag & (Status_Ground | Status_OnColli)))
		{
			perG[pnum].press |= JumpButtons;
		}
		break;
	case 13:
		input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
		input_dataG[pnum].stroke = 1.0f;
		if (++twp->value.w[1] <= 60i16)
			perG[pnum].on |= AttackButtons;
		else
			sonic_ctrl->path_flag = sonic_ctrl->path_flag_bak;
		break;
	case 14u:
	{
		if (stwp->flag & 0x2000)
		{
			auto idk = AnalogTbl[4 * ((sonic_ctrl->path_flag >> 30) & 3)];
			perG[pnum].x1 = HIWORD(idk);
			perG[pnum].y1 = LOWORD(idk);
		}
		else
		{
			input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
			input_dataG[pnum].stroke = MakeStroke_m(sonic_ctrl);
		}
		break;
	}
	case 15:
		if (stwp->flag & (Status_Ground | Status_OnColli))
		{
			input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
			input_dataG[pnum].stroke = MakeStrokeWithDist_m(&sonic_ctrl->vec_snc_tgt, 40.0f, 0.6f);
		}
	case 16:
		if (twp->smode)
		{
			if (twp->smode == 1i8)
			{
				if (++twp->wtimer > 300i16)
				{
					ForcePlayerAction(pnum, 34);
					twp->wtimer = 0i16;
				}
			}
		}
		else
		{
			input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
			input_dataG[pnum].stroke = MakeStrokeWithDist_m(&sonic_ctrl->vec_snc_tgt, 40.0f, 0.6f);
			VecTemp0.x = sonic_ctrl->tgt_path_pos.x - sonic_ctrl->now_path_pos.x;
			VecTemp0.y = sonic_ctrl->tgt_path_pos.y - sonic_ctrl->now_path_pos.y;
			VecTemp0.z = sonic_ctrl->tgt_path_pos.z - sonic_ctrl->now_path_pos.z;
			if (njInnerProduct(&sonic_ctrl->vec_snc_tgt, &VecTemp0) <= 0.0f || sonic_ctrl->dist_snc_tgt < 25.0f)
			{
				twp->smode = 1i8;
				SetInputP(pnum, 34);
			}
		}
		break;
	case 17:
		input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
		input_dataG[pnum].stroke = MakeStroke_m(sonic_ctrl);

		if (MRaceLevel)
		{
			VecTemp0.x = sonic_ctrl->vec_snc_tgt.x;
			VecTemp0.y = 0.0f;
			VecTemp0.z = sonic_ctrl->vec_snc_tgt.z;
			njUnitVector(&VecTemp0);
			SetAccelerationP(pnum, (VecTemp0.x * 0.05f), -0.05f, VecTemp0.z * 0.05f);
		}
		else if (twp->timer.w[0] - twp->timer.w[1] > 0 && twp->timer.w[1] < 210)
		{
			VecTemp0.x = sonic_ctrl->vec_snc_tgt.x;
			VecTemp0.y = 0.0f;
			VecTemp0.z = sonic_ctrl->vec_snc_tgt.z;
			njUnitVector(&VecTemp0);
			float v24;
			if (twp->timer.w[0] - twp->timer.w[1] <= 6)
			{
				if (twp->timer.w[0] - twp->timer.w[1] <= 3)
					SetAccelerationP(pnum, VecTemp0.x * 0.05f, -0.05f, VecTemp0.z * 0.05f);
				else
					SetAccelerationP(pnum, VecTemp0.x * 0.1f, -0.1f, VecTemp0.z * 0.1f);
			}
			else
			{
				SetAccelerationP(pnum, VecTemp0.x * 0.2f, -0.2f, VecTemp0.z * 0.2f);
			}
		}
		break;
	case 18:
		switch (twp->smode)
		{
		case 0i8:
			if (MRaceResult)
			{
				twp->smode = 5i8;
			}
			else
			{
				twp->smode = 1i8;
				perG[pnum].press |= JumpButtons;
				sonic_ctrl->jump_cnt = 0;
			}
			break;
		case 1i8:
			input_dataG[pnum].angle = MakeDirAngle(stwp, sonic_ctrl, twp);
			input_dataG[pnum].stroke = MakeStrokeWithDist_m(&sonic_ctrl->vec_snc_tgt, 40.0f, 0.6f);
			perG[pnum].on |= JumpButtons;

			if (stwp->flag & (Status_Ground | Status_OnColli))
			{
				// func here
			}

			twp->smode = 2i8;
			twp->wtimer = 0i16;
			break;
		case 2i8:
			if (MRaceResult)
			{
				if (playertwp[0])
				{
					VecTemp0.x = stwp->pos.x - playertwp[0]->pos.x;
					VecTemp0.y = stwp->pos.y - playertwp[0]->pos.y;
					VecTemp0.z = stwp->pos.z - playertwp[0]->pos.z;
					if (njScalor2(&VecTemp0) < 25.0f)
						SetAccelerationP(pnum, VecTemp0.x, VecTemp0.y, VecTemp0.z);
				}

				return;
			}

			if (++twp->wtimer > 32)
			{
				twp->smode = 1i8;
				perG[pnum].press |= JumpButtons;
				if (++sonic_ctrl->jump_cnt > 3)
				{
					twp->smode = 3i8;
					VecTemp0.x = sonic_ctrl->tgt_path_pos.x;
					VecTemp0.y = sonic_ctrl->tgt_path_pos.y + 45.0f;
					VecTemp0.z = sonic_ctrl->tgt_path_pos.z;
					InitJumpSonicToPoint_m(stwp, &VecTemp0, 60);
				}
			}
			break;
		case 3i8:
			if (ChkEndSonicFakeJump_m(stwp))
				twp->smode = 4i8;
			break;
		case 4i8:
			if (stwp->flag & (Status_Ground | Status_OnColli))
			{
				// func here
			}

			twp->smode = 2i8;
			twp->wtimer = 0i16;
			break;
		}
		break;
	case 19:
		if (ChkEndSonicFakeJump_m(stwp))
			sonic_ctrl->path_flag = sonic_ctrl->path_flag_bak;
		break;
	}
}

static __int16 GetNearestPath_m(NJS_POINT3* pos, sMRacePath* path_tbl, __int16 max_path)
{
	float max = 100000000.0f;
	__int16 num = 0;

	for (__int16 i = 0; i < max_path; ++i)
	{
		auto& item = path_tbl[i];

		VecTemp0.x = pos->x - item.pos.x;
		VecTemp0.y = pos->y - item.pos.y;
		VecTemp0.z = pos->z - item.pos.z;

		auto dist = njScalor2(&VecTemp0);
		if (dist < max)
		{
			max = dist;
			num = i;
		}
	}

	return num;
}

static void Windy_Nomal_m(task* tp, taskwk* stwp, taskwk* mtwp)
{
	auto twp = tp->twp;

	SonicCtrlBuff.vec_snc_tgt.x = SonicCtrlBuff.tgt_path_pos.x - stwp->cwp->info->center.x;
	SonicCtrlBuff.vec_snc_tgt.y = SonicCtrlBuff.tgt_path_pos.y - stwp->cwp->info->center.y;
	SonicCtrlBuff.vec_snc_tgt.z = SonicCtrlBuff.tgt_path_pos.z - stwp->cwp->info->center.z;
	SonicCtrlBuff.dist_snc_tgt = njScalor2(&SonicCtrlBuff.vec_snc_tgt);

	twp->timer.w[0] = GetNearestPath_m(&mtwp->pos, PathTbl_Miles, twp->counter.w[0]);
	twp->timer.w[1] = ChkSonicPathPos_m(twp, stwp, &SonicCtrlBuff);

	SonicControl_m(twp, stwp, &SonicCtrlBuff);
	MRaceVoiceCtrl(tp, twp->timer.w[1], twp->timer.w[0]);
}

static void Icecap_Init_m(task* tp, taskwk* stwp, taskwk* mtwp)
{
	sMRacePath* pathTbl = PathTbl_Sonic;
	tp->twp->mode = 1;
	ForcePlayerAction(TASKWK_PLAYERID(stwp), 44);
	stwp->ang = mtwp->ang;
	MiscEntityVector.x = 0.0f;
	MiscEntityVector.y = 1.0f;
	MiscEntityVector.z = 10.0f;
	PConvertVector_P2G(mtwp, &MiscEntityVector);
	stwp->pos.x = MiscEntityVector.x + mtwp->pos.x;
	stwp->pos.y = MiscEntityVector.y + mtwp->pos.y;
	stwp->pos.z = MiscEntityVector.z + mtwp->pos.z;
	ObjectMaster* board = LoadObject((LoadObj)(LoadObj_Data1 | LoadObj_Data2), 2, Snowboard_Sonic_Load);
	board->Data1->CharIndex = TASKWK_PLAYERID(stwp);
	memset(&SonicCtrlBuff, 0, 60u);

	sMRacePath** v7 = &PathTbl_Sonic;
	int max = 15;
	do
	{
		*++v7 = 0;
		--max;
	} while (max);

	SonicCtrlBuff.now_path_pos.x = pathTbl->pos.x;
	SonicCtrlBuff.now_path_pos.y = pathTbl->pos.y;
	SonicCtrlBuff.now_path_pos.z = pathTbl->pos.z;
	SonicCtrlBuff.tgt_path_pos = pathTbl[1].pos;
	SonicCtrlBuff.path_flag = pathTbl[1].flag;
}

static void __cdecl DispMilesMeter2P_r(task* tp)
{
	if (SplitScreen::IsActive())
	{
		if (SplitScreen::GetCurrentScreenNum() == 0)
		{
			SplitScreen::ChangeViewPort(-1);
			late_SetFunc((void(__cdecl*)(void*))late_DispMilesMeter2P, tp, 22046.5f, QueuedModelFlagsB_EnableZWrite);
			SplitScreen::ChangeViewPort(0);
		}
	}
	else
	{
		late_SetFunc((void(__cdecl*)(void*))late_DispMilesMeter2P, tp, 22046.5f, QueuedModelFlagsB_EnableZWrite);
	}
}

static void __cdecl Sonic2PControl_r(task* tp)
{
	auto twp = tp->twp;
	auto pnum = twp->btimer;
	auto stwp = playertwp[pnum];
	auto mtwp = playertwp[0];

	if (!stwp || !mtwp)
	{
		FreeTask(tp);
		return;
	}

	auto colInfo = stwp->cwp;
	for (int i = 0; i < colInfo->nbInfo; ++i)
	{
		colInfo->info[i].damage &= 0xDFu;
	}

	if (twp->mode)
	{
		Windy_Nomal_m(tp, stwp, mtwp);
	}
	else
	{
		if (AICourse == 3)
		{
			Icecap_Init_m(tp, stwp, mtwp);
		}
		else
		{
			Casino_Init(twp, stwp, mtwp);
		}
	}

	tp->disp(tp);
}

static void LoadNPCSonicTask(int num)
{
	auto tp = CreateElementalTask(0xAu, 0, InitSonic2PControl);
	tp->twp->btimer = num;

	tp = CreateElementalTask(7u, 1, SonicTheHedgehog);
	TASKWK_CHARID(tp->twp) = Characters_Sonic;
	TASKWK_PLAYERID(tp->twp) = num;
	tp->twp->id = 2;

	PadReadOffP(num);
}

void Set_NPC_Sonic_m(int num)
{
	MRaceResult = 0;
	MRaceLevel = GetMRaceLevel();

	switch (ssStageNumber)
	{
	case LevelIDs_WindyValley:
		if (CurrentAct == 2)
		{
			MRaceStageNumber = 0;
			LoadNPCSonicTask(num);
		}
		break;
	case LevelIDs_SpeedHighway:
		MRaceStageNumber = 1;
		CreateElementalTask(0xAu, 0, InitMoble2PControl);
		break;
	case LevelIDs_SkyDeck:
		if (CurrentAct == 0)
		{
			MRaceStageNumber = 2;
			LoadNPCSonicTask(num);
		}
		break;
	case LevelIDs_IceCap:
		if (CurrentAct == 2)
		{
			MRaceStageNumber = 3;
			LoadNPCSonicTask(num);
		}
		break;
	case LevelIDs_Casinopolis:
		if (CurrentAct == 1)
		{
			MRaceStageNumber = 4;
			LoadNPCSonicTask(num);
		}
		break;
	}
}

void InitMilesRace()
{
	WriteJump((void*)0x47D640, Sonic2PControl_r);
	WriteJump((void*)0x47D2E0, DispMilesMeter2P_r);
}
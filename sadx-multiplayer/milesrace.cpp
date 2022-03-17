#include "pch.h"
#include "multiplayer.h"
#include "milesrace.h"
#include "splitscreen.h"

DataPointer(sSonicCtrl, SonicCtrlBuff, 0x3C539F8);
DataPointer(sMRacePath*, PathTbl_Sonic, 0x03C539F4);
DataPointer(sMRacePath*, PathTbl_Miles, 0x3C53A64);

auto Casino_Init = GenerateUsercallWrapper<void (*)(taskwk* twp, taskwk* stwp, taskwk* mtwp)>(noret, 0x47C540, rESI, rEAX, rEDI);
auto GetNearestPath = GenerateUsercallWrapper<__int16 (*)(sMRacePath* path_tbl, __int16 maxpath, NJS_POINT3* pos)>(noret, 0x47B7F0, rECX, rBX, stack4);
auto ChkSonicPathPos = GenerateUsercallWrapper<__int16 (*)(taskwk* stwp, sSonicCtrl* sonic_ctrl, taskwk* twp)>(noret, 0x47C6A0, rEBX, rESI, stack4);
auto SonicControl = GenerateUsercallWrapper<void (*)(taskwk* stwp, taskwk* twp, sSonicCtrl* sonic_ctrl)>(noret, 0x47C9F0, rEAX, stack4, stack4);
auto MRaceVoiceCtrl = GenerateUsercallWrapper<void (*)(task* tp, __int16 new_m_path, __int16 new_s_path)>(noret, 0x47D1D0, rEAX, rDX, rCX);

static void Windy_Nomal_m(task* tp, taskwk* stwp, taskwk* mtwp)
{
	auto twp = tp->twp;

	SonicCtrlBuff.vec_snc_tgt.x = SonicCtrlBuff.tgt_path_pos.x - stwp->cwp->info->center.x;
	SonicCtrlBuff.vec_snc_tgt.y = SonicCtrlBuff.tgt_path_pos.y - stwp->cwp->info->center.y;
	SonicCtrlBuff.vec_snc_tgt.z = SonicCtrlBuff.tgt_path_pos.z - stwp->cwp->info->center.z;
	SonicCtrlBuff.dist_snc_tgt = njScalor2(&SonicCtrlBuff.vec_snc_tgt);

	twp->timer.w[0] = GetNearestPath(PathTbl_Miles, twp->counter.w[0], &mtwp->pos);
	twp->timer.w[1] = ChkSonicPathPos(stwp, &SonicCtrlBuff, twp); // <-- rewrite

	SonicControl(stwp, twp, &SonicCtrlBuff); // <-- rewrite
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
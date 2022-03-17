#include "pch.h"
#include "multiplayer.h"
#include "milesrace.h"

DataPointer(sSonicCtrl, SonicCtrlBuff, 0x3C539F8);
DataPointer(sMRacePath*, PathTbl_Sonic, 0x03C539F4);
DataPointer(sMRacePath*, PathTbl_Miles, 0x3C53A64);

auto Casino_Init = GenerateUsercallWrapper<void (*)(taskwk* data, taskwk* sonicData, taskwk* milesData)>(noret, 0x47C540, rEAX, rEDI, rESI);
FunctionPointer(void, Windy_Normal, (task* tp, taskwk* stwp, taskwk* mtwp), 0x47D3A0);

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

static void __cdecl Sonic2PAI_Main_r(task* tp)
{
	auto twp = tp->twp;
	auto pnum = twp->btimer;
	auto AIptr = playertwp[pnum];
	auto P1ptr = playertwp[0];

	if (!AIptr || !P1ptr)
	{
		FreeTask(tp);
		return;
	}

	auto colInfo = AIptr->cwp;
	for (int i = 0; i < colInfo->nbInfo; ++i)
	{
		colInfo->info[i].damage &= 0xDFu;
	}

	switch (AICourse) {
		case Levels2P_WindyValley:
			if (twp->mode)
			{
				Windy_Normal(tp, AIptr, P1ptr);
			}
			else
			{
				Casino_Init(twp, AIptr, P1ptr);
			}
			break;
		case Levels2P_SpeedHighway:
			break;
		case Levels2P_SkyDeck:
			if (twp->mode)
			{
				Windy_Normal(tp, AIptr, P1ptr);
			}
			else
			{
				Casino_Init(twp, AIptr, P1ptr);
			}
			break;
		case Levels2P_IceCap:
			if (twp->mode)
			{
				Windy_Normal(tp, AIptr, P1ptr);
			}
			else
			{
				Icecap_Init_m(tp, AIptr, P1ptr);
			}
			break;
		case Levels2P_Casinopolis:
			if (twp->mode)
			{
				Windy_Normal(tp, AIptr, P1ptr);
			}
			else
			{
				Casino_Init(twp, AIptr, P1ptr);
			}
			break;
		default:
			FreeTask(tp);
			return;
	}

	late_SetFunc((void(__cdecl*)(void*))late_DispMilesMeter2P, tp, 22046.5f, QueuedModelFlagsB_EnableZWrite);
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
	WriteJump(Sonic2PAI_Main, Sonic2PAI_Main_r);
}
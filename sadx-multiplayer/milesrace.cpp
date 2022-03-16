#include "pch.h"
#include "multiplayer.h"
#include "milesrace.h"

FunctionPointer(int, GetMRaceLevel, (), 0x47C200);
DataPointer(int, MRaceLevel, 0x3C53AB8);
DataPointer(int, MRaceStageNumber, 0x3C539EC);
TaskFunc(InitMoble2PControl, 0x47D8C0);
TaskFunc(InitSonic2PControl, 0x47D820);



struct sMRacePath
{
	int flag;
	NJS_POINT3 pos;
};

struct sSonicCtrl
{
	NJS_POINT3 now_path_pos;
	NJS_POINT3 tgt_path_pos;
	NJS_POINT3 vec_snc_tgt;
	float dist_snc_tgt;
	int path_flag;
	int path_flag_bak;
	int last_ang;
	float pl_last_spd;
	int jump_cnt;
};


DataPointer(sSonicCtrl, SonicCtrlBuff, 0x3C539F8);
DataPointer(sMRacePath*, PathTbl_Sonic, 0x03C539F4);
DataPointer(sMRacePath*, PathTbl_Miles, 0x3C53A64);

auto GetNearestPath = GenerateUsercallWrapper<int (*)(sMRacePath* path_tbl, __int16 max_path, NJS_POINT3* pos)>(rAX, 0x47B7F0, rECX, rBX, noret);

static void SonicAI_Casino_Init(taskwk* twp, taskwk* stwp, taskwk* mtwp)
{
	sMRacePath* v6; // r4
	__int16 v7; // r3
	__int16 v8; // r4
	sMRacePath* v9; // r10
	sMRacePath** v10; // r8
	sMRacePath* v11; // r10
	int v12; // ctr

	twp->mode = 1;
	stwp->ang = mtwp->ang;
	MiscEntityVector.y = 1.0;
	MiscEntityVector.z = 10.0;
	MiscEntityVector.x = 0.0;
	PConvertVector_P2G(mtwp, &MiscEntityVector);
	stwp->pos.x = mtwp->pos.x + MiscEntityVector.x;
	v6 = PathTbl_Miles;
	stwp->pos.y = mtwp->pos.y + MiscEntityVector.y;
	stwp->pos.z = mtwp->pos.z + MiscEntityVector.z;
	twp->timer.w[0] = GetNearestPath(v6, twp->counter.w[0], &mtwp->pos);
	v7 = GetNearestPath(PathTbl_Sonic, twp->counter.w[1], &stwp->pos);
	v8 = twp->timer.w[0];
	v9 = PathTbl_Sonic;
	twp->timer.w[1] = v7;
	v10 = &PathTbl_Sonic;
	v11 = &v9[v8];
	v12 = 15;
	do
	{
		*++v10 = 0;
		--v12;
	} while (v12);
	SonicCtrlBuff.now_path_pos.x = v11->pos.x;
	SonicCtrlBuff.now_path_pos.y = v11->pos.y;
	SonicCtrlBuff.now_path_pos.z = v11->pos.z;
	SonicCtrlBuff.tgt_path_pos = v11[1].pos;
	SonicCtrlBuff.path_flag = v11[1].flag;
}

static void SonicAI_IceCapInit(EntityData1* data, EntityData1* sonicData, EntityData1* milesData)
{
	sMRacePath* pathTbl; 
	float v5; 
	int max;

	pathTbl = PathTbl_Sonic;
	data->Action = 1;
	ForcePlayerAction(sonicData->CharIndex, 44);
	sonicData->Rotation = milesData->Rotation;
	MiscEntityVector.x = 0.0;
	MiscEntityVector.y = 1.0;
	MiscEntityVector.z = 10.0;
	PConvertVector_P2G((taskwk*)milesData, &MiscEntityVector);
	sonicData->Position.x = MiscEntityVector.x + milesData->Position.x;
	sonicData->Position.y = MiscEntityVector.y + milesData->Position.y;
	sonicData->Position.z = MiscEntityVector.z + milesData->Position.z;
	ObjectMaster* board = LoadObject((LoadObj)(LoadObj_Data1 | LoadObj_Data2), 2, Snowboard_Sonic_Load);
	board->Data1->CharIndex = sonicData->CharIndex;
	memset(&SonicCtrlBuff, 0, 60u);
	sMRacePath** v7 = &PathTbl_Sonic;
	max = 15;
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

static void __cdecl Sonic2PAI_Main_r(ObjectMaster* task)
{
	EntityData1* AIptr;
	EntityData1* data;
	EntityData1* P1ptr; 
	CollisionInfo* colInfo;
	int colMax;
	int colCount;
	int sonicAction;
	data = task->Data1;
	char pnum = data->CharIndex;
	AIptr = EntityData1Ptrs[pnum];
	P1ptr = EntityData1Ptrs[0];

	if (!AIptr || !P1ptr) {
		CheckThingButThenDeleteObject(task);
		return;
	}

	colInfo = EntityData1Ptrs[pnum]->CollisionInfo;
	colMax = 0;

	if (colInfo->nbInfo)
	{
		colCount = 0;
		do
		{
			colInfo->CollisionArray[colCount].damage &= 0xDFu;
			colInfo = AIptr->CollisionInfo;
			++colMax;
			++colCount;
		} while (colMax < colInfo->nbInfo);
	}

	sonicAction = data->Action;

	switch (AICourse)
	{
		case Levels2P_WindyValley:
			if (!data->Action)
			{
				goto LABEL_17;
			}
			goto LABEL_13;
		case Levels2P_SpeedHighway:
			break;
		case Levels2P_SkyDeck:
			if (data->Action)
			{
				goto LABEL_13;
			}
			goto LABEL_17;
		case Levels2P_IceCap:
			if (data->Action)
			{
				goto LABEL_13;
			}
			SonicAI_IceCapInit(data, AIptr, P1ptr);
			break;
		case Levels2P_Casinopolis:

			if (data->Action)
			{
			LABEL_13:
				if (sonicAction == 1)
				{
					//SonicAI_Casino_Init(task AIptr, P1ptr);
				}
			}
			else
			{
			LABEL_17:
				SonicAI_Casino_Init((taskwk*)data, (taskwk*)AIptr, (taskwk*)P1ptr);
			}
			break;
		default:
			CheckThingButThenDeleteObject(task);
			return;
	}

	late_SetFunc(
		(void(__cdecl*)(void*))TailsVS_DrawBarThing,
		task,
		22046.5,
		QueuedModelFlagsB_EnableZWrite);
	
}

static void LoadNPCSonicTask(int num)
{
	auto tp = CreateElementalTask(0xAu, 0, InitSonic2PControl);
	TASKWK_CHARID(tp->twp) = Characters_Sonic;
	TASKWK_PLAYERID(tp->twp) = num;

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

}
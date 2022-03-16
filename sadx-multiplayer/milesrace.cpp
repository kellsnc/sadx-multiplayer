#include "pch.h"
#include "multiplayer.h"
#include "milesrace.h"

FunctionPointer(int, GetMRaceLevel, (), 0x47C200);
DataPointer(int, MRaceLevel, 0x3C53AB8);
DataPointer(int, MRaceStageNumber, 0x3C539EC);
TaskFunc(InitMoble2PControl, 0x47D8C0);
TaskFunc(InitSonic2PControl, 0x47D820);

static void LoadNPCSonicTask(int num)
{
	auto tp = CreateElementalTask(0xAu, 0, InitSonic2PControl);
	TASKWK_CHARID(tp->twp) = Characters_Sonic;
	TASKWK_PLAYERID(tp->twp) = num;

	tp = CreateElementalTask(7u, 1, SonicTheHedgehog);
	TASKWK_CHARID(tp->twp) = Characters_Sonic;
	TASKWK_PLAYERID(tp->twp) = num;
	tp->twp->id = 2;

	PadReadOffP(1u);
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
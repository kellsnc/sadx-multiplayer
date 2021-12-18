#include "pch.h"
#include "deathzones.h"


void __cdecl sub_440C80_r(task* obj)
{
	char pNum = obj->twp->scl.y;
	anywk* dz = obj->awp;

	if (GetDebugMode())
	{
		CheckThingButThenDeleteObject((ObjectMaster*)obj);
		return;
	}

	task* parent = obj->ptp;
	if (parent)
	{
		parent->twp->flag |= 4u;
	}

	if (GetLives_r(pNum) <= 0)
		StartLevelCutscene(2);
	else {
		EntityData1* player = EntityData1Ptrs[pNum];
		SetLives_r(pNum, -1);
		MovePlayerToStartPoint(player);
		parent->twp->flag &= ~4u;
	}

	CheckThingButThenDeleteObject((ObjectMaster*)obj);

}

void __cdecl KillPlayer_r(unsigned __int8 pID)
{
	EntityData1* data; // esi

	data = EntityData1Ptrs[pID];
	if (!GetDebugMode())
	{
		data->Status | 0x1000;
		data->NextAction = 50;
		CharObj2Ptrs[pID]->Powerups |= 0x4000u;
		task* dz = CreateElementalTask(LoadObj_Data1 | LoadObj_UnknownB, 0, sub_440C80_r);
		dz->twp->scl.y = pID;
	}
}

void PlayCharacterDeathSound_r(task* obj, int pNum)
{

	int pNumCopy = (unsigned __int8)pNum;
	EntityData1* player = EntityData1Ptrs[pNumCopy];
	CharObj2* co2 = CharObj2Ptrs[pNumCopy];

	CreateChildTask(LoadObj_Data1 | LoadObj_UnknownB, sub_440C80_r, obj);
	//CameraSetEventCameraFunc((CamFuncPtr)sub_464DF0, 0, 0);

	if (!co2 || (co2->Powerups & 0x4000) == 0)
	{
		switch (pNumCopy)
		{
		case Characters_Sonic:
			if (MetalSonicFlag)
			{
				PlayVoice(2046);
			}
			else
			{
				PlaySound(1503, 0, 0, 0);
			}
			break;
		case Characters_Tails:
			PlaySound(1465, 0, 0, 0);

			if (player)
			{
				player->Status |= 0x10u;
				player->NextAction = 24;
			}
			break;
		case Characters_Knuckles:
			PlaySound(1453, 0, 0, 0);

			if (player)
			{
				player->Status |= 0x10u;
				player->NextAction = 24;
			}
			break;
		case Characters_Amy:
			PlaySound(1396, 0, 0, 0);
			break;
		case Characters_Gamma:
			PlaySound(1433, 0, 0, 0);
			break;
		case Characters_Big:
			PlaySound(1412, 0, 0, 0);
			break;
		default:
			return;
		}
	}
}

void __cdecl DeathZoneHandler_r(ObjectMaster* obj)
{
	Mysterious64Bytes dyncolthing;
	uint16_t levelact = CurrentAct | CurrentLevel;

	DeathZone* dz = *DeathZoneList[levelact];

	if (dz && (!obj->Child && (obj->Data1->Status & Status_Hurt) == 0)) {
		while (dz->Characters) {

			for (uint8_t i = 0; i < PLAYER_MAX; i++) {

				if (!EntityData1Ptrs[i])
					continue;

				dyncolthing.Position = EntityData1Ptrs[i]->Position;

				if (TestObjectIntersect(&dyncolthing, dz->Model)) {
					if (!dyncolthing.struct_v3_a.SomeFlag)
					{
						if (dyncolthing.struct_v3_b.SomeFlag)
						{
							if (fabs(dyncolthing.Position.y - dyncolthing.struct_v3_b.Distance) <= 30.0)
							{
								PlayCharacterDeathSound_r((task*)obj, i); // also run the death cutscene
								continue;
							}
						}
					}

				}
			}

			++dz;

		}
	}


	if (obj->Child)
	{
		RunObjectChildren(obj);
	}

}

//serie of hacks to not reset the game if player 1 die and make every players able to die
void init_DeathPatches() {
	WriteJump((void*)0x440CD0, KillPlayer_r);
	WriteJump(DeathZoneHandler, DeathZoneHandler_r); //also manage player death
	return;
}
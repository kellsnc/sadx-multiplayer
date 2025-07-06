#include "pch.h"
#include "pinball.h"

/// Casinopolis object: Slot machine

#define ROLL_FLAG_SPIN_L	0x100					/* Left Slot is spinning  */
#define ROLL_FLAG_SPIN_C	0x200					/* Middle Slot is spinning  */
#define ROLL_FLAG_SPIN_R	0x400					/* Right Slot is spinning  */
#define ROLL_FLAG_ACTIVE	0x800					/* Slot machine activated */
#define ROLL_FLAG_FLASH		0x1000					/* Slot machine flashing lights animation is playing */
#define ROLL_FLAG_INIT		0x8000					/* Slot machine initialized */

#define L_SLOT_TIMER twp->counter.b[0]				/* Spins Left slot when non-zero */
#define C_SLOT_TIMER twp->counter.b[1]				/* Spins Centre slot when non-zero */
#define R_SLOT_TIMER twp->counter.b[2]				/* Spins Right slot when non-zero */
#define GET_PNUM twp->id

DataArray(uint8_t, roll1_info, 0x1E78D00, 16);
DataArray(uint8_t, roll2_info, 0x1E78D10, 16);
DataArray(NJS_POINT3, RingPos, 0x1E78C44, 16);
TaskFunc(ObjectYaku, 0x5E14E0);

TaskFunc(SonicControl, 0x5C1970);
TaskFunc(TailsControl, 0x5C1B50);
TaskFunc(RingRingControl, 0x5C19E0);
TaskFunc(KnucklesControl, 0x5C1AE0);
TaskFunc(AmyControl, 0x5C1BC0);
TaskFunc(EggmanControl, 0x5C1C30);

FastFunctionHook<void, task*> ObjectRollNormal_h(0x5DAF60);


enum
{
	ROLL_SONIC,
	ROLL_RING,
	ROLL_KNUCKLES,
	ROLL_TAILS,
	ROLL_AMY,
	ROLL_EGGMAN
};

static void PutOut(taskwk* twp)
{
	auto pnum = GET_PNUM;
	//Run the initial "Get Bonus" animation the first time this function is called.
	if (twp->flag & ROLL_FLAG_FLASH)
	{
		twp->flag &= ~ROLL_FLAG_FLASH;
		if (twp->value.b[0] == 100)
		{
			twp->wtimer = 30;
		}
		else
		{
			twp->wtimer = PinballFlag & PINBALL_FLAG_SLOT_BONUS ? 90 : 200;
			if (twp->value.b[2] != ROLL_EGGMAN || PinballFlag & PINBALL_FLAG_SLOT_BONUS)
			{
				dsPlay_oneshot(SE_CA_SLOTYAKU, 0, 0, 0);
			}
			else
			{
				dsPlay_oneshot(SE_CA_SLOYDAME, 0, 0, 0);
			}
		}
	}
	else
	{
		if (--twp->wtimer)
		{
			//Screen animation
			if (twp->wtimer == 150 && !(PinballFlag & PINBALL_FLAG_SLOT_BONUS) && twp->value.b[2] != 100)
			{
				CreateElementalTask(IM_TASKWK, LEV_1, ObjectYaku)->twp->mode = twp->value.b[2];
			}
		}
		else
		{
			//Spit the player out.
			SetInputP(pnum, PL_OP_PINBALL);
			Angle3 ang = { 0, 0xF800, 0 };
			NJS_POINT3 s = {
				njRandom() * 0.5f + 2.0f,
				0.5f,
				njRandom() * 0.2f + 0.6f };
			SetVelocityAndRotationAndNoconTimeP(pnum, &s, &ang, 0);
			PinballCameraMode[pnum] = 0x80;
			twp->flag &= ~ROLL_FLAG_ACTIVE;

			//If any bonus other than Eggman is already active, do nothing.
			if (!(PinballFlag & (PINBALL_FLAG_SLOT_AMY | PINBALL_FLAG_SLOT_TAILS | PINBALL_FLAG_SLOT_KNUCKLES | PINBALL_FLAG_SLOT_RING | PINBALL_FLAG_SLOT_SONIC)))
			{
				if (PinballFlag & PINBALL_FLAG_SLOT_EGGMAN)
				{
					// Award 100 Rings + Confetti for getting Eggman in the slots while his "bonus" is already active.
					if (twp->value.b[2] == ROLL_EGGMAN)
					{
						PinballFlag |= PINBALL_FLAG_SLOT_CANNON_R | PINBALL_FLAG_SLOT_CANNON_L;
						SetGetRingM(100, pnum);
					}
				}
				else
				{
					//Apply bonus
					switch (twp->value.b[2])
					{
					case ROLL_SONIC:
						PinballFlag = PinballFlag & ~(PINBALL_FLAG_SLOT_TOWER_R | PINBALL_FLAG_SLOT_TOWER_L) | PINBALL_FLAG_SLOT_SONIC;
						CreateElementalTask(IM_TASKWK, LEV_3, SonicControl)->twp->wtimer = 5940;
						SetGetRingM(50, pnum);
						break;
					case ROLL_RING:
					{
						PinballFlag |= PINBALL_FLAG_SLOT_RING;
						task* ringtp = CreateElementalTask(IM_TASKWK, LEV_3, RingRingControl);
						ringtp->twp->wtimer = 1800;
						for (Sint32 i = 0; i < 16; ++i) {
							taskwk* ringctp = CreateChildTask(IM_TASKWK, Ring, ringtp)->twp;
							ringctp->pos = RingPos[i];
						}
						SetGetRingM(15, pnum);
						break;
					}
					case ROLL_KNUCKLES:
						PinballFlag |= PINBALL_FLAG_SLOT_KNUCKLES;
						LineKnucklesCounter = 0;
						CreateElementalTask(IM_TASKWK, LEV_3, KnucklesControl)->twp->wtimer = 4800;
						SetGetRingM(30, pnum);
						break;
					case ROLL_TAILS:
						PinballFlag |= PINBALL_FLAG_SLOT_TAILS | PINBALL_FLAG_SLOT_STOPER_C;
						CreateElementalTask(IM_TASKWK, LEV_3, TailsControl)->twp->wtimer = 2700;
						SetGetRingM(15, pnum);
						break;
					case ROLL_AMY:
						PinballFlag |= PINBALL_FLAG_SLOT_AMY;
						AmyFourCounter = 0;
						CreateElementalTask(IM_TASKWK, LEV_3, AmyControl)->twp->wtimer = 3600;
						SetGetRingM(50, pnum);
						break;
					case ROLL_EGGMAN:
						PinballFlag |= PINBALL_FLAG_SLOT_EGGMAN;
						CreateElementalTask(IM_TASKWK, LEV_3, EggmanControl)->twp->wtimer = 3600;
						SetDropRingM(50, pnum);
						break;
					default:
						return;
					}
				}
			}
		}
	}
}

 void ObjectRollNormal(task* tp)
{

	 if (multiplayer::IsActive() == false)
	 {
		 return ObjectRollNormal_h.Original(tp);
	 }


	taskwk* twp = tp->twp;

	if (twp->flag & ROLL_FLAG_ACTIVE)
	{
		auto pnum = GET_PNUM;
		//A button pressed x0
		if (twp->flag & ROLL_FLAG_SPIN_L)
		{
			//Wait for input
			if (per[pnum]->press & Buttons_A)
			{
				twp->value.b[3] = 0; //Reset delay timer for later.
				twp->flag &= ~ROLL_FLAG_SPIN_L;
			}
		}
		//A button pressed x1
		else if (twp->flag & ROLL_FLAG_SPIN_C)
		{
			//Wait for input
			if (per[pnum]->press & Buttons_A)
			{
				twp->flag &= ~ROLL_FLAG_SPIN_C;
			}

			//Stop the left slot spinning in an appropriate spot.
			if (!(twp->mode & 3))
			{
				L_SLOT_TIMER = 0;
			}
		}
		//A button pressed x2
		else if (twp->flag & ROLL_FLAG_SPIN_R)
		{
			//Wait for input
			if (per[pnum]->press & Buttons_A)
			{
				twp->flag &= ~ROLL_FLAG_SPIN_R;
			}
			//Stop the left slot spinning in an appropriate spot.
			if (!(twp->mode & 3))
			{
				L_SLOT_TIMER = 0;
			}
			//Stop the middle slot spinning in an appropriate spot.
			if (!(twp->smode & 3))
			{
				C_SLOT_TIMER = 0;
			}
		}
		//A Button pressed x3. Make sure all the slots are stopped and check the combo.
		else if (L_SLOT_TIMER || C_SLOT_TIMER || R_SLOT_TIMER)
		{
			//Stop the left slot spinning in an appropriate spot.
			if (!(twp->mode & 3))
			{
				L_SLOT_TIMER = 0;
			}

			//Stop the middle slot spinning in an appropriate spot.
			if (!(twp->smode & 3))
			{
				C_SLOT_TIMER = 0;
			}

			//Stop the right slot spinning in an appropriate spot.
			if (!(twp->btimer & 3))
			{
				if (R_SLOT_TIMER)
				{
					/*	If no bonus, delay returning the camera so the player can see what they got.
						If CheckYakuLine finds a bonus it skips this because PutOut has its own delays.*/
					if (++twp->value.b[3] == 3)
					{
						R_SLOT_TIMER = 0;
					}
					CheckYakuLine(twp);
				}
			}
		}
		else
		{
			//Process bonuses.
			PutOut(tp->twp);
		}

		//Spin the slots.
		if (L_SLOT_TIMER)
		{
			twp->ang.x += 0x400;
			++twp->mode;
		}
		if (C_SLOT_TIMER)
		{
			twp->ang.y += 0x400;
			++twp->smode;
		}
		if (R_SLOT_TIMER)
		{
			twp->ang.z += 0x400;
			++twp->btimer;
		}
	}
	else if (twp->cwp->flag & CWK_FLAG_HIT)
	{
		auto player = CCL_IsHitPlayer(twp);
		if (player)
		{
			auto pnum = player->counter.b[0];

			if (!(twp->flag & ROLL_FLAG_INIT))
			{
				twp->flag |= (ROLL_FLAG_SPIN_L | ROLL_FLAG_SPIN_C | ROLL_FLAG_SPIN_R | ROLL_FLAG_ACTIVE | ROLL_FLAG_FLASH);
				R_SLOT_TIMER = 100;
				C_SLOT_TIMER = 100;
				L_SLOT_TIMER = 100;
				//PinballCameraMode = 0x81; //this break the cam in multi so I'll let it disabled for now
				SetInputP(pnum, PL_OP_PLACEWITHSPIN);
				player->pos.x = twp->pos.x;
				player->pos.y = twp->pos.y - 10.0f;
				player->pos.z = twp->pos.z;
				SetGetRingM(5, pnum);
				twp->flag |= ROLL_FLAG_INIT;
				twp->value.b[2] = 100;
				twp->value.b[0] = 100;
				GET_PNUM = pnum;
			}
		}
	}
	else
	{
		twp->flag &= ~ROLL_FLAG_INIT;
	}
	
	tp->disp(tp);
	EntryColliList(twp);
	ObjectSetupInput(twp, 0);
}


void patch_roll_init()
{
	ObjectRollNormal_h.Hook(ObjectRollNormal);
}

RegisterPatch patch_roll(patch_roll_init);

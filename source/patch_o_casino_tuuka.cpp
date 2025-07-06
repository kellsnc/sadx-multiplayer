#include "pch.h"


#define TUUKA_FLAG_L        0x100                                           /* Left Panel */
#define TUUKA_FLAG_C        0x200                                           /* Middle Panel */
#define TUUKA_FLAG_R        0x400                                           /* Right Panel */
#define TUUKA_FLAG_CINIT    0x8000                                          /* Prevent getting rewards multiple times */
#define TUUKA_FLAG_ALL      (TUUKA_FLAG_L|TUUKA_FLAG_C|TUUKA_FLAG_R)        /* All Panels */
#define LR_FLAG_L           0x1                                             /* Left Shoulder */
#define LR_FLAG_R           0x2                                             /* Right Shoulder */


#define PINBALL_FLAG_SLOT_STOPER_C  0x1						/* Slot: Safety block active (Center) */
#define PINBALL_FLAG_SLOT_STOPER_L  0x2						/* Slot: Safety block active (Left) */
#define PINBALL_FLAG_SLOT_STOPER_R  0x4						/* Slot: Safety block active (Right) */
#define PINBALL_FLAG_DTARGET_R		0x8						/* Slot: Arrow pointing to right purple target is active */
#define PINBALL_FLAG_DTARGET_C		0x10					/* Slot: Arrow pointing to middle purple target is active */
#define PINBALL_FLAG_DTARGET_L		0x20					/* Slot: Arrow pointing to left purple target is active  */
#define PINBALL_FLAG_DTARGET_ALL	(PINBALL_FLAG_DTARGET_R|PINBALL_FLAG_DTARGET_C|PINBALL_FLAG_DTARGET_L)
#define PINBALL_FLAG_SLOT_GATE_L	0x40					/* Slot: Left tube gate is open */
#define PINBALL_FLAG_SLOT_GATE_R	0x80					/* Slot: Right tube gate is open */
#define PINBALL_FLAG_STOPER_CAM		0x100					/* Both: No visible effect in-game. See o_stoper for details.*/
#define PINBALL_FLAG_SLOT_TOWER_L	0x200					/* Slot: Entered left tower during Sonic slot bonus */
#define PINBALL_FLAG_SLOT_TOWER_R	0x400					/* Slot: Entered right tower during Sonic slot bonus */
#define PINBALL_FLAG_SLOT_CANNON_L	0x800					/* Slot: Fire left confetti cannon */
#define PINBALL_FLAG_SLOT_CANNON_R	0x1000					/* Slot: Fire right confetti cannon */
#define PINBALL_FLAG_CARD_IDEYA		0x2000					/* Card: Ideya collected (But not given to NiGHTS yet) */
#define PINBALL_FLAG_CARD_DOOR		0x4000					/* Card: Door to boss room open */
#define PINBALL_FLAG_UNUSED			0x8000					/* Not used for anything */
#define PINBALL_FLAG_CARD_THREE		0x10000					/* Card: Three-of-a-Kind bonus active */
#define PINBALL_FLAG_CARD_FHOUSE	0x20000					/* Card: Full House bonus active */
#define PINBALL_FLAG_CARD_FOUR		0x40000					/* Card: Four-of-a-Kind bonus active */
#define PINBALL_FLAG_CARD_STRAIGHT	0x80000					/* Card: Straight bonus active */
#define PINBALL_FLAG_CARD_BONUS		(PINBALL_FLAG_CARD_THREE|PINBALL_FLAG_CARD_FHOUSE|PINBALL_FLAG_CARD_FOUR|PINBALL_FLAG_CARD_STRAIGHT)
#define PINBALL_FLAG_SLOT_SONIC		0x100000				/* Slot: Sonic bonus ("Speed Star") active  */
#define PINBALL_FLAG_SLOT_RING		0x200000				/* Slot: Ring bonus ("Ring 3K") active */
#define PINBALL_FLAG_SLOT_KNUCKLES	0x400000				/* Slot: Knuckles bonus ("Shining 'K'") active */
#define PINBALL_FLAG_SLOT_TAILS		0x800000				/* Slot: Tails bonus ("T-Kickback") active */
#define PINBALL_FLAG_SLOT_AMY		0x1000000				/* Slot: Amy bonus ("Blue Rose") active */
#define PINBALL_FLAG_SLOT_EGGMAN 	0x2000000				/* Slot: Eggman bonus ("Final Eggman") active */
#define PINBALL_FLAG_SLOT_REPLAY 	0x4000000				/* Slot: Activated by hitting all 3 purple targets and deactivated by SetReplayNow. No effect, probably a leftover init flag for it. (old) */
#define PINBALL_FLAG_SLOT_NOTIMERS 	0x8000000				/* Slot: Disables the bonus timers that appear on the stage. Unused in-game, as only the unused purple target camera animation activates it. (old) */
#define PINBALL_FLAG_SLOT_BONUS		(PINBALL_FLAG_SLOT_EGGMAN|PINBALL_FLAG_SLOT_AMY|PINBALL_FLAG_SLOT_TAILS|PINBALL_FLAG_SLOT_KNUCKLES|PINBALL_FLAG_SLOT_RING|PINBALL_FLAG_SLOT_SONIC)
#define PINBALL_FLAG_OBJ_CASINO     0x10000000				/* texlist_obj_casino loaded */
#define PINBALL_FLAG_OBJ_CASINO2    0x20000000				/* texlist_obj_casino2 loaded */
#define PINBALL_FLAG_OBJ_CASINO9    0x40000000				/* texlist_obj_casino9 loaded */
#define PINBALL_FLAG_OBJ_CASINO8    0x80000000				/* texlist_obj_casino8 loaded */

DataPointer(uint8_t, LineKnucklesCounter, 0x3C748E9);
DataPointer(char, old_lr_flag, 0x3C750F1);
DataPointer(char, lr_flag_edge, 0x3C750F2);

FastFunctionHook<void, task*> TuukaExec_h(0x5D91F0);
FastFunctionHook<void, task*> RightlaneExec_h(0x5D9360);
FastFunctionHook<void, task*> TuukaExecChild_h(0x5D9130);

static void TuukaExecChild(task* tp)
{
	if (multiplayer::IsActive() == false)
	{
		return TuukaExecChild_h.Original(tp);
	}

	taskwk* twp = tp->twp;
	taskwk* ptwp = tp->ptp->twp;
	if (twp->cwp->flag & CWK_FLAG_HIT)
	{
		auto player = CCL_IsHitPlayer(twp);
		auto pnum = player->counter.b[0];

		if (!(twp->flag & TUUKA_FLAG_CINIT)) 
		{
			switch (twp->btimer) 
			{
			case 0:
				ptwp->flag |= TUUKA_FLAG_L;
				if (PinballFlag & PINBALL_FLAG_SLOT_KNUCKLES)
				{
					if (LineKnucklesCounter >= 3)
					{
						PinballFlag |= PINBALL_FLAG_SLOT_CANNON_R | PINBALL_FLAG_SLOT_CANNON_L;
						SetGetRingM((LineKnucklesCounter++ - 1) ^ 0x50, pnum);
					}
				}
				break;
			case 1:
				ptwp->flag |= TUUKA_FLAG_C;
				break;
			case 2:
				ptwp->flag |= TUUKA_FLAG_R;
				break;
			default:
				break;
			}
			SetGetRingM(1, pnum);
			twp->flag |= TUUKA_FLAG_CINIT;
		}
	}
	else {
		twp->flag &= ~TUUKA_FLAG_CINIT;
	}

	EntryColliList(twp);
	ObjectSetupInput(twp, 0);
}

static void TuukaExec(task* tp)
{

	if (multiplayer::IsActive() == false)
	{
		return TuukaExec_h.Original(tp);
	}


	taskwk* twp = tp->twp;
	LoopTaskC(tp);
	auto playerCount = multiplayer::GetPlayerCount();

	if (twp->wtimer)
	{
		//Flashing animation
		if (--twp->wtimer & 8)
			twp->flag |= TUUKA_FLAG_ALL;
		else
			twp->flag &= ~TUUKA_FLAG_ALL;
	}
	else
	{
		//If all panels lit up give rewards
		if ((twp->flag & TUUKA_FLAG_ALL) == TUUKA_FLAG_ALL)
		{
			twp->wtimer = 80;
	
			int ringCount = 0;

			if (PinballFlag & PINBALL_FLAG_SLOT_TAILS)
			{
				PinballFlag |= PINBALL_FLAG_SLOT_CANNON_R | PINBALL_FLAG_SLOT_CANNON_L;
				ringCount = (int)(30 / playerCount);
			}
			else
			{
				ringCount = (int)(15 / playerCount);

			}
			for (uint8_t i = 0; i < playerCount; i++)
			{
				SetGetRingM(ringCount, i);
			}

			// Activate the right safety bumper in Card Pinball.
			if (ssActNumber == 3)
			{
				PinballFlag |= PINBALL_FLAG_SLOT_STOPER_R;
			}
		}
		else
		{
			Sint8 lr;

			//Check for Triggers/A Button being pressed. If they are, shift the panels' state left/right. 
			for (uint8_t i = 0; i < playerCount; i++)
			{
				lr = (perG[i].l - 0x80) << 8 > 0x80;
				if ((perG[i].r - 0x80) << 8 > 0x80)
				{
					lr |= LR_FLAG_R;
					break;
				}
			}

			lr_flag_edge = lr & ~old_lr_flag;
			old_lr_flag = lr;

			Sint16 flag = twp->flag;
			if (lr_flag_edge & LR_FLAG_L)
			{
				//Switch to left
				twp->flag &= ~TUUKA_FLAG_ALL;
				if (flag & TUUKA_FLAG_L)
				{
					twp->flag = flag & ~TUUKA_FLAG_ALL | TUUKA_FLAG_R;
				}
				if (flag & TUUKA_FLAG_C)
				{
					twp->flag |= TUUKA_FLAG_L;
				}
				if (flag & TUUKA_FLAG_R)
				{
					twp->flag |= TUUKA_FLAG_C;
				}
			}
			else 
			{
				for (uint8_t i = 0; i < playerCount; i++)
				{
					if (lr_flag_edge & LR_FLAG_R || per[i]->press & Buttons_A)
					{
						//Switch to right
						twp->flag &= ~TUUKA_FLAG_ALL;
						if (flag & TUUKA_FLAG_L)
						{
							twp->flag = flag & ~TUUKA_FLAG_ALL | TUUKA_FLAG_C;
						}
						if (flag & TUUKA_FLAG_C)
						{
							twp->flag |= TUUKA_FLAG_R;
						}
						if (flag & TUUKA_FLAG_R)
						{
							twp->flag |= TUUKA_FLAG_L;
						}
						break;
					}
				}
			}
		}
	}

	tp->disp(tp);
}

static void RightlaneExec(task* tp)
{
	if (multiplayer::IsActive() == false)
	{
		return RightlaneExec_h.Original(tp);
	}

	taskwk* twp = tp->twp;
	if (twp->cwp->flag & CWK_FLAG_HIT)
	{
		auto player = CCL_IsHitPlayer(twp);
		auto pnum = player->counter.b[0];

		if (!(twp->flag & TUUKA_FLAG_CINIT))
		{
			//Slot Pinball bonus
			if (PinballFlag & PINBALL_FLAG_SLOT_KNUCKLES && LineKnucklesCounter == 1)
			{
				LineKnucklesCounter = 2;
				SetGetRingM(20, pnum);
			}

			//Card Pinball bonus
			if (PinballFlag & PINBALL_FLAG_CARD_THREE)
			{
				++ThreeCardCounter;
				SetGetRingM(5 * ThreeCardCounter, pnum);
			}

			if (++twp->btimer % 5)
			{
				SetGetRingM(3, pnum);
				twp->flag |= TUUKA_FLAG_CINIT;
			}
			else
			{
				SetGetRingM(13, pnum);
				twp->flag |= TUUKA_FLAG_CINIT;
				twp->btimer = 0;
			}
		}
	}
	else
	{
		twp->flag &= ~TUUKA_FLAG_CINIT;
	}
	EntryColliList(twp);
	ObjectSetupInput(twp, 0);
}

void patch_tuuka_init()
{
	TuukaExec_h.Hook(TuukaExec);
	RightlaneExec_h.Hook(RightlaneExec);
	TuukaExecChild_h.Hook(TuukaExecChild);
}

RegisterPatch patch_tuuka(patch_tuuka_init);
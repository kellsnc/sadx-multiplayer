#include "pch.h"
#include "pinball.h"

#define TUUKA_FLAG_L        0x100                                           /* Left Panel */
#define TUUKA_FLAG_C        0x200                                           /* Middle Panel */
#define TUUKA_FLAG_R        0x400                                           /* Right Panel */
#define TUUKA_FLAG_CINIT    0x8000                                          /* Prevent getting rewards multiple times */
#define TUUKA_FLAG_ALL      (TUUKA_FLAG_L|TUUKA_FLAG_C|TUUKA_FLAG_R)        /* All Panels */
#define LR_FLAG_L           0x1                                             /* Left Shoulder */
#define LR_FLAG_R           0x2                                             /* Right Shoulder */





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
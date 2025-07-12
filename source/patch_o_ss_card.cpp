#include "pch.h"

TaskHook CardExec_h(0x63C030);

enum {
	MODE_START,
	MODE_NORMAL,
	MODE_HOLD,
	MODE_HOLD_ALREADY,
	MODE_FADEOUT,
	MODE_FREEZE,
	MODE_END
};

DataArray(const char**, use_message, 0x2BC080C, 0x5);
DataArray(const char**, get_message, 0x2BC07F8, 0x5);

static const void* const Exec_checkNearSpeedHighwayPtr = (void*)0x63BE60;
static inline signed int Exec_checkNearSpeedHighway(NJS_POINT3* pos)
{
	signed int result;
	__asm
	{
		mov eax, [pos]
		call Exec_checkNearSpeedHighwayPtr
		mov result, eax
	}
	return result;
}


static void Exec(task* tp)
{
	taskwk* twp = tp->twp;
	if (!CheckRangeOut(tp))
	{
		if (OnEdit(tp))
		{
			tp->disp(tp);
		}
		else
		{
			switch (twp->mode)
			{
			case MODE_START:
			case MODE_NORMAL:
			{
				NJS_ACTION* counter = (NJS_ACTION*)twp->counter.ptr;
				twp->timer.f += 0.1f;
				if (twp->timer.f >= (Float)(counter->motion->nbFrame - 1))
					twp->timer.f = -0.8f;
				ObjectMovableSRegularExecute(tp);
				tp->disp(tp);
				if (twp->flag & TWK_FLAG_HELD)
				{
					auto pnum = GetTheNearestPlayerNumber(&twp->pos);
					SetHoldingItemIDP(pnum, PORTABLE_CARD);
					twp->mode = MODE_HOLD;
					twp->id = pnum;
					DisplayHintText(get_message[Language], 180);
				}
				if (twp->mode == MODE_NORMAL && ++twp->wtimer > 1800)
				{
					FreeTask(tp);
				}
				break;
			}
			case MODE_HOLD:
			{
				twp->timer.l = 0;
				twp->ang.y = -playertwp[twp->id]->ang.y;
				ObjectMovableSRegularExecute(tp);
				tp->disp(tp);

				//If held, check for being near the entrance to Speed Highway.
				if (twp->flag & TWK_FLAG_HELD)
				{
					SetContinue(tp);
					if (!SeqCheckFlag(FLAG_SONIC_SS_ENTRANCE_HIGHWAY) && ssActNumber == 3)
					{
						if (Exec_checkNearSpeedHighway(&twp->pos))
						{
							seqVars[FLAG_SONIC_SS_ENTRANCE_HIGHWAY] = 1;
							twp->value.l = 0;
							HintMainMessages(use_message[Language]);
							PlayJingle(56); //Item unlock
							twp->wtimer = 0;
						}
					}
				}
				else
				{
					SetHoldingItemIDP(twp->id, PORTABLE_NOTHING);
					twp->wtimer = 0;
					twp->mode = MODE_NORMAL;
					tp->ocp->ssCondition &= ~0x4000;
				}
				break;
			}
			default:
			{
				CardExec_h.Original(tp);
			}
			}
		}
	}
}


void patch_o_ss_card_init()
{
	//SH Card entrance
	WriteCall((void*)0x63BEC2, GetCharacterID_r);
	//CardExec_h.Hook(Exec);
}

RegisterPatch patch_o_ss_card(patch_o_ss_card_init);
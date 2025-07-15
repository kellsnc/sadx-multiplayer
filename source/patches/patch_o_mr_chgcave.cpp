#include "pch.h"

FunctionPointer(void, create_leaf, (float x, float y, float z, float rad), 0x537B20);
FunctionPointer(void, create_woodytrash, (float x, float y, float z, float rad), 0x537BF0);


TaskHook OChgCave_h(0x537FB0);
static void OChgCave_r(task* tp)
{
	auto twp = tp->twp;
	if (!CheckRangeOut(tp) && !OnEdit(tp))
	{
		if (twp->mode == 1)
		{
			++twp->ang.x;
			create_leaf(twp->pos.x, twp->pos.y - 20.0f, twp->pos.z, 40.0f);
			if ((twp->ang.x & 1) == 0)
			{
				create_smoke(twp, 30.0f);
			}
			if ((twp->ang.x & 3) == 0)
			{
				create_woodytrash(twp->pos.x, twp->pos.y - 20.0f, twp->pos.z, 30.0);
			}
			auto pnum = CheckCollisionCylinderP(&twp->pos, 30.0f, 100.0f);
			if (pnum)
			{
				pnum -= 1;
				Float v3 = twp->value.f - (playertwp[pnum]->pos.y - twp->pos.y) * 0.02;
				if (v3 < 0.01f)
				{
					v3 = 0.01f;
				}
				SetAscendPowerP(pnum, 0.0f, v3, 0.0f);
				Uint32 wtimer = twp->wtimer;
				twp->wtimer = wtimer + 1;
				if (wtimer == 20)
				{
					SetEntranceNumber(0);
					SeqChangeStage(LevelIDs_MysticRuins, 1u);
					InitFreeCamera();
				}
			}
			else
			{
				twp->wtimer = 0;
			}

		}
	}

	if (twp->mode != 1)
		OChgCave_h.Original(tp);
}

void __cdecl OChgCave2_r(task* tp)
{
    taskwk* twp = tp->twp;
    if (!CheckRangeOut(tp) && !OnEdit(tp))
    {
        if (twp->mode)
        {
            if (twp->mode == 1)
            {
				int pnum = CheckCollisionCylinderP(&twp->pos, 30.0f, 100.0f);
                if (pnum)
                {
					pnum -= 1;
                    Float Y = twp->value.f - (playertwp[pnum]->pos.y - twp->pos.y) * 0.02f;
        
                    if (Y < 0.01f)
                    {
						Y = 0.01f;
                    }
                    SetAscendPowerP(pnum, 0.0f, Y, 0.0f);
                    Uint32 v3 = twp->wtimer;
                    if (v3 > 0x14u)
                    {
                        twp->value.f -= 0.02f;
                    }
                    twp->wtimer = v3 + 1;
                    if (v3 == 20)
                    {
                        SetEntranceNumber(1);
                        SeqChangeStage(LevelIDs_MysticRuins, 0);
                        InitFreeCamera();
                    }
                }
                else
                {
                    twp->wtimer = 0;
                    twp->value.l = 0x3F333333;
                }
            }
        }
        else
        {
            twp->mode = 1;
            twp->value.l = 0x3F333333;
        }
    }
}

void patch_o_mr_chgcave_init()
{
	OChgCave_h.Hook(OChgCave_r);
	WriteJump((void*)0x536ED0, OChgCave2_r);
}

RegisterPatch patch_o_mr_chgcave(patch_o_mr_chgcave_init);
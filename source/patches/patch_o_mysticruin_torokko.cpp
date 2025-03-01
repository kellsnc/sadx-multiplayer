#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"

// Mystic Ruin minecarts

FastUsercallHookPtr<void(*)(task* tp), noret, rEAX> ObjectMysticRuinTorokko_ExecATask_t(0x53D830);

void __cdecl ObjectMysticRuinTorokko_ExecATask_r(task* tp)
{
	if (!multiplayer::IsActive())
	{
		ObjectMysticRuinTorokko_ExecATask_t.Original(tp);
		return;
	}

	taskwk* twp = tp->twp;
	colliwk* cwp = twp->cwp;

	if (!twp)
	{
		return;
	}

	auto pnum = GetTheNearestPlayerNumber(&twp->pos);
	taskwk* pData = playertwp[pnum];

	if (!pData)
	{
		return;
	}

	NJS_POINT3 pos;

	switch (twp->smode)
	{
	case 0:
		njPushMatrix(nj_unit_matrix_);
		njTranslateV(0, &twp->pos);
		ROTATEY(0, 0, twp->ang.y);
		njInvertMatrix(0);
		njCalcPoint(0, &pData->pos, &pos);
		njPopMatrix(1u);

		if (cwp)
		{
			if ((pos.z - -0.14892) * (pos.z - -0.14892) + (pos.x - -1.25203) * (pos.x - -1.25203) < 36.0)
			{
				cwp->info[1].attr |= 0x10;
				cwp->info[2].attr |= 0x10;
				cwp->info[3].attr |= 0x10;
				cwp->info[4].attr |= 0x10;
				twp->btimer = 0;
			}
			else
			{
				cwp->info[1].attr &= ~0x10;
				cwp->info[2].attr &= ~0x10;
				cwp->info[3].attr &= ~0x10;
				cwp->info[4].attr &= ~0x10;
				twp->btimer = 1;
			}
		}
		else
		{
			twp->btimer = 0;
		}

		if (cwp && (cwp->flag & 2) && cwp->hit_cwp->mytask == playertp[pnum] && cwp->my_num == 0 && cwp->hit_num == 0)
		{
			if (twp->flag & 0x1 && twp->btimer == 0)
			{
				if (++twp->wtimer >= 2)
				{
					for (int i = 0; i < PLAYER_MAX; i++)
					{
						if (playertwp[i])
						{
							SetInputP(i, PL_OP_PLACEON);
							RotatePlayer(i, twp->ang.y + 0x4000);
						}
					}
					twp->btimer = 1;
					twp->smode = 1;
				}
			}
		}
		else
		{
			twp->btimer = 0;
			twp->wtimer = 0;
		}
		break;
	case 1:
		playertwp[pnum]->pos.x = twp->pos.x;
		playertwp[pnum]->pos.y = twp->pos.y + 5.4f;
		playertwp[pnum]->pos.z = twp->pos.z;

		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (playertwp[i])
			{
				SetInputP(i, PL_OP_PLACEON);
				RotatePlayer(i, twp->ang.y + 0x4000);
				if (i != pnum)
					PositionPlayer(i, pData->pos.x, pData->pos.y, pData->pos.z + 3.0f * (float)i);
			}
		}

		twp->value.f -= 0.8f;
		if (twp->value.f <= 0.0f)
		{
			njPushMatrix(nj_unit_matrix_);
			njTranslateV(0, &twp->pos);
			ROTATEY(0, 0, twp->ang.y);
			njInvertMatrix(0);
			njCalcPoint(0, &pData->pos, &RidePos);
			njPopMatrix(1u);
			MRTorokkoRideFlg = 1;
			twp->value.l = 0;
			dsPlay_oneshot(589, 0, 0, 0);
			twp->smode = 2;
		}
		break;
	case 2:
		pData = playertwp[0];
		njPushMatrix(nj_unit_matrix_);
		CalcMMMatrix(0, MoveAction[twp->timer.w[1]], twp->value.f, 0, 0);
		SetInstancedMatrix(0, 0);
		njCalcPoint(0, &RidePos, &pData->pos);
		njCalcVector(0, &XVec1, &pos);
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (playertwp[i])
			{
				if (i != 0)
					PositionPlayer(i, pData->pos.x, pData->pos.y, pData->pos.z + 5.0f * (float)i);
				RotatePlayer(i, 0x4000 - -njArcTan2(-pos.z, pos.x));
			}
		}
		njPopMatrix(1u);

		twp->value.f += 0.1f;
		twp->timer.w[0] += 0x1000;
		if (twp->value.f >= (float)MoveAction[twp->timer.w[1]]->motion->nbFrame - 1.0f)
		{
			twp->value.f = (float)MoveAction[twp->timer.w[1]]->motion->nbFrame - 1.0f;
		}

		dsPlay_timer(590, 0, 1, 0, 2);
		if (++twp->wtimer == 120)
		{
			switch (twp->timer.w[1])
			{
			case 0:
				if (GetPlayerCharacterName(0) == Characters_Gamma)
					SeqClrFlag(48);
				SetEntranceNumber(0);
				SeqChangeStage(LevelIDs_MysticRuins, 2);
				InitFreeCamera();
				break;
			case 1:
				SetEntranceNumber(0);
				SeqChangeStage(LevelIDs_MRGarden, 0);
				InitFreeCamera();
				break;
			case 2:
				SetEntranceNumber(2);
				SeqChangeStage(LevelIDs_MysticRuins, 0);
				InitFreeCamera();
				break;
			}
		}
		break;
	}

	if (CheckCollisionP(&twp->pos, 50.0f))
	{
		twp->flag |= 0x1;
	}
	else
	{
		twp->flag &= ~0x1;
	}
}

void patch_o_mysticruin_torokko_init()
{
	ObjectMysticRuinTorokko_ExecATask_t.Hook(ObjectMysticRuinTorokko_ExecATask_r);
}

RegisterPatch patch_o_mysticruin_torokko(patch_o_mysticruin_torokko_init);
#include "pch.h"
#include "patches/chaos_common.h"

FastFunctionHook<void, task*> Eggman_Chaos6Normal_h(0x55A600);
DataPointer(char, eisei_flag, 0x3C69AC4);
VoidFunc(shotSateriteEggman, 0x55A580);
FunctionPointer(signed int, Chaos6SuikomiObject, (taskwk* data), 0x5590D0);
DataPointer(char, player_iced_flag, 0x3C69ABC);

TaskHook Bomb_Chaos6Main_h(0x55C3A0);


enum Chaos6Actions : __int32
{
	MD_CHAOS6_INIT = 0x0,
	MD_CHAOS6_STND = 0x1,
	MD_CHAOS6_WALK = 0x2,
	MD_CHAOS6_PUNCH = 0x3,
	MD_CHAOS6_HORMINGTAIL = 0x4,
	MD_CHAOS6_SUIKOMI = 0x5,
	MD_CHAOS6_JUMPATTACK = 0x6,
	MD_CHAOS6_JUMP = 0x7,
	MD_CHAOS6_O2W = 0x8,
	MD_CHAOS6_W2O = 0x9,
	MD_CHAOS6_O2HT = 0xA,
	MD_CHAOS6_HT2O = 0xB,
	MD_CHAOS6_FREEZE = 0xC,
	MD_CHAOS6_PUSH = 0xD,
	MD_CHAOS6_DAMAGE = 0xE,
	MD_CHAOS6_DAMAGE2 = 0xF,
	MD_CHAOS6_DEAD = 0x10,
	MD_CHAOS6_EV_WAIT = 0x11,
	MD_CHAOS6_EV_WATER = 0x12,
	MD_CHAOS6_ZIGZAG = 0x13,
	MD_CHAOS6_JUMP_R = 0x14,
	MD_CHAOS6_WALK_S = 0x15,
	MD_CHAOS6_STOP = 0x16,
	MD_CHAOS6_APART = 0x17,
	MD_CHAOS6_EDIT = 0x18,
};

//need an esp adjust of +4 here for some reason...
FastUsercallHookPtr<void(*)(chaoswk* cwk, task* tw), noret, rEDI, rECX, stack4> Chaos6Suikomi_h(0x559600);
void Chaos6Suikomi(chaoswk* cwk, task* tp)
{
	static NJS_POINT3 v_2 = { -2.0, 2.0, 0.0 };
	taskwk* twp = tp->twp;

	auto pnum = GetBossTargetPlayerRandom();
	auto p = playertwp[pnum];

	switch (twp->smode)
	{
	case 2:
		dsPlay_timer(393, (int)twp, 1, 0, 10);
		VibShot(pnum, 0);
		turnToPlayer_r(twp, cwk);
		if (!--twp->timer.w[0])
		{
			twp->smode = 4;
		}
		if (Chaos6SuikomiObject(p))
		{
			cwk->bwk.req_action = MD_CHAOS6_PUSH;
			SetInputP(pnum, 12);
			twp->smode = 3;
			VibShot(pnum, 0);
			if ((p->flag & 0x800) != 0)
			{
				player_iced_flag = 1;
			}
		}
		break;
	case 3:
	{
		p->ang.x += 2048;
		p->ang.y += 4096;
		p->ang.z += 6144;

		NJS_POINT3 pPos;
		pPos = p->pos;
		njSubVector(&pPos, (const NJS_VECTOR*)&chaos_objpv[0].pos);
		njScalor(&pPos);
		p->pos = chaos_objpv[0].pos;
		if (cwk->bwk.action == 14 && cwk->bwk.nframe >= 20.0f)
		{
			p->ang.z = 0;
			p->ang.x = 0;
			p->ang.y = 0x4000 - twp->ang.y;
			SetInputP(pnum, 24);
			if (player_iced_flag)
			{
				SetInputP(pnum, 38);
			}
			else
			{
				p->flag |= 4u;
			}
			PConvertVector_P2G(p, &v_2);
			SetVelocityP(pnum, v_2.x, v_2.y, v_2.z);
			twp->smode = 5;
		}
	}
		break;
	}

	Chaos6Suikomi_h.Original(cwk, tp);
}


//todo rework the entire thing so bomb can freeze others players
void Bomb_Chaos6Main(task* parent)
{
	auto twp = parent->twp;

	if (multiplayer::IsActive())
	{
		int flag = twp->cwp->flag | 0x40;

		for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
		{
			if (!playertwp[i])
				continue;
	
			if ((playertwp[i]->flag & 0x800) != 0)
			{
				flag = twp->cwp->flag & 0xFFBF;
				break;
			}
			else
			{
				flag = twp->cwp->flag | 0x40;
			}
		}

		twp->cwp->flag = flag;
	}

	if (twp->mode <= 6 && (twp->flag & 4) != 0)
	{

		auto p = CCL_IsHitPlayer(twp);

		if (p)
		{
			auto pnum = p->counter.b[0];
			SetVelocityP(pnum, 0.0f, 2.0f, 0.0f);
			VibShot(pnum, 0);
		}
	}

	Bomb_Chaos6Main_h.Original(parent);
}

void Eggman_Chaos6Normal(task* tp)
{
	auto mwp = tp->mwp;
	auto twp = tp->twp;
	auto pnum = GetTheNearestPlayerNumber(&twp->pos);
	auto p = playertwp[pnum];

	if (!CheckEditMode() && chaostwp->mode != 1)
	{
		NJS_POINT3* twpPos = &twp->pos;
		twp->ang.y = AdjustAngle(
			twp->ang.y,
			(unsigned __int64)(njArcTan2(p->pos.z - twp->pos.z, p->pos.x - twp->pos.x)),
			546);

		switch (twp->mode)
		{
		case 0:

			twp->pos.y -= 1.5f;
			if (twp->pos.y <= 774.0f)
			{
				twp->pos.y = 774.0f;
				twp->mode++;
			}
			break;
		case 1:
		{
			Angle v8 = (unsigned __int64)(njArcTan2(p->pos.z - twp->pos.z, p->pos.x - twpPos->x));
			mwp->spd.x = njCos(v8) * 1.5f;
			mwp->spd.z = njSin(v8) * 1.5f;
			twp->timer.w[0] = (unsigned __int64)(((double)njRandom() + 1.0f) * 60.0f);
			eisei_flag = 0;
			twp->mode = 2;
		}
			break;
		case 2:
		{
			mwp->spd.y = 0.0f;
			Float v9 = twpPos->x - p->pos.x;
			Float v10 = twp->pos.z - p->pos.z;
			if (v10 * v10 + v9 * v9 >= 900.0f)
			{
				mwp->spd.y = -2.0f;
			}
			else if (twp->pos.y - p->pos.y < 20.0f)
			{
				mwp->spd.y = 3.0f;
			}
			njAddVector(&twp->pos, &mwp->spd);
			if (twp->pos.y >= 774.0f)
			{
				if (twp->pos.y <= 844.0f)
				{
					twp->pos.y = twp->pos.y; //nothing
				}
				else
				{
					twp->pos.y = 844.0f;
				}
			}
			else
			{
				twp->pos.y = 774.0f;
			}

			if (sub_557BB0(&chaostwp->pos, &twp->pos) < 50.0f || Chaos6AttackStatus())
			{
				twp->mode = 3;
			}
			else if (--twp->timer.w[0] <= 16)
			{
				twp->mode = 1;
			}
			else if (++twp->timer.w[1] > 120)
			{
				twp->timer.w[1] = 0;
				shotSateriteEggman();
			}
			break;
		}

		}


		if (twp->mode > 2)
		{
			Eggman_Chaos6Normal_h.Original(tp);
		}
		else
		{
			tp->disp(tp);
			LoopTaskC(tp);
			DrawShadow((EntityData1*)twp, 1.2f);
		}
	}
}

TaskHook ExecModeChaos6_h(0x559CC0);

void ExecModeChaos6(task* tp)
{
	auto twp = tp->twp;
	static NJS_POINT3 stru_1251E38 = { -2.0f, 2.0f, 0.0f };

	switch (twp->mode)
	{
	case MD_CHAOS6_FREEZE:
		if (twp->smode == 3)
		{
			if ((twp->flag & 4) != 0)
			{
				auto p = CCL_IsHitPlayer(twp);
				if (p)
				{
					auto pnum = p->counter.b[0];
					PConvertVector_P2G(p, &stru_1251E38);
					SetVelocityP(pnum, stru_1251E38.x, stru_1251E38.y, stru_1251E38.z);
					RumbleB(pnum, 7u, 59, 6);
				}
			}
		}
		break;
	}

	ExecModeChaos6_h.Original(tp);
}

static void FreePlayers(uint8_t pnum, char mode)
{
	SetInputP(pnum, mode);

	for (uint8_t i = 1; i < multiplayer::GetPlayerCount(); i++)
	{
		SetInputP(i, mode);
	}
}

void patch_chaos6_init()
{
	WriteData<3>((int*)0x55A66A, 0x90); //don't update chaos angle with player 1
	Eggman_Chaos6Normal_h.Hook(Eggman_Chaos6Normal);

	Bomb_Chaos6Main_h.Hook(Bomb_Chaos6Main);
	WriteData<5>((int*)0x55C4A3, 0x90); //don't bump player 1 when hitting bomb
	WriteData<5>((int*)0x55C4AC, 0x90); //don't turn on vibration for player 1 when hitting bomb
	
	ExecModeChaos6_h.Hook(ExecModeChaos6);
	WriteData<5>((int*)0x559399, 0x90); //don't bump player 1 when hitting chaos
	WriteData<5>((int*)0x5593EC, 0x90); //don't turn on vibration for player 1 when hitting chaos

	Chaos6Suikomi_h.Hook(Chaos6Suikomi);
	WriteCall((void*)0x5592B1, FreePlayers);
}

#ifdef MULTI_TEST
	RegisterPatch patch_chaos6(patch_chaos6_init);
#endif
#include "pch.h"
#include "multiplayer.h"
#include "camera.h"

enum : __int8
{
	MODE_INIT,
	MODE_SLEEP,
	MODE_2,
	MODE_3
};

DataPointer(int, SAL_funsleeptime, 0x1E76808);
DataPointer(int, SAL_funalivetime, 0x1E7680C);
DataPointer(float, SAL_funfraction, 0x1E76804);
DataPointer(float, SAL_funrotacc, 0x1E76800);
DataPointer(float, SAL_funrotspd, 0x1E767FC);
DataPointer(float, SAL_funaccy, 0x1E767F8);
DataPointer(float, SAL_randpitch, 0x1E76810);
DataPointer(float, SAL_randspd, 0x1E76814);
DataPointer(int, kuzuref, 0x3C75070);
DataPointer(LPVOID, pkuzubuffer, 0x3C7506C);

DataArray(int, fanstat, 0x3C75058, 5);

FunctionPointer(void, ExecKazu, (float yacc, taskwk* twp), 0x5CB6B0);

static void __cdecl ObjectCasinoFanfan_Exec_r(task* tp);
FastFunctionHookPtr<decltype(&ObjectCasinoFanfan_Exec_r)> ObjectCasinoFanfan_Exec_t(0x5CBAF0);

UsercallFunc(Bool, IsRangeIn, (taskwk* twp, NJS_POINT3* pos, float mul_), (twp, pos, mul_), 0x5CB9B0, rEAX, rEAX, rECX, stack4);
UsercallFuncVoid(InitKuzu, (taskwk* twp), (twp), 0x5CB9B0, rEBX);

static bool IsPlayerRangeIn(taskwk* twp, float mul)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (playertwp[i] && IsRangeIn(twp, &playertwp[i]->pos, mul) == TRUE)
		{
			return true;
		}
	}

	return false;
}

static bool IsCameraRangeIn(taskwk* twp, float mul)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto cam_pos = GetCameraPosition(i);

		if (cam_pos && IsRangeIn(twp, cam_pos, mul) == TRUE)
		{
			return true;
		}
	}

	return false;
}

static void EndKazu(taskwk* twp) //inlined
{
	if (twp->counter.ptr)
	{
		if (pkuzubuffer)
		{
			if (!--kuzuref)
			{
				FreeMemory(pkuzubuffer);
				pkuzubuffer = 0;
			}
		}
		twp->counter.ptr = 0;
	}
}

static void Exec_m(task* tp)
{
	auto twp = tp->twp;

	if (CheckRangeOutWithR(tp, 2250000.0f) || !IsCameraRangeIn(twp, 3.0f))
	{
		EndKazu(twp);
	}

	if (IsPlayerRangeIn(twp, 2.5f))
	{
		if (twp->mode == MODE_SLEEP)
		{
			if (twp->value.f == 0.0f)
			{
				dsPlay_Dolby_time(263, (int)twp, 1, 0, 400, twp);
			}

			if (IsPlayerRangeIn(twp, 1.0f))
			{
				twp->value.f = min(SAL_funrotacc + twp->value.f, SAL_funrotspd);

				if (!--twp->wtimer)
				{
					twp->mode = MODE_2;
					twp->wtimer = SAL_funsleeptime - (int)(njRandom() * -150.0);
				}
			}
			else
			{
				twp->mode = MODE_2;
				twp->wtimer = SAL_funsleeptime - (int)(njRandom() * -300.0);
			}
		}
		else
		{
			if (twp->mode == MODE_2)
			{
				if (!--twp->wtimer)
				{
					twp->mode = MODE_INIT;
					twp->wtimer = 0ui16;
				}
			}
			else if (IsPlayerRangeIn(twp, 1.0f))
			{
				twp->mode = MODE_SLEEP;
				twp->wtimer = SAL_funalivetime - (int)(njRandom() * -300.0);
			}

			twp->value.f = max(0, twp->value.f - SAL_funfraction * SAL_funrotacc);
		}

		twp->ang.y += NJM_DEG_ANG(twp->value.f);

		if (twp->value.f > 0.0f && IsPlayerRangeIn(twp, 1.0f))
		{
			if (!(GameTimer & 1))
			{
				float pow = twp->value.f / SAL_funrotspd * SAL_funaccy;

				if ((float)njRandom() <= SAL_randpitch)
				{
					pow += ((float)(njRandom() - 0.5) * SAL_randspd) * 2.0f;
				}

				pow = pow + pow;

				for (int i = 0; i < PLAYER_MAX; ++i)
				{
					auto ptwp = playertwp[i];

					if (!ptwp)
					{
						continue;
					}

					if (IsRangeIn(twp, &ptwp->pos, 1.0f) == TRUE)
					{
						SetAscendPowerP(i, 0, pow, 0);
					}
				}

				if (IsCameraRangeIn(twp, 3.0f))
				{
					InitKuzu(twp);
					ExecKazu(pow, twp);
				}
			}

			dsPlay_Dolby_time(264, (int)twp, 1, 0, 30, twp);
		}
		else if (!(GameTimer & 1) && IsCameraRangeIn(twp, 3.0f))
		{
			InitKuzu(twp);
			ExecKazu(0.0f, twp);
		}

		auto fan_id = (int)twp->scl.z;
		if (fan_id < 5)
		{
			fanstat[fan_id] = twp->mode != MODE_SLEEP;
		}

		EntryColliList(twp);
	}

	tp->disp(tp);
}

static void __cdecl ObjectCasinoFanfan_Exec_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		Exec_m(tp);
	}
	else
	{
		ObjectCasinoFanfan_Exec_t.Original(tp);
	}
}

void patch_casino_fanfan_init()
{
	ObjectCasinoFanfan_Exec_t.Hook(ObjectCasinoFanfan_Exec_r);
}

RegisterPatch patch_casino_fanfan(patch_casino_fanfan_init);
#include "pch.h"
#include "fishing.h"
#include "multiplayer.h"

// Tub stopper in Hot Shelter

#define SEN_PNUM(twp) twp->smode

enum : __int8
{
	MODE_0,
	MODE_WAIT,
	MODE_2,
	MODE_3,
	MODE_4,
	MODE_END
};

DataPointer(int, sen_flag, 0x3C72A4C);
DataPointer(int, aquarium_sequence_flag, 0x3C72A50);

static void execObjShelterSen_m(task* tp)
{
	auto twp = tp->twp;
	auto mwp = tp->mwp;

	switch (twp->mode)
	{
	case MODE_WAIT:
		if ((sen_flag & 1) && aquarium_sequence_flag == 2)
		{
			FreeTask(tp);
			return;
		}

		for (int i = 0; i < PLAYER_MAX; ++i)
		{
			auto etc = GetBigEtc(i);

			if (!etc || !etc->Big_Lure_Ptr)
			{
				continue;
			}

			auto lure_twp = etc->Big_Lure_Ptr->twp;
			auto lure_mwp = etc->Big_Lure_Ptr->mwp;

			if (!lure_twp || !lure_mwp)
			{
				continue;
			}

			const float x = lure_twp->pos.x - twp->pos.x;
			const float y = lure_twp->pos.y - (twp->pos.y + 7.0f);
			const float z = lure_twp->pos.z - twp->pos.z;
			const float dist = x * x + y * y + z * z;

			if (dist < 1600.0f)
			{
				if (!etc->Big_Fish_Ptr)
				{
					etc->Big_Fish_Ptr = tp;
				}

				if (dist < 64.0f && njScalor2(&lure_mwp->spd) < 0.035f)
				{
					SEN_PNUM(twp) = i;
					twp->wtimer = 0i16;
					twp->mode = MODE_3;
					etc->Big_Fish_Flag |= (LUREFLAG_ESCAPE | LUREFLAG_HIT);
					lure_twp->pos.x = twp->pos.x;
					lure_twp->pos.y = twp->pos.y - 5.5f;
					lure_twp->pos.z = twp->pos.z;
					dsPlay_oneshot(318, 0, 0, 0);
					break;
				}
			}
			else
			{
				if (etc->Big_Fish_Ptr == tp)
				{
					etc->Big_Fish_Ptr = nullptr;
				}
			}
		}

		break;
	case MODE_3:
	{
		auto etc = GetBigEtc(SEN_PNUM(twp));

		if (etc)
		{
			if (etc->Big_Fish_Flag & LUREFLAG_SWING)
			{
				etc->Big_Fish_Flag &= ~(LUREFLAG_ESCAPE | LUREFLAG_HIT);
				sen_flag |= 1u;
				twp->mode = MODE_4;

				if (etc->Big_Fish_Ptr == tp)
				{
					etc->Big_Fish_Ptr = nullptr;
				}

				mwp->spd.y = 0.05f;

				if (etc->Big_Lure_Ptr)
				{
					auto lure_twp = etc->Big_Lure_Ptr->twp;

					if (lure_twp)
					{
						twp->ang.y = NJM_RAD_ANG(atan2(lure_twp->pos.x - twp->pos.x, lure_twp->pos.z - twp->pos.z));
					}
				}

				mwp->ang_spd.x = 0x3E8;
				dsPlay_oneshot(314, 0, 0, 0);
			}
			else
			{
				if (etc->Big_Lure_Ptr)
				{
					auto lure_twp = etc->Big_Lure_Ptr->twp;

					if (lure_twp)
					{
						lure_twp->pos.x = twp->pos.x;
						lure_twp->pos.y = twp->pos.y - 5.5f;
						lure_twp->pos.z = twp->pos.z;
					}
				}
			}
		}

		break;
	}
	case MODE_4:
		twp->pos.y += 0.4f;
		twp->ang.y += 0x7D0;
		twp->ang.x += mwp->ang_spd.x;

		twp->counter.f -= 0.005f;
		if (twp->counter.f < -1.0f)
		{
			twp->counter.f = -1.0f;
			twp->mode = MODE_END;
		}
		break;
	}

	tp->disp(tp);
}

static void __cdecl execObjShelterSen_r(task* tp); // "Exec"
Trampoline execObjShelterSen_t(0x5AAB10, 0x5AAB15, execObjShelterSen_r);
static void __cdecl execObjShelterSen_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		execObjShelterSen_m(tp);
	}
	else
	{
		TARGET_STATIC(execObjShelterSen)(tp);
	}
}
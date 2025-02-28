#include "pch.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "multiplayer.h"
#include "result.h"

DataPointer(uint8_t, byte_3C75126, 0x3C75126);
DataPointer(uint16_t, word_3C75124, 0x3C75124);
DataPointer(int, ring_kiran, 0x38D8D64);

static auto RdRuinInit = GenerateUsercallWrapper<void (*)(task * tp)>(noret, 0x5E1670, rEDI);

FastFunctionHook<void, task*> Rd_Ruin_t(0x5E18B0);

// Patch act swaps
void __cdecl Rd_Ruin_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		auto twp = tp->twp;

		switch (twp->mode)
		{
		case 0i8:
			RdRuinInit(tp);
			break;
		case 1i8:
			if (IsPlayerInSphere(6111.0f, -2445.0f, 1333.0f, 40.0f))
			{
				ChangeActM(1);
				twp->mode = 0i8;
			}
			break;
		case 2i8:

		{
			auto pnum = IsPlayerInSphere(6441.0f, -2421.0f, 1162.0f, 50.0f) - 1;

			if (pnum >= 0)
			{
				SetWinnerMulti(pnum); // Set winner there because act 3 consists of nothing
				ChangeActM(1);
				twp->mode = 0i8;
			}
		}

		if (byte_3C75126)
		{
			if (byte_3C75126 == 1)
			{
				if (playertwp[GetClosestPlayerNum(7740.0f, -2431.0f, 948.0f)]->pos.x >= 7600.0f)
				{
					if (++word_3C75124 > 600)
					{
						word_3C75124 = 0;
						ring_kiran = 1;
						SetSwitchOnOff(3u, 0);
						byte_3C75126 = 0;
					}
				}
				else
				{
					SetSwitchOnOff(3u, 1);
					ring_kiran = 0;
					byte_3C75126 = 2;
				}
			}
			else if (byte_3C75126 == 2)
			{
				if (++word_3C75124 > 600)
				{
					word_3C75124 = 0;
					ring_kiran = 1;
					byte_3C75126 = 0;
				}
				SetSwitchOnOff(3u, 0);
			}
		}
		else if (GetSwitchOnOff(2u))
		{
			byte_3C75126 = 1;
			word_3C75124 = 0;
		}

		break;
		}
	}
	else
	{
		Rd_Ruin_t.Original(tp);
	}
}

void patch_rd_ruin_init()
{
	Rd_Ruin_t.Hook(Rd_Ruin_r);
	WriteData((void**)0x5E1FCE, (void*)0x5E1F30); // Patch skybox mode
}

RegisterPatch patch_rd_ruin(patch_rd_ruin_init);
#include "pch.h"
#include "multiplayer.h"

// Rewrite the Windy Valley Tornado event
// Note: not accurate

DataPointer(NJS_POINT3*, tor_pos, 0x3C5D674);

static Float offset_ypos_m[PLAYER_MAX]{};
static Float yang_m[PLAYER_MAX]{};

static void VacumePlayer_w();
Trampoline VacumePlayer_t(0x4DE5F0, 0x4DE5F7, VacumePlayer_w);

static void VacumePlayer_o(task* tp)
{
	auto target = VacumePlayer_t.Target();
	__asm
	{
		mov eax, [tp]
		call target
	}
}

static void VacumePlayer_m(task* tp)
{
	auto twp = tp->twp;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (!tor_pos || !ptwp)
		{
			continue;
		}

		Float dist = sqrtf((ptwp->pos.x - tor_pos->x) * (ptwp->pos.x - tor_pos->x) + (ptwp->pos.z - tor_pos->z) * (ptwp->pos.z - tor_pos->z));
		if (dist < 190.0f)
		{
			// Fully taken by tornado

			offset_ypos_m[i] = 0.0f;
			yang_m[i] += 4.0f;

			ptwp->pos.x = njCos(NJM_DEG_ANG(-yang_m[i])) * 130.0f + tor_pos->x;
			ptwp->pos.z = njSin(NJM_DEG_ANG(-yang_m[i])) * 130.0f + tor_pos->z;
			ptwp->pos.y += 0.9f;

			SetInputP(i, PL_OP_PLACEON);
			SetAscendPowerP(i, 0, 0.092f, 0);

			if (++twp->value.w[0] > 300i16)
			{
				tornade_flag = TRUE;
			}
		}
		else if (dist < 790.0f)
		{
			// Slowly taken by tornado

			offset_ypos_m[i] += 3.8f;
			NJS_VECTOR v = { tor_pos->x + 110.0f, tor_pos->y + offset_ypos_m[i] + 190.0f, tor_pos->z + 110.0f };

			SetInputP(i, PL_OP_PLACEON);
			CalcAdvanceAsPossible(&ptwp->pos, &v, 5.5, &ptwp->pos);
			SetVelocityP(i, 0.0f, 0.0f, 0.0f);
			SetAscendPowerP(i, 0.0f, 0.0f, 0.0f);
			yang_m[i] = 90.0f;
		}
		else
		{
			offset_ypos_m[i] = 0;
		}
	}
}

static void __cdecl VacumePlayer_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		VacumePlayer_m(tp);
	}
	else
	{
		VacumePlayer_o(tp);
	}
}

static void __declspec(naked) VacumePlayer_w()
{
	__asm
	{
		push eax
		call VacumePlayer_r
		pop eax
		retn
	}
}
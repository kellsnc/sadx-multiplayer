#include "pch.h"

void __cdecl execTPCoaster_r(task* tp);
Trampoline execTPCoaster_t(0x61D6E0, 0x61D6E6, execTPCoaster_r);
void __cdecl execTPCoaster_r(task* tp)
{
	TARGET_STATIC(execTPCoaster)(tp);

	auto twp = tp->twp;
	auto id = twp->value.b[0];

	if (id > 0 && playertwp[id])
	{
		switch (twp->mode)
		{
		case 1:
			twp->scl.x = (twp->pos.x - playertwp[0]->pos.x) * 0.05f;
			twp->scl.y = (twp->pos.y - playertwp[0]->pos.y) * 0.05f;
			twp->scl.z = (twp->pos.z - playertwp[0]->pos.z) * 0.05f;
			break;
		case 2:
			if (++twp->wtimer <= 0x14ui16)
			{
				twp->pos.x += twp->scl.x;
				twp->pos.y += twp->scl.y;
				twp->pos.z += twp->scl.z;
			}
			else
			{
				SetInputP(id, PL_OP_PLACEWITHCART);
			}
			break;
		case 3:
			SetPositionP(id, twp->pos.x, twp->pos.y, twp->pos.z);
			SetRotationP(id, twp->ang.x, twp->ang.y - 0x4000, twp->ang.z);
			break;
		case 4:
			if (multiplayer::IsActive()) {
				for (uint8_t i = 1; i < multiplayer::GetPlayerCount(); i++)
				{
					RumbleA(i, 0);
					GetOutOfCartP(i, -1.2f, 1.5f, 0.0f);
				}
				twp->mode = 5;
			}
			break;
		}
	}
}
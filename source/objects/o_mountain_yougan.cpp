#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Patch to allow lava to go both ways

static void __cdecl Object_Mountain_Yougan_r(task* tp);
Trampoline Object_Mountain_Yougan_t(0x602670, 0x602676, Object_Mountain_Yougan_r);
static void __cdecl Object_Mountain_Yougan_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
		for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
		{
			if (playerpwp[i])
			{
				if (playerpwp[i]->item & Powerups_Dead && tp->twp->mode >= 2)
				{
					tp->twp->mode = 0;
					break;
				}
			}
		}
	}

	if (multiplayer::IsEnabled() && tp->twp->mode == 1 && CurrentCharacter != Characters_Gamma)
	{
		auto twp = tp->twp;

		twp->pos.z = 1559.0f;
		twp->pos.x = -1468.0f;

		if (rd_mountain_twp)
		{
			twp->pos.y += rd_mountain_twp->scl.y;

			if ((rd_mountain_twp->scl.y >= 0.0f && twp->pos.y >= rd_mountain_twp->scl.x) ||
				(rd_mountain_twp->scl.y < 0.0f && twp->pos.y <= rd_mountain_twp->scl.x))
			{
				twp->pos.y = rd_mountain_twp->scl.x;
				twp->scl.z = rd_mountain_twp->scl.x;
				rd_mountain_twp->scl.y = 0.0f;
				twp->mode = 3;

				object_youUP_yogun_yogun.basicdxmodel->mats->attr_texId = *(Uint8*)0x3C80F94;
				tp->disp(tp);

				return;
			}

			rd_mountain_twp->pos.y = twp->pos.y;
			rd_mountain_twp->counter.f = rd_mountain_twp->scl.y;
		}

		dsPlay_timer(130, (int)twp, 1, 0, 30);
		EntryColliList(twp);

		object_youUP_yogun_yogun.basicdxmodel->mats->attr_texId = *(Uint8*)0x3C80F94;
		tp->disp(tp);
	}
	else
	{
		TARGET_STATIC(Object_Mountain_Yougan)(tp);
	}
}
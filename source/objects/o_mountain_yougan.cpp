#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"

// Patch to allow lava to go both ways and player respawn above it

#define TARGET rd_mountain_twp->scl.x
#define SPEED rd_mountain_twp->scl.y

static void Object_Mountain_Yougan_m(task* tp)
{
	auto twp = tp->twp;

	//twp->pos.y = twp->scl.z = rd_mountain_twp->pos.y = 30.0f;

	switch (twp->mode)
	{
	case 1:
		twp->pos.z = 1559.0f;
		twp->pos.x = -1468.0f;

		if (rd_mountain_twp)
		{
			twp->pos.y += SPEED;

			if ((SPEED >= 0.0f && twp->pos.y >= TARGET) || (SPEED < 0.0f && twp->pos.y <= TARGET))
			{
				twp->pos.y = twp->scl.z = TARGET;
				rd_mountain_twp->scl.y = 0.0f;
				twp->mode = 3;
			}

			rd_mountain_twp->pos.y = twp->pos.y;
			rd_mountain_twp->counter.f = rd_mountain_twp->scl.y;

			if (continue_data.pos.y < twp->pos.y + 10.0f)
			{
				continue_data.pos.y = twp->pos.y;
			}
		}
		
		dsPlay_timer(130, (int)twp, 1, 0, 30);
		EntryColliList(twp);
		break;
	case 3:
	{
		Float offset = njSin(NJM_DEG_ANG(++twp->wtimer)) * 0.7f;
		rd_mountain_twp->counter.f = offset + twp->scl.z - twp->pos.y;
		twp->pos.y = rd_mountain_twp->pos.y = offset + twp->scl.z;
		if (rd_mountain_twp->scl.y != 0.0f)
		{
			twp->mode = 1;
		}
		EntryColliList(twp);
		break;
	}
	}

	object_youUP_yogun_yogun.basicdxmodel->mats->attr_texId = *(Uint8*)0x3C80F94;
	tp->disp(tp);
}

static void __cdecl Object_Mountain_Yougan_r(task* tp);
Trampoline Object_Mountain_Yougan_t(0x602670, 0x602676, Object_Mountain_Yougan_r);
static void __cdecl Object_Mountain_Yougan_r(task* tp)
{
	if (multiplayer::IsActive() && tp->twp->mode != 0)
	{
		Object_Mountain_Yougan_m(tp);
	}
	else
	{
		TARGET_STATIC(Object_Mountain_Yougan)(tp);
	}
}
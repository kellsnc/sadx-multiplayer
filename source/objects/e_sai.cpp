#include "pch.h"
#include "SADXModLoader.h"
#include "netplay.h"
#include "set.h"

TaskHook EnemySai_h(0x7A1380);

enum
{
	MODE_DYING = 7,
	MODE_DEAD = 8
};

static task* pTask;

#ifdef MULTI_NETPLAY
static bool SaiListener(Packet& packet, Netplay::PACKET_TYPE type, Netplay::PNUM pnum)
{
	if (type == Netplay::PACKET_OBJECT_RHINOTANK)
	{
		uint32_t set_id;

		if (packet.read(set_id))
		{
			auto task = GetSetTask(set_id, "E SAITO");

			if (task)
			{
				auto twp = task->twp;
				auto ewp = (enemywk*)task->mwp;

				if (twp && twp->mode < MODE_DYING && ewp)
				{
					packet >> twp->mode >> twp->pos >> twp->ang.y >> ewp->aim >> ewp->aim_angle >> ewp->velo >> ewp->ang_spd;
					return true;
				}
			}
		}
	}
	return false;
}

static bool SaiSender(Packet& packet, Netplay::PACKET_TYPE type, Netplay::PNUM pnum)
{
	if (type == Netplay::PACKET_OBJECT_RHINOTANK && pTask)
	{
		auto id = GetSetID(pTask);
		if (id >= 0)
		{
			auto twp = pTask->twp;
			auto ewp = (enemywk*)pTask->mwp;
			if (twp && ewp)
			{
				packet << (uint32_t)id << twp->mode << twp->pos << twp->ang.y << ewp->aim << ewp->aim_angle << ewp->velo << ewp->ang_spd;
				return true;
			}
		}
	}
	return false;
}
#endif

static void __cdecl EnemySai_r(task* tp)
{
#ifdef MULTI_NETPLAY
	if (tp->twp->mode != 0 && netplay.IsConnected())
	{
		if (netplay.GetPlayerNum() == 0)
		{
			pTask = tp;
			netplay.Send(Netplay::PACKET_OBJECT_RHINOTANK, SaiSender, -1, tp->twp->mode == MODE_DEAD ? true : false);
		}
	}
#endif

	auto backup = playertwp[0];
	playertwp[0] = playertwp[GetClosestPlayerNum(&tp->twp->pos)];
	EnemySai_h.Original(tp);
	playertwp[0] = backup;
}

void InitEnemySaiPatches()
{
	EnemySai_h.Hook(EnemySai_r);
#ifdef MULTI_NETPLAY
	netplay.RegisterListener(Netplay::PACKET_OBJECT_RHINOTANK, SaiListener);
#endif
}

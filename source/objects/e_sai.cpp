#include "pch.h"
#include "SADXModLoader.h"
#include "network.h"
#include "set.h"

enum
{
	MODE_DYING = 7,
	MODE_DEAD = 8
};

static task* pTask;

static bool SaiListener(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum)
{
	if (type == Network::PACKET_OBJECT_RHINOTANK)
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

static bool SaiSender(Packet& packet, Network::PACKET_TYPE type, Network::PNUM pnum)
{
	if (type == Network::PACKET_OBJECT_RHINOTANK && pTask)
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

static void __cdecl EnemySai_r(task* tp);
Trampoline EnemySai_t(0x7A1380, 0x7A138A, EnemySai_r);
static void __cdecl EnemySai_r(task* tp)
{
	// Temporary:
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;
		network.RegisterListener(Network::PACKET_OBJECT_RHINOTANK, SaiListener);
	}

	if (tp->twp->mode != 0 && network.IsConnected())
	{
		if (network.GetPlayerNum() == 0)
		{
			pTask = tp;
			network.Send(Network::PACKET_OBJECT_RHINOTANK, SaiSender, -1, tp->twp->mode == MODE_DEAD ? true : false);
		}
	}

	TARGET_STATIC(EnemySai)(tp);
}
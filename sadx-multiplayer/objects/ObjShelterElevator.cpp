#include "pch.h"
#include "multiplayer.h"

static auto InitATask = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5A1BF0, rEDI);
static auto ExecATask = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5A1A40, rEAX);
static auto DownMove = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5A1B60, rEDX);
static auto OpenDoor = GenerateUsercallWrapper<void (*)(task* tp)>(noret, 0x5A1D10, rEAX); // custom name

DataArray(NJS_POINT3, ElevatorPos, 0x1873100, 3 * 2); // [3][2]

enum : __int8
{
	MODE_INIT,
	MODE_NORMAL,
	MODE_WAIT,
	MODE_DOWN,
	MODE_OPEN,
	MODE_NONE // custom
};

static void chkPlayer(task* tp) // custom
{
	auto twp = tp->twp;

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		if (CheckPlayerRideOnMobileLandObjectP(i, tp))
		{
			PadReadOffP(-1);
			tp->twp->mode = MODE_DOWN;
			return;
		}
	}
}

static void movePlayer(task* tp) // custom
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			ptwp->pos.y = tp->twp->pos.y;
		}
	}
}

// Manage respawn in multiplayer
static void multiRespawn(task* tp)
{
	auto twp = tp->twp;
	auto pnum = IsPlayerInSphere(&ElevatorPos[twp->counter.b[0] * 2], 10.0f) - 1;

	if (pnum >= 0)
	{
		playertwp[pnum]->pos = ElevatorPos[twp->counter.b[0] * 2 + 1];
	}
}

// Set elevator into finished mode if object respawned naturally
static void InitATask_m(task* tp)
{
	InitATask(tp);

	if (GameState != 4 && GetStageNumber() != LevelAndActIDs_HotShelter2)
	{
		auto twp = tp->twp;

		twp->pos = ElevatorPos[twp->counter.b[0] * 2 + 1];
		twp->mode = MODE_OPEN;
	}
}

static void ObjShelterElevator_m(task* tp)
{
	auto twp = tp->twp;

	if (CheckRangeOutWithR(tp, 250000.0f))
	{
		return;
	}

	switch (twp->mode)
	{
	case MODE_INIT:
		InitATask_m(tp);
		break;
	case MODE_NORMAL:
		multiRespawn(tp);
		break;
	case MODE_WAIT:
		ExecATask(tp);
		chkPlayer(tp);
		break;
	case MODE_DOWN:
		DownMove(tp);
		movePlayer(tp);
		ExecATask(tp);
		break;
	case MODE_OPEN:
		OpenDoor(tp);
		ExecATask(tp);

		if (twp->mode == MODE_NORMAL)
		{
			PadReadOnP(-1);
		}
		break;
	case MODE_NONE:
		ExecATask(tp);
		break;
	}

	tp->disp(tp);
}

static void __cdecl ObjShelterElevator_r(task* tp);
Trampoline ObjShelterElevator_t(0x5A1D70, 0x5A1D76, ObjShelterElevator_r);
static void __cdecl ObjShelterElevator_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		ObjShelterElevator_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjShelterElevator)(tp);
	}
}